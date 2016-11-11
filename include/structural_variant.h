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

#ifndef NANOSV_STRUCTURAL_VARIANT_H
#define NANOSV_STRUCTURAL_VARIANT_H

#include "breakpoint.h"

struct nsv_sv_info_t
{
  char *precision;
  char *end;
  char *sv_type;
  char *sv_method;
};

struct nsv_sv_format_t
{
  
};

struct nsv_sv_t {
  char reference;
  char alternative;
  char quality;
  char *filter;

  struct nsv_breakpoint_t *breakpoint;
};

/**
 * This function allocates a nsv_sv_t.
 *
 * @return A pointer to a dynamically allocated nsv_sv_t struct.
 */
struct nsv_sv_t *nsv_sv_new (void);

/**
 * This function allocates a nsv_sv_t and sets 'breakpoint' as its breakpoint.
 * @param breakpoint  The breakpoint to set for this SV.
 *
 * @return A pointer to a dynamically allocated nsv_sv_t struct.
 */
struct nsv_sv_t *
nsv_sv_new_with_breakpoint (struct nsv_breakpoint_t *breakpoint);

void add_breakpoint (void);
void add_info_field (void);
void add_format_field (void);
void set_arguments (void);
void bayes_gt (void);
void log_choose (void);
void log10 (void);
void print_vcf (void);
void set_info_field (void);
void median (void);


#endif
