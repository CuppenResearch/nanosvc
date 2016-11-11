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

  /* When the 0x10 flag is set, it's a reverse complement. */
  breakpoint->breakpoints[0] = (first->flag & 0x10) ? first->pos : first->end;
  breakpoint->breakpoints[1] = (second->flag & 0x10) ? second->pos : second->end;

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
