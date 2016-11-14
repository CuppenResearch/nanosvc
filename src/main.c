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
#include <string.h>
#include <stdlib.h>
#include <getopt.h>
#include <stdint.h>
#include <glib.h>
#include <libinfra/logger.h>

#ifdef ENABLE_MTRACE
#include <mcheck.h>
#endif

#include "nanosvc.h"
#include "breakpoint.h"
#include "segment.h"
#include "read.h"
#include "trie.h"

/* Program-wide configuration variables.  Do not assign new values to these
 * variables.  These variables can be updated at run-time with command-line
 * switches. */
extern struct nsv_config_t nsv_config;

static void
show_version ()
{
  printf ("Version: %s\n", VERSION);
}

static void
show_help ()
{
  puts ("\nAvailable options:\n"
        " --max-threads, -m   Maximum number of threads to use.\n"
        " --split,       -s   Maximum number of segments per read.\n"
        " --distance,    -d   Maximum distance to cluster SVs together.\n"
        " --min-pid,     -p   Minimum percentage identity to reference.\n"
        " --file,        -f   A valid path to a session file.\n"
        " --log-file     -l   A log file to store the program's output.\n"
        " --version,     -v   Show versioning information.\n"
        " --help,        -h   Show this message.\n");
}

void
parse_sam_output (char *filename)
{
  infra_logger_log (nsv_config.logger, LOG_INFO, "Parsing '%s'\n", filename);

  char *extension = strrchr (filename, '.');
  if (extension == NULL)
    {
      infra_logger_log (nsv_config.logger, LOG_ERROR,
                        "Could not determine the file extension of '%s'\n",
                        filename);
      return;
    }

  /* Skip the dot. */
  extension++;

  GList *reads_list;
  if (!strcmp (extension, "sam"))
    reads_list = nsv_reads_from_sam (filename);
  else if (!strcmp (extension, "bam"))
    reads_list = nsv_reads_from_bam (filename);
  else
    {
      infra_logger_log (nsv_config.logger, LOG_ERROR,
                        "Unsupported file extension for '%s'\n",
                        filename);
      return;
    }

  if (reads_list == NULL)
    return;

  GList *breakpoints_list = NULL;
  while (reads_list->next != NULL)
    {
      struct nsv_read_t *read_obj = reads_list->data;
      if (read_obj == NULL)
        {
          reads_list = reads_list->next;
          continue;
        }

      /* Gather a list of breakpoints.  Unfortunately, this isn't all
       * "functional programming perfect", so we let the callback function
       * add to the new list.*/
      g_list_foreach (read_obj->segments,
                      &nsv_breakpoints_from_read,
                      (void *)&breakpoints_list);

      reads_list = reads_list->next;
    }

  infra_logger_log (nsv_config.logger, LOG_INFO,
                    "Found %d breakpoints.\n",
                    g_list_length (breakpoints_list));

  /* TODO: Free the breakpoints and the reads.. */
  g_list_free_full (breakpoints_list, nsv_breakpoint_destroy);
  g_list_free_full (reads_list, nsv_read_destroy);
}
  
int
main (int argc, char **argv)
{
  #ifdef ENABLE_MTRACE
  mtrace ();
  #endif

  if (argc < 2)
    {
      show_help ();
      return 1;
    }

  int32_t arg = 0;
  int32_t index = 0;
  char *z_option = NULL;

  /*----------------------------------------------------------------------.
   | OPTIONS                                                              |
   | An array of structs that list all possible arguments that can be     |
   | provided by the user.                                                |
   '----------------------------------------------------------------------*/
  static struct option options[] =
  {
    { "max-threads",       required_argument, 0, 't' },
    { "split",             required_argument, 0, 's' },
    { "distance",          required_argument, 0, 'd' },
    { "min-pid",           required_argument, 0, 'p' },
    { "mate-distance",     required_argument, 0, 'r' },
    { "max-window-size",   required_argument, 0, 'w' },
    { "cluster",           required_argument, 0, 'n' },
    { "min-mapq",          required_argument, 0, 'm' },
    { "file",              required_argument, 0, 'f' },
    { "log-file",          required_argument, 0, 'l' },
    { "help",              no_argument,       0, 'h' },
    { "version",           no_argument,       0, 'v' },
    { "test",              no_argument,       0, 'z' },
    { 0,                   0,                 0, 0 }
  };

  while (arg != -1)
    {
      /* Make sure to list all short options in the string below. */
      arg = getopt_long (argc, argv, "t:s:d:p:r:w:n:m:f:l:z:vh", options, &index);
      switch (arg)
        {
        case 't': nsv_config.max_threads = atoi (optarg); break;
        case 's': nsv_config.max_split = atoi (optarg); break;
        case 'd': break;
        case 'p': nsv_config.min_identity = atof (optarg); break;
        case 'r': break;
        case 'w': nsv_config.max_window_size = atoi (optarg); break;
        case 'n': break;
        case 'm': nsv_config.min_map_quality = atof (optarg); break;
        case 'f': break;
        case 'l': nsv_config.logger = infra_logger_new (optarg); break;
        case 'z': z_option = optarg; break;
        case 'v': show_version (); break;
        case 'h': show_help (); break;
        }
    }

  if (z_option != NULL)
    parse_sam_output (z_option);

  #ifdef ENABLE_MTRACE
  muntrace ();
  #endif

  if (nsv_config.logger)
    infra_logger_destroy (nsv_config.logger);

  return 0;
}
