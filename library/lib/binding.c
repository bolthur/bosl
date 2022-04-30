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

#include <string.h>
#include "binding.h"

static hashmap_table_t* binding = NULL;

/**
 * @brief Binding hashmap cleanup helper
 *
 * @param object
 */
static void binding_cleanup( void* object ) {
  bosl_object_destroy( object );
}

/**
 * @brief Helper to build a return from c
 *
 * @param type
 * @param data
 * @param size
 * @return
 */
static bosl_object_t* build_return(
  bosl_object_type_t type,
  void* data,
  size_t size
) {
  // determine value type
  bosl_object_value_type_t value_type;
  switch ( type ) {
    case BOSL_OBJECT_TYPE_BOOL:
      value_type = BOSL_OBJECT_VALUE_BOOL;
      break;
    case BOSL_OBJECT_TYPE_UINT_8:
    case BOSL_OBJECT_TYPE_UINT_16:
    case BOSL_OBJECT_TYPE_UINT_32:
    case BOSL_OBJECT_TYPE_UINT_64:
      value_type = BOSL_OBJECT_VALUE_INT_UNSIGNED;
      break;
    case BOSL_OBJECT_TYPE_INT_8:
    case BOSL_OBJECT_TYPE_INT_16:
    case BOSL_OBJECT_TYPE_INT_32:
    case BOSL_OBJECT_TYPE_INT_64:
      value_type = BOSL_OBJECT_VALUE_INT_SIGNED;
      break;
    case BOSL_OBJECT_TYPE_STRING:
      value_type = BOSL_OBJECT_VALUE_STRING;
      break;
    case BOSL_OBJECT_TYPE_FLOAT:
      value_type = BOSL_OBJECT_VALUE_FLOAT;
      break;
    default:
      return NULL;
  }
  // build object and return it
  return bosl_object_allocate(
    value_type,
    type,
    data,
    size
  );
}

/**
 * @brief Init binding handling
 *
 * @return
 */
bool bosl_binding_init( void ) {
  // create hash map
  binding = hashmap_construct( binding_cleanup );
  // return result of construct as success or false
  return binding;
}

/**
 * @brief Free binding again
 */
void bosl_binding_free( void ) {
  // handle not initialized
  if ( !binding ) {
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
  if ( !binding ) {
    return false;
  }
  // don't allow to overwrite
  if ( bosl_binding_get( name ) ) {
    return false;
  }
  // create a new callable object
  bosl_object_t* callable = bosl_object_allocate_callable( NULL, callback, NULL );
  if ( !callable ) {
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
  if ( !binding ) {
    return false;
  }
  // return true in case there is no such binding
  if ( !bosl_binding_get( name ) ) {
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
  if ( !binding ) {
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
bosl_object_t* bosl_binding_get_n( const char* name, size_t length ) {
  // handle not initialized
  if ( !binding ) {
    return NULL;
  }
  // try to get binding from hashmap
  return hashmap_value_get_n( binding, name, length );
}

/**
 * @brief Return unsigned integer helper
 *
 * @param type
 * @param data
 * @return
 */
bosl_object_t* bosl_binding_build_return_uint(
  bosl_object_type_t type,
  uint64_t data
) {
  // handle invalid type
  if ( type < BOSL_OBJECT_TYPE_UINT_8 || type > BOSL_OBJECT_TYPE_UINT_64 ) {
    return NULL;
  }
  return build_return( type, &data, sizeof( data ) );
}

/**
 * @brief Return signed integer helper
 *
 * @param type
 * @param data
 * @return
 */
bosl_object_t* bosl_binding_build_return_int(
  bosl_object_type_t type,
  int64_t data
) {
  // handle invalid type
  if ( type < BOSL_OBJECT_TYPE_INT_8 || type > BOSL_OBJECT_TYPE_INT_64 ) {
    return NULL;
  }
  return build_return( type, &data, sizeof( data ) );
}

/**
 * @brief Return float helper
 *
 * @param data
 * @return
 */
bosl_object_t* bosl_binding_build_return_float( long double data ) {
  return build_return(
    BOSL_OBJECT_TYPE_FLOAT, &data, sizeof( data ) );
}

/**
 * @brief Return string helper
 *
 * @param data
 * @return
 */
bosl_object_t* bosl_binding_build_return_string(
  const char* data
) {
  return build_return(
    BOSL_OBJECT_TYPE_STRING, data, sizeof( char ) * strlen( data ) );
}

/**
 * @brief Return bool helper
 *
 * @param data
 * @return
 */
bosl_object_t* bosl_binding_build_return_bool( bool data ) {
  return build_return(
    BOSL_OBJECT_TYPE_BOOL, &data, sizeof( data ) );
}
