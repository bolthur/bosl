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

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "environment.h"
#include "collection/hashmap.h"
#include "scanner.h"
#include "error.h"

/**
 * @brief Environment hashmap cleanup helper
 *
 * @param object
 */
static void environment_cleanup( void* object ) {
  bosl_interpreter_destroy_object( object );
}

/**
 * @brief Initialize execution environment
 *
 * @return
 */
bosl_environment_t* bosl_environment_init( bosl_environment_t* enclosing ) {
  // allocate environment structure
  bosl_environment_t* environment = malloc( sizeof( bosl_environment_t ) );
  if ( ! environment ) {
    return NULL;
  }
  // clearout
  memset( environment, 0, sizeof( bosl_environment_t ) );
  // setup hashmap
  environment->value = hashmap_construct( environment_cleanup );
  if ( ! environment->value ) {
    free( environment );
    return NULL;
  }
  // set enclosing
  environment->enclosing = enclosing;
  // return success
  return environment;
}

/**
 * @brief Destroy environment
 *
 * @param environment
 */
void bosl_environment_free( bosl_environment_t* environment ) {
  // handle not initialized
  if ( ! environment ) {
    return;
  }
  // destroy hashmap
  hashmap_destruct( environment->value );
  // destroy object
  free( environment );
}

/**
 * @brief Push value to environment
 *
 * @param environment
 * @param token
 * @param data
 * @return
 */
bool bosl_environment_push_value(
  bosl_environment_t* environment,
  bosl_token_t* token,
  bosl_interpreter_object_t* data
) {
  // add to hashmap
  const char* r = hashmap_value_nset(
    environment->value,
    token->start,
    token->length,
    data
  );
  // set environment member
  if ( r ) {
    data->environment = true;
  }
  // return
  return r;
}

/**
 * @brief Get variable from environment
 *
 * @param environment
 * @param token
 * @return
 */
bosl_interpreter_object_t* bosl_environment_get_value(
  bosl_environment_t* environment,
  bosl_token_t* token
) {
  // try to get value
  bosl_interpreter_object_t* value = hashmap_value_nget(
    environment->value,
    token->start,
    token->length
  );
  // return if something is there
  if ( value ) {
    return value;
  }
  // try enclosing if set
  if ( environment->enclosing ) {
    return bosl_environment_get_value( environment->enclosing, token );
  }
  // handle not found
  bosl_error_raise( token, "Undefined variable." );
  return NULL;
}

/**
 * @brief Assign a value
 *
 * @param environment
 * @param token
 * @param object
 * @return
 */
bool bosl_environment_assign_value(
  bosl_environment_t* environment,
  bosl_token_t* token,
  bosl_interpreter_object_t* object
) {
  // try current level
  if ( hashmap_value_nget( environment->value, token->start, token->length ) ) {
    return bosl_environment_push_value( environment, token, object );
  }
  // try enclosing
  if ( environment->enclosing ) {
    return bosl_environment_assign_value( environment->enclosing, token, object );
  }
  // raise error and return false
  bosl_error_raise( token, "Undefined variable." );
  return false;
}
