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

#include "breakpoint.h"
#include "segment.h"
#include "nanosvc.h"

#include <stdbool.h>
#include <stdlib.h>
#include <libinfra/logger.h>

extern struct nsv_config_t nsv_config;

struct nsv_breakpoint_t *
nsv_breakpoint_new (void)
{
  struct nsv_breakpoint_t *breakpoint;
  breakpoint = calloc (1, sizeof (struct nsv_breakpoint_t));
  if (breakpoint == NULL)
    {
      infra_logger_error_alloc (nsv_config.logger);
      return NULL;
    }

  breakpoint->type = NSVC_OBJ_BREAKPOINT;
  return breakpoint;
}

struct nsv_breakpoint_t *
nsv_breakpoint_new_with_segments (struct nsv_segment_t *first,
                                  struct nsv_segment_t *second)
{
  struct nsv_breakpoint_t *breakpoint = nsv_breakpoint_new ();
  if (breakpoint == NULL)
    return NULL;

  /* We don't check the validity of the arguments.  It is considered the
   * responsibility of the user to provide proper segment structs. */
  breakpoint->segments[0] = first;
  breakpoint->segments[1] = second;

  /* TODO: Not sure whether it's the right place to set the breakpoint
   * positions yet. */
  nsv_breakpoint_set_breakpoint (breakpoint);

  return breakpoint;
}

void
nsv_breakpoint_destroy (void *breakpoint_obj)
{
  struct nsv_breakpoint_t *breakpoint = breakpoint_obj;
  if (breakpoint->type != NSVC_OBJ_BREAKPOINT)
    {
      infra_logger_log (nsv_config.logger, LOG_ERROR,
                        "%s attempted to destroy an unknown object!",
                        __func__);
      return;
    }

  free (breakpoint);
}

bool
nsv_breakpoints_from_read (void *read_ptr, void **list_ptr)
{
  if (read_ptr == NULL || list_ptr == NULL)
    return FALSE;

  struct nsv_read_t *read_obj = read_ptr;
  GList *list = *list_ptr;

  uint32_t segments_len = g_list_length (read_obj->segments);
  if (segments_len > 1 && segments_len < nsv_config.max_split)
    {
      /* The segments must be sorted on their clip value for the next
       * step to be meaningful. */
      GList *segments = g_list_sort(read_obj->segments,
                                    &nsv_segment_clip_compare);

      while (segments->next != NULL)
        {
          struct nsv_segment_t *first = segments->data;
          struct nsv_segment_t *second = segments->next->data;

          int32_t clip_first = nsv_segment_cigar_first_clip (first);
          int32_t clip_second = nsv_segment_cigar_first_clip (second);

          /* When properly filtered in the BAM parsing step, we don't
           * need to check this here anymore. */
          /* if (clip_first == -1 || clip_second == -1) */
          /*   { */
          /*     segments = segments->next; */
          /*     continue; */
          /*   } */

          struct nsv_breakpoint_t *breakpoint;
          breakpoint = nsv_breakpoint_new_with_segments (first, second);
          breakpoint->gap = clip_first - clip_second + first->seq_len;

          /* Add the breakpoint to the lsit. */
          list = g_list_prepend (list, breakpoint);
          segments = segments->next;
        }
    }

  *list_ptr = list;
  return TRUE;
}

bool
nsv_breakpoint_set_breakpoint (struct nsv_breakpoint_t *breakpoint)
{
  if (breakpoint == NULL)
    return FALSE;

  if (breakpoint->segments[0] == NULL || breakpoint->segments[1] == NULL)
    return FALSE;

  /* We don't check the validity of the arguments.  It is considered the
   * responsibility of the user to provide proper segment structs. */
  struct nsv_segment_t *first = breakpoint->segments[0];
  struct nsv_segment_t *second = breakpoint->segments[1];

  /* When the 0x10 flag is set, it's a reverse complement. */
  breakpoint->breakpoints[0] = (first->flag & 0x10) ? first->pos : first->end;
  breakpoint->breakpoints[1] = (second->flag & 0x10) ? second->pos : second->end;

  return TRUE;
}

bool
nsv_breakpoint_switch_segments (struct nsv_breakpoint_t *breakpoint)
{
  if (breakpoint == NULL)
    return FALSE;

  struct nsv_segment_t *first = breakpoint->segments[0];
  struct nsv_segment_t *second = breakpoint->segments[1];

  if (first == NULL || second == NULL)
    return FALSE;

  /* Switch the segment pointers. */
  breakpoint->segments[0] = second;
  breakpoint->segments[1] = first;

  /* Toggle the 0x10 flag.
   * TODO: Figure out why. */
  first->flag &= (first->flag & 0x10) ? 0x0 : 0x10;
  second->flag &= (second->flag & 0x10) ? 0x0 : 0x10;

  return TRUE;
}

void
nsv_breakpoint_destroy_full (void *breakpoint_obj)
{
  struct nsv_breakpoint_t *breakpoint = breakpoint_obj;
  if (breakpoint->type != NSVC_OBJ_BREAKPOINT)
    {
      infra_logger_log (nsv_config.logger, LOG_ERROR,
                        "%s attempted to destroy an unknown object!",
                        __func__);
      return;
    }

  nsv_segment_destroy (breakpoint->segments[0]);
  nsv_segment_destroy (breakpoint->segments[1]);

  free (breakpoint->segments);
  free (breakpoint);
}
