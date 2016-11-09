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

#ifndef NANOSVC_H
#define NANOSVC_H

#include <stdint.h>
#include <libinfra/logger.h>

/**
 * This enumeration contains all struct types implemented in the nsvc
 * namespace.
 */
enum nanosvc_e {
  NSVC_OBJ_UNKNOWN,
  NSVC_OBJ_VCF,
  NSVC_OBJ_BREAKPOINT,
  NSVC_OBJ_SEGMENT,
  NSVC_OBJ_READ,
  NSVC_OBJ_TEMPLATE,
  NSVC_OBJ_SV,
  NSVC_OBJ_SVINFO,
  NSVC_OBJ_SVFORMAT
};

/**
 * This struct contains program-wide settings so that every component can
 * use them without explicitly passing the properties as arguments.
 */
struct nsv_config_t {
  uint16_t max_threads;
  uint32_t max_window_size;
  uint32_t min_map_quality;
  uint32_t max_split;
  float min_identity;
  struct infra_logger_t *logger;
};

/**
 * By casting any nsv_*_t object to this struct, one can identify its type.
 * This is useful when exposing the object to a loosely typed language, or
 * when passing objects around as void pointers.
 */
struct nsv_object_t {
  enum nanosvc_e type;
};

#endif
