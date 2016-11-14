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

#ifndef NANOSVC_READ_H
#define NANOSVC_READ_H

#include "trie.h"
#include "segment.h"
#include "nanosvc.h"

#include <glib.h>

/**
 * This data structure contains the information about a read of a
 * sequence alignment map.
 */
struct nsv_read_t
{
  /*----------------------------------------------------------------------.
   | Object identification elements.
   '----------------------------------------------------------------------*/
  enum nanosvc_e type;

  /*----------------------------------------------------------------------.
   | Other elements.
   '----------------------------------------------------------------------*/
  char *qname;
  //uint32_t seq_len;             /* This cannot be determined easily. */
  GList *segments;
  GTree *btree;
};

/**
 * This function creates an empty read base structure.
 * @param A pointer to a dynamically allocated nsv_read_t object.
 */
struct nsv_read_t *nsv_read_new (void);

/**
 * This function adds a segment to a read.
 * @param read     
 * @param segment  
 *
 * @return true on succes, false on failure.
 */
bool nsv_read_add_segment (struct nsv_read_t *read,
                           struct nsv_segment_t *segment);

/**
 * This function extracts a list of nsv_read_t objects from a BAM file.
 * @param filename  The file to read.
 * @return A GList containing nsv_read_t objects.
 */
GList * nsv_reads_from_bam (const char *filename);

/**
 * This function extracts a list of nsv_read_t objects from a SAM file.
 * @param filename  The file to read.
 * @return A GList containing nsv_read_t objects.
 */
GList * nsv_reads_from_sam (const char *filename);

/**
 * This function removes a nsv_read_t from memory.  A void pointer
 * is used to play nicely with generic 'free' callback handlers.
 * @param read_obj   A pointer to a nsv_read_t struct.
 */
void nsv_read_destroy (void *read_obj);

#endif
