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

#include "structural_variant.h"
#include "breakpoint.h"
#include "nanosvc.h"

#include <stdlib.h>
#include <libinfra/logger.h>

extern struct nsv_config_t nsv_config;

struct nsv_structural_variant_t *
nsv_structural_variant_new (void)
{
  struct nsv_structural_variant_t *structural_variant;
  structural_variant = calloc (1, sizeof (struct nsv_structural_variant_t));
  if (structural_variant == NULL)
    {
      infra_logger_error_alloc (nsv_config.logger);
      return NULL;
    }

  structural_variant->type = NSVC_OBJ_STRUCTURAL_VARIANT;
  return structural_variant;
}

void
nsv_structural_variant_destroy (void *structural_variant_obj)
{
  struct nsv_structural_variant_t *structural_variant = structural_variant_obj;
  if (structural_variant->type != NSVC_OBJ_STRUCTURAL_VARIANT)
    {
      infra_logger_log (nsv_config.logger, LOG_ERROR,
                        "%s attempted to destroy an unknown object!",
                        __func__);
      return;
    }

  free (structural_variant);
}

void
nsv_structural_variant_destroy_full (void *structural_variant_obj)
{
  struct nsv_structural_variant_t *structural_variant = structural_variant_obj;
  if (structural_variant->type != NSVC_OBJ_STRUCTURAL_VARIANT)
    {
      infra_logger_log (nsv_config.logger, LOG_ERROR,
                        "%s attempted to destroy an unknown object!",
                        __func__);
      return;
    }

  nsv_breakpoint_destroy (structural_variant->breakpoint);
  free (structural_variant);
}
