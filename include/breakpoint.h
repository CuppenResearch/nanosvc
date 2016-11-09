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

#ifndef NANOSVC_BREAKPOINT_H
#define NANOSVC_BREAKPOINT_H

#include "trie.h"
#include "segment.h"
#include "read.h"
#include "nanosvc.h"

#include <glib.h>

/**
 * This data structure contains the information about a read of a
 * sequence alignment map.
 */
struct nsv_breakpoint_t
{
  /*----------------------------------------------------------------------.
   | Object identification elements.
   '----------------------------------------------------------------------*/
  enum nanosvc_e type;

  /*----------------------------------------------------------------------.
   | Other elements.
   '----------------------------------------------------------------------*/

  /* It's probably easiest to understand this as an array of two pointers.
   * So, this array does not have to allocated explicitly.  Its memory
   * is allocated when the memory for the entire struct is allocated. */
  struct nsv_segment_t *segments[2]; /*< Segments involved in the breakpoint. */

  int32_t breakpoints[2];            /*< The position of the breakpoints. */
  int32_t gap;                       /*< The gap between the segments. */
};

/**
 * This function creates an empty breakpoint base structure.
 *
 * @return A pointer to a dynamically allocated nsv_breakpoint_t object.
 */
struct nsv_breakpoint_t *nsv_breakpoint_new (void);

/**
 * This function creates a breakpoint base structure with 'first' and 'second'
 * as its segments.
 * @param first  The first segment.
 * @param second the second segment.
 *
 * @return A pointer to a dynamically allocated nsv_breakpoint_t object.
 */
struct nsv_breakpoint_t *
nsv_breakpoint_new_with_segments (struct nsv_segment_t *first,
                                  struct nsv_segment_t *second);

/**
 * This function removes a nsv_breakpoint_t from memory.  A void pointer
 * is used to play nicely with generic 'free' callback handlers.
 * @param breakpoint   A pointer to a nsv_breakpoint_t struct.
 */
void nsv_breakpoint_destroy (void *breakpoint);

/**
 * This function removes a nsv_breakpoint_t from memory, including the
 * nsv_segment_t instances linked to this breakpoint.
 * @param breakpoint   A pointer to a nsv_breakpoint_t struct.
 */
void nsv_breakpoint_destroy_full (void *breakpoint);

#endif
