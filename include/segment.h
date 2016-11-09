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

#ifndef NANOSVC_SEGMENT_H
#define NANOSVC_SEGMENT_H

#include "nanosvc.h"
#include <glib.h>

/**
 * This data structure contains the quantitative information from a CIGAR
 * string.
 */
struct nsv_segment_cigar_overview_t
{
  uint16_t insertions;        /*< Number of insertions. */
  uint16_t deletions;         /*< Number of deletions. */
  uint16_t alignment_matches; /*< Number of alignment matches. */
  uint16_t matches;           /*< Number of matches to the reference. */
  uint16_t mismatches;        /*< Number of mismatches to the reference. */
  uint16_t skipped;           /*< Number of skipped bases from the reference. */
  uint16_t soft_clip;         /*< Number of soft clipping sequences. */
  uint16_t hard_clip;         /*< Position of the hard clipping point. */
  uint16_t padding;           /*< Number of silent deletions from the
                                  padded reference. */
};

/**
 * This data structure contains the information about a segment of a
 * sequence alignment map.
 */
struct nsv_segment_t
{
  /*----------------------------------------------------------------------.
   | Object identification elements.
   '----------------------------------------------------------------------*/
  enum nanosvc_e type;          /*< Data type specification. */

  /*----------------------------------------------------------------------.
   | SAMv1 elements.
   '----------------------------------------------------------------------*/
  /* The qname is stored by the read. */
  int16_t flag;                 /*< Bitwise flag. */
  char *rname;                  /*< Reference sequence name. */
  int32_t pos;                  /*< 1-based left most mapping position. */
  uint16_t mapq;                 /*< Mapping quality. */
  char *cigar;                  /*< CIGAR string. */
  char *rnext;                  /*< Reference name of the mate/next read. */
  int32_t pnext;                /*< Position of the mate/next read. */
  int32_t tlen;                 /*< Observed template length. */
  char *seq;                    /*< Segment sequence. */
  char *qual;                   /*< ASCII of the Phred-scaled base quality+33.*/

  /*----------------------------------------------------------------------.
   | Extra elements.
   '----------------------------------------------------------------------*/
  struct nsv_read_t *read;      /*< The read this segment belongs to. */
  uint32_t seq_len;             /*< Segment sequence length. */

  float rlength;                /*< Median length of the total reads. */
  float plength;                /*< Median segment length percentage of the
                                    two segments of the structural variant. */
  int32_t clip;                 /*< The first clip value in the CIGAR string. */

  int32_t end;
  char *id;
  float pid;
};

/**
 * This function creates an empty segment base structure.
 * @param A pointer to a dynamically allocated nsv_segment_t object.
 */
struct nsv_segment_t * nsv_segment_new (void);

/**
 * This function parses the CIGAR string and sets the clip member of a
 * @ref nsv_segment.
 * @param segment  The segment to analyze the CIGAR string of.
 * @return A nsv_segment_cigar_overview_t struct.
 */
struct nsv_segment_cigar_overview_t
nsv_segment_cigar_overview (struct nsv_segment_t *segment);

/**
 * This function returns the value of the first clip (hard or soft)
 * it finds in the CIGAR string.
 *
 * @param segment  The segment to analyze the CIGAR string of.
 * @return the first clip found in the CIGAR string.
 */
int32_t nsv_segment_cigar_first_clip (struct nsv_segment_t *segment);

/**
 * This function returns the percentage identity to the reference.
 * @param segment  The segment to analyze th CIGAR string of.
 * @return The percentage identity to the reference.
 */
float nsv_segment_cigar_pid (struct nsv_segment_t *segment);

/**
 * 
 */
void nsv_segment_plength (struct nsv_segment_t *segment);

void nsv_segment_set_pid (void);
void nsv_segment_set_clip (void);

/**
 * This function removes a nsv_segment_t from memory.  A void pointer
 * is used to play nicely with generic 'free' callback handlers.
 * @param segment_obj   A pointer to a nsv_segment_t struct.
 */
void nsv_segment_destroy (void *segment_obj);

/**
 * This function attempts to read a segment from a stream.
 * @param stream  The stream to read from.
 * @param qname_ptr  A pointer to a char* in which the qname will be placed.
 *
 * @return A pointer to a dynamically allocated nsv_segment_t.
 */
struct nsv_segment_t * nsv_segment_from_stream (FILE *stream, char **qname_ptr);

#endif
