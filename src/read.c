/*
 * Copyright (C) 2016  Roel Janssen
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>

#include "read.h"
#include "segment.h"
#include "nanosvc.h"
#include "trie.h"

#include <libinfra/logger.h>
#include <libinfra/timer.h>

extern struct nsv_config_t nsv_config;

struct nsv_read_t *
nsv_read_new (void)
{
  struct nsv_read_t *read;
  read = calloc (1, sizeof (struct nsv_read_t));
  if (read == NULL)
    {
      infra_logger_error_alloc (nsv_config.logger);
      return NULL;
    }

  read->type = NSVC_OBJ_READ;
  return read;
}

bool
nsv_read_add_segment (struct nsv_read_t *read, struct nsv_segment_t *segment)
{
  if (read == NULL || segment == NULL)
    return FALSE;

  read->segments = g_list_prepend (read->segments, segment);
  return TRUE;
}

bool
nsv_reads_from_stream (FILE *stream, GList **output_ptr)
{
  if (output_ptr == NULL)
    return FALSE;

  /* We will store the list of segments in this variable. */
  GList *output = *output_ptr;

  /* Do some accounting for the segment parsing. */
  uint32_t filtered_count = 0;
  uint32_t added_count = 0;

  /* This trie will index the qname values of reads so that a read can be
   * found quickly. */
  struct trie_node_t *trie = trie_new ();
  if (trie == NULL)
    goto allocation_error_handler;

  char *qname = NULL;
  struct nsv_segment_t *segment = NULL;
  while ((segment = nsv_segment_from_stream (stream, &qname)) != NULL)
    {
      /* Filter/remove unmapped and low map quality segments.
       *
       * When the 0x4 flag is set, the region is unmapped, and we cannot make
       * any assumptions about the mapping quality.  We filter these segments
       * out.
       *
       * According to the SAMv1 specification, a value of 255 indicates that
       * the mapping quality is not available.  We therefore filter these out
       * too.
       *
       * At run-time, the user can set the minimum map quality value.  Anything
       * lower than this value will be filtered too.
       *
       * TODO: These filter conditions can be abstracted away as functions,
       * making the applicability of this function useful to a broader
       * audience.
       **/
      if (segment->flag & 0x4
          || segment->mapq < nsv_config.min_map_quality
          || segment->mapq == 255
          || nsv_segment_cigar_pid (segment) < nsv_config.min_identity)
        {
          free (qname);
          nsv_segment_destroy (segment);
          filtered_count++;
        }
      else
        {
          /* When a segment does not have a clipping point, then we cannot
           * use it to detect structural variation. */
          int32_t clip = nsv_segment_cigar_first_clip (segment);
          if (clip == -1)
            {
              free (qname);
              nsv_segment_destroy (segment);
              filtered_count++;
              continue;
            }

          struct nsv_read_t *read_obj = NULL;
          struct nsv_read_t *trie_element = trie_find (trie, qname);
          if (trie_element == NULL)
            {
              read_obj = nsv_read_new ();
              if (read_obj == NULL)
                goto allocation_error_handler;

              read_obj->qname = qname;
              qname = NULL;
              trie_insert (trie, read_obj->qname, read_obj);
              output = g_list_prepend (output, read_obj);
            }
          else
            {
              read_obj = trie_element;
              free (qname);
            }

          segment->read = read_obj;
          read_obj->segments = g_list_prepend (read_obj->segments, segment);
          added_count++;
        }
    }

  infra_logger_log (nsv_config.logger, LOG_INFO,
                    "Parsed %u segments, of which %u were filtered.",
                    added_count + filtered_count, filtered_count);

  /* Provide feedback to the user on the parsing step. */
  infra_logger_log (nsv_config.logger, LOG_INFO,
                    "Parsed %u segments from sambamba's output.",
                    filtered_count + added_count);

  infra_logger_log (nsv_config.logger, LOG_INFO,
                    "Filtered %u segments with a map quality threshold of %d.",
                    filtered_count, nsv_config.min_map_quality);

  /* All segments have been read, so we no longer need the trie. */
  trie_destroy (trie);

  *output_ptr = output;
  return TRUE;

 allocation_error_handler:
  infra_logger_error_alloc (nsv_config.logger);
  g_list_free_full (output, nsv_read_destroy);
  *output_ptr = NULL;
  return FALSE;
}

GList *
nsv_reads_from_sam (const char *filename)
{
  if (filename == NULL)
    return NULL;

  FILE *sam_file = fopen (filename, "r");
  if (sam_file == NULL)
    {
      infra_logger_log (nsv_config.logger, LOG_ERROR,
                        "Could not open the SAM file.");

      return NULL;
    }

  infra_logger_log (nsv_config.logger, LOG_INFO, "Reading from: %s", filename);

  /* When nsv_reads_from_stream fails, 'output' will be NULL, which is
   * exactly the value we need upon an error. */
  GList *output = NULL;
  nsv_reads_from_stream (sam_file, &output);
  
  fclose (sam_file);
  return output;
}

GList *
nsv_reads_from_bam (const char *filename)
{
  if (filename == NULL)
    return NULL;

  /* This stack buffer will contain the command to run. */
  size_t command_line_len = 20 + strlen (filename);
  char command_line[command_line_len];

  snprintf (command_line, command_line_len,
            "sambamba view -t %u %s",
            nsv_config.max_threads, filename);

  command_line[command_line_len - 1] = '\0';

  infra_logger_log (nsv_config.logger, LOG_INFO, "Running: %s", command_line);

  FILE *command = popen (command_line, "r");
  if (command == NULL)
    {
      infra_logger_log (nsv_config.logger, LOG_ERROR,
                        "Could not run sambamba.");

      return NULL;
    }

  /* We will store the list of reads in this variable. */
  GList *output = NULL;
  nsv_reads_from_stream (command, &output);

  /* Now that we have parsed all output from sambamba, we can close the pipe. */
  pclose (command);

  return output;
}

void
nsv_read_destroy (void *read_obj)
{
  struct nsv_read_t *obj = read_obj;
  if (obj->type != NSVC_OBJ_READ)
    {
      infra_logger_log (nsv_config.logger, LOG_ERROR,
                        "%s attempted to destroy an unknown object!",
                        __func__);
      return;
    }

  free (obj->qname);
  g_list_free_full (obj->segments, nsv_segment_destroy);
  free (obj);
}
