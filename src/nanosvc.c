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

#include "nanosvc.h"

/* Program-wide configuration variables.  Do not assign new values to these
 * variables.  These variables can be updated at run-time with command-line
 * switches. */
struct nsv_config_t nsv_config = {
  .max_threads = 1,
  .max_window_size = 1000,
  .min_map_quality = 80,
  .min_identity = 0.80,
  .logger = NULL
};
