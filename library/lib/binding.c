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

#include "binding.h"

static hashmap_table_t* binding = NULL;

/**
 * @brief Binding hashmap cleanup helper
 *
 * @param object
 */
static void environment_cleanup( void* object ) {
  bosl_object_destroy( object );
}

/**
 * @brief Init binding handling
 *
 * @return
 */
bool bosl_binding_init( void ) {
  // create hash map
  binding = hashmap_construct( environment_cleanup );
  // return result of construct as success or false
  return binding;
}

/**
 * @brief Free binding again
 */
void bosl_binding_free( void ) {
  // handle not initialized
  if ( ! binding ) {
    return;
  }
  // destroy hashmap
  hashmap_destruct( binding );
}

/**
 * @brief Bind a function
 *
 * @param name
 * @param callback
 * @return
 */
bool bosl_binding_bind_function( const char* name, bosl_callback_t callback ) {
  // handle not initialized
  if ( ! binding ) {
    return false;
  }
  // don't allow overwrite
  if ( bosl_binding_get( name ) ) {
    return false;
  }
  // create a new calllable object
  bosl_object_t* callable = bosl_object_allocate_callable( NULL, callback, NULL );
  if ( ! callable ) {
    return false;
  }
  // add to bindings
  return hashmap_value_set( binding, name, callable );
}

/**
 * @brief Unbind a function by name
 *
 * @param name
 * @return
 */
bool bosl_binding_unbind_function( const char* name ) {
  // handle not initialized
  if ( ! binding ) {
    return false;
  }
  // return true in case there is no such binding
  if ( ! bosl_binding_get( name ) ) {
    return true;
  }
  // remove from hashmap
  return hashmap_value_del( binding, name );
}

/**
 * @brief Get binding with normal null terminated string
 *
 * @param name
 * @return
 */
bosl_object_t* bosl_binding_get( const char* name ) {
  // handle not initialized
  if ( ! binding ) {
    return NULL;
  }
  // try to get binding from hashmap
  return hashmap_value_get( binding, name );
}

/**
 * @brief Get binding with name and length
 *
 * @param name
 * @param length
 * @return
 */
bosl_object_t* bosl_binding_nget( const char* name, size_t length ) {
  // handle not initialized
  if ( ! binding ) {
    return NULL;
  }
  // try to get binding from hashmap
  return hashmap_value_nget( binding, name, length );
}
