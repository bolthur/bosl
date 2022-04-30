/**
 * Copyright (C) 2022 bolthur project.
 *
 * This file is part of bolthur/bosl.
 *
 * bolthur/bosl is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * bolthur/bosl is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with bolthur/bosl.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdbool.h>

#if defined( _COMPILING_BOSL )
  #include "collection/hashmap.h"
  #include "scanner.h"
  #include "object.h"
#else
  #include <bosl/collection/hashmap.h>
  #include <bosl/scanner.h>
  #include <bosl/object.h>
#endif

#if !defined( BOSL_ENVIRONMENT_H )
#define BOSL_ENVIRONMENT_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct bosl_environment bosl_environment_t;

typedef struct bosl_environment {
  hashmap_table_t* value;
  bosl_environment_t* enclosing;
} bosl_environment_t;

bosl_environment_t* bosl_environment_init( bosl_environment_t* );
void bosl_environment_free( bosl_environment_t* );
bool bosl_environment_push_value( bosl_environment_t*, bosl_token_t*, bosl_object_t* );
bosl_object_t* bosl_environment_get_value( bosl_environment_t*, bosl_token_t* );
bool bosl_environment_assign_value( bosl_environment_t*, bosl_token_t*, bosl_object_t* );

#ifdef __cplusplus
}
#endif

#endif
