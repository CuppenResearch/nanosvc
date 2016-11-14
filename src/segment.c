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

#include "trie.h"
#include "segment.h"
#include "nanosvc.h"

#include <libinfra/logger.h>
#include <libinfra/timer.h>

extern struct nsv_config_t nsv_config;

struct nsv_segment_t *
nsv_segment_new (void)
{
  struct nsv_segment_t *segment;
  segment = calloc (1, sizeof (struct nsv_segment_t));
  if (segment == NULL)
    {
      infra_logger_error_alloc (nsv_config.logger);
      return NULL;
    }

  segment->type = NSVC_OBJ_SEGMENT;
  return segment;
}

struct nsv_segment_t *
nsv_segment_from_stream (FILE *stream, char **qname_ptr)
{
  if (stream == NULL || qname_ptr == NULL)
    return NULL;

  struct nsv_segment_t *segment = nsv_segment_new ();
  if (segment == NULL)
    return NULL;

  /* Columns:
   * qname, flag, rname, pos, mapq, cigar, rnext, pnext, tlen, seq, qual, tags. */

  /* The 'field_index' tells which field we are currently parsing.
   * So, field_index = 0 means we are parsing the qname, field_index = 1 means
   * we are parsing the flag. */
  uint8_t field_index = 0;

  /* 'field' is a temporary buffer on the stack that contains the contents of
   * the current field.  When the field's contents are complete, we make a copy
   * to the heap memory. */
  const uint16_t field_max_length = 512;
  char field[field_max_length];
  uint16_t field_pos = 0;
  memset (field, '\0', field_max_length);

  char *qname = NULL;
  char buffer;
  buffer = getc(stream);
  if (buffer == EOF)
    {
      nsv_segment_destroy (segment);
      return NULL;
    }

  while (buffer != EOF)
    {
      /* When we encounter a header line, we need to skip the entire line. */
      if (field_pos == 0 && field_index == 0 && buffer == '@')
        {
          /* Skip the current line. */
          while (buffer != '\n' && buffer != EOF)
            buffer = getc (stream);

          if (buffer == EOF)
            {
              nsv_segment_destroy (segment);
              return NULL;
            }

          /* Reset the state. */
          field_pos = 0;
          field_index = 0;

          /* Move past the '\n' character. */
          buffer = getc (stream);
          continue;
        }

      else if (buffer == '\n')
        {
          /* TODO: Add the last field to the segment. */
          break;
        }

      else if (buffer == '\t')
        {
          /* Process the field that has now been completely parsed. */
          switch (field_index)
            {
              case 0:  qname          = strdup (field); break;
              case 1:  segment->flag  = atoi(field);    break;
              case 2:  segment->rname = strdup (field); break;
              case 3:  segment->pos   = atoi(field);    break;
              case 4:  segment->mapq  = atoi(field);    break;
              case 5:  segment->cigar = strdup (field); break;
              case 6:  segment->rnext = strdup (field); break;
              case 7:  segment->pnext = atoi(field);    break;
              case 8:  segment->tlen  = atoi(field);    break;
              case 9:  segment->seq   = strdup (field); break;
              case 10: segment->qual  = strdup (field); break;
            }

          field_index++;

          /* Reset the field so that the next field can be stored into it. */
          memset (field, '\0', field_pos + 1);
          field_pos = 0;
        }
      /* When the maximum length of the field buffer has been reached, we wait
       * for the next delimiter, effectively truncating the field's contents. */
      else if (field_pos < field_max_length)
        {
          field[field_pos] = buffer;
          field_pos++;
        }

      buffer = getc(stream);
    }

  if (segment->cigar != NULL)
    {
      //segment->clip = nsv_segment_parse_cigar (segment);
    }

  /* Set extra fields. */
  if (segment->seq != NULL)
    segment->seq_len = strlen (segment->seq);

  /* TODO: What's the proper name for this? */
  struct nsv_segment_cigar_overview_t overview;
  segment->end = segment->pos + segment->seq_len;
  segment->end += overview.deletions;
  segment->end -= overview.insertions;

  *qname_ptr = qname;
  return segment;
}

int
nsv_segment_clip_compare (const void *first, const void *second)
{
  /* Treat an unset element as the smallest possible value. */
  if (first == NULL && second == NULL)
    return 0;
  else if (first == NULL)
    return -1;
  else if (second == NULL)
    return 1;

  struct nsv_segment_t *a = (struct nsv_segment_t *)first;
  struct nsv_segment_t *b = (struct nsv_segment_t *)second;

  int32_t a_clip = nsv_segment_cigar_first_clip (a);
  int32_t b_clip = nsv_segment_cigar_first_clip (b);

  /* This is the shorthand notation for returning -1 when a is smaller than b, 
   * 0 when a is equal to b, and 1 when a is bigger than b. */
  return (a_clip < b_clip) ? -1 : (a_clip == b_clip) ? 0 : 1;
}

float
nsv_segment_cigar_pid (struct nsv_segment_t *segment)
{
  /* We assume a number won't have more than 64 digits here. :) */
  char buffer[64];
  memset (buffer, '\0', 64);

  uint32_t cigar_len = strlen (segment->cigar);
  if (cigar_len == 0)
    return -1;

  uint32_t cigar_index = 0;
  uint32_t buffer_index = 0;
  for (; cigar_index < cigar_len; cigar_index++)
    {
      switch (segment->cigar[cigar_index])
        {
        case 'I':
        case 'D':
        case 'M':
        case 'N':
        case 'X':
        case 'S':
        case 'H':
          break;
        case '=':
          return (atof (buffer) / segment->seq_len);
          break;
        default:
          {
            buffer[buffer_index] = segment->cigar[cigar_index];
            buffer_index++;

            /* We don't want to clear the buffer when it hasn't reached an
             * action yet.  We therefore skip the rest of the loop. */
            continue;
          }
          break;
        }

      memset (buffer, '\0', buffer_index);
      buffer_index = 0;
    }

  return -1;
}

int32_t
nsv_segment_cigar_first_clip (struct nsv_segment_t *segment)
{
  if (segment == NULL)
    return -1;

  /* Use the cached value whenever it's available. */
  if (segment->clip != -1)
    return segment->clip;

  /* We assume a number won't have more than 64 digits here. :) */
  char buffer[64];
  memset (buffer, '\0', 64);

  uint32_t cigar_len = strlen (segment->cigar);
  if (cigar_len == 0)
    return -1;

  uint32_t cigar_index = 0;
  uint32_t buffer_index = 0;
  for (; cigar_index < cigar_len; cigar_index++)
    {
      switch (segment->cigar[cigar_index])
        {
        case 'I':
        case 'D':
        case 'M':
        case 'N':
        case '=':
        case 'X':
          break;
        case 'S':
        case 'H':
          segment->clip = atoi (buffer);
          return segment->clip;
          break;
        default:
          {
            buffer[buffer_index] = segment->cigar[cigar_index];
            buffer_index++;

            /* We don't want to clear the buffer when it hasn't reached an
             * action yet.  We therefore skip the rest of the loop. */
            continue;
          }
          break;
        }

      memset (buffer, '\0', buffer_index);
      buffer_index = 0;
    }

  return -1;
}

struct nsv_segment_cigar_overview_t
nsv_segment_cigar_overview (struct nsv_segment_t *segment)
{
  struct nsv_segment_cigar_overview_t overview =
    { .insertions = 0, .deletions = 0, .matches = 0 };

  if (segment == NULL)
    return overview;

  if (segment->cigar == NULL)
    return overview;

  uint32_t cigar_len = strlen (segment->cigar);
  if (cigar_len == 0)
    return overview;

  /* We assume a number won't have more than 64 digits here. :) */
  char buffer[64];
  memset (buffer, '\0', 64);

  uint32_t cigar_index = 0;
  uint32_t buffer_index = 0;
  for (; cigar_index < cigar_len; cigar_index++)
    {
      switch (segment->cigar[cigar_index])
        {
        case 'I': overview.insertions        += atoi (buffer); break;
        case 'D': overview.deletions         += atoi (buffer); break;
        case 'M': overview.alignment_matches += atoi (buffer); break;
        case 'N': overview.skipped           += atoi (buffer); break;
        case 'S': overview.soft_clip          = atoi (buffer); break;
        case 'H': overview.hard_clip          = atoi (buffer); break;
        case 'P': overview.padding           += atoi (buffer); break;
        case '=': overview.matches           += atoi (buffer); break;
        case 'X': overview.mismatches        += atoi (buffer); break;
        default:
          {
            buffer[buffer_index] = segment->cigar[cigar_index];
            buffer_index++;

            /* We don't want to clear the buffer when it hasn't reached an
             * action yet.  We therefore skip the rest of the loop. */
            continue;
          }
          break;
        }

      memset (buffer, '\0', buffer_index);
      buffer_index = 0;
    }

  return overview;
}

void
nsv_segment_destroy (void *segment_obj)
{
  struct nsv_segment_t *segment = segment_obj;
  if (segment->type != NSVC_OBJ_SEGMENT)
    {
      infra_logger_log (nsv_config.logger, LOG_ERROR,
                        "%s attempted to destroy an unknown object!",
                        __func__);
      return;
    }

  free (segment->rname);
  free (segment->cigar);
  free (segment->rnext);
  free (segment->seq);
  free (segment->qual);
  free (segment);
}
