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
#include <float.h>
#include "object.h"
#include "collection/hashmap.h"

static hashmap_table_t* type_map = NULL;

/**
 * @brief Init object handling stuff
 *
 * @return
 */
bool bosl_object_init( void ) {
  // create and fill hash map
  type_map = hashmap_construct( NULL );
  if ( ! type_map ) {
    return false;
  }
  // populate hashmap with key words
  if (
    ! hashmap_value_set( type_map, "int8", ( void* )OBJECT_TYPE_INT8 )
    || ! hashmap_value_set( type_map, "int16", ( void* )OBJECT_TYPE_INT16 )
    || ! hashmap_value_set( type_map, "int32", ( void* )OBJECT_TYPE_INT32 )
    || ! hashmap_value_set( type_map, "int64", ( void* )OBJECT_TYPE_INT64 )
    || ! hashmap_value_set( type_map, "uint8", ( void* )OBJECT_TYPE_UINT8 )
    || ! hashmap_value_set( type_map, "uint16", ( void* )OBJECT_TYPE_UINT16 )
    || ! hashmap_value_set( type_map, "uint32", ( void* )OBJECT_TYPE_UINT32 )
    || ! hashmap_value_set( type_map, "uint64", ( void* )OBJECT_TYPE_UINT64 )
    || ! hashmap_value_set( type_map, "float", ( void* )OBJECT_TYPE_FLOAT )
    || ! hashmap_value_set( type_map, "string", ( void* )OBJECT_TYPE_STRING )
  ) {
    hashmap_destruct( type_map );
    return false;
  }
  // return success
  return true;
}

/**
 * @brief Free object handling stuff
 */
void bosl_object_free( void ) {
  if ( ! type_map ) {
    return;
  }
  hashmap_destruct( type_map );
}

/**
 * @brief Destroy an object
 *
 * @param object
 */
void bosl_object_destroy( bosl_object_t* object ) {
  // handle no object
  if ( ! object ) {
    return;
  }
  // destroy data
  if ( object->data ) {
    free( object->data );
  }
  // destroy object
  free( object );
}

/**
 * @brief Allocate an object
 *
 * @param type
 * @param data
 * @param size
 * @return
 */
bosl_object_t* bosl_object_allocate(
  bosl_object_value_type_t value_type,
  void* data,
  size_t size
) {
  // allocate object
  bosl_object_t* o = malloc( sizeof( bosl_object_t ) );
  if ( ! o ) {
    return NULL;
  }
  // allocate data
  o->data = malloc( size );
  if ( ! o->data ) {
    bosl_object_destroy( o );
    return NULL;
  }
  // populate data
  o->size = size;
  o->value_type = value_type;
  o->type = OBJECT_TYPE_UNDEFINED;
  o->environment = false;
  o->constant = false;
  o->is_return = false;
  // copy over
  if ( OBJECT_VALUE_BOOL == value_type ) {
    *( ( bool* )( o->data ) ) = *( ( bool* )data );
  } else {
    memcpy( o->data, data, size );
  }
  // return object
  return o;
}

/**
 * @brief Helper to allocate callable object
 *
 * @param statement
 * @param callback
 * @return
 */
bosl_object_t* bosl_object_allocate_callable(
  bosl_ast_statement_function_t* statement,
  bosl_callback_t callback
) {
  // allocate object
  bosl_object_t* o = malloc( sizeof( bosl_object_t ) );
  if ( ! o ) {
    return NULL;
  }
  // allocate data
  o->data = malloc( sizeof( bosl_object_callable_t ) );
  if ( ! o->data ) {
    bosl_object_destroy( o );
    return NULL;
  }
  // clear out data
  memset( o->data, 0, sizeof( bosl_object_callable_t ) );
  // populate object data
  o->size = sizeof( bosl_object_callable_t );
  o->value_type = OBJECT_VALUE_CALLABLE;
  o->type = OBJECT_TYPE_UNDEFINED;
  o->environment = false;
  o->constant = false;
  // populate nested data
  bosl_object_callable_t* callable = o->data;
  callable->statement = statement;
  callable->callback = callback;
  // return object
  return o;
}

/**
 * @brief Duplicate an object if from environment
 *
 * @param obj
 * @return
 */
bosl_object_t* bosl_object_duplicate_environment( bosl_object_t* obj ) {
  // handle invalid pointer
  if ( ! obj ) {
    return NULL;
  }
  // no duplication necessary if not from environment
  if ( ! obj->environment ) {
    return obj;
  }
  // return duplicate
  return bosl_object_allocate(
    obj->value_type,
    obj->data,
    obj->size
  );
}

/**
 * @brief Convert string to type
 *
 * @param str
 * @param length
 * @return
 */
bosl_object_type_t bosl_object_str_to_type( const char* str, size_t length ) {
  void* result = hashmap_value_nget( type_map, str, length );
  if ( ! result ) {
    return OBJECT_TYPE_UNDEFINED;
  }
  return ( bosl_object_type_t )result;
}

/**
 * @brief Get minimum possible integer value per type
 *
 * @param type
 * @return
 */
int64_t bosl_object_type_min_int_value( bosl_object_type_t type ) {
  switch ( type ) {
    case OBJECT_TYPE_UINT8: return 0;
    case OBJECT_TYPE_UINT16: return 0;
    case OBJECT_TYPE_UINT32: return 0;
    case OBJECT_TYPE_UINT64: return 0;
    case OBJECT_TYPE_INT8: return INT8_MIN;
    case OBJECT_TYPE_INT16: return INT16_MIN;
    case OBJECT_TYPE_INT32: return INT32_MIN;
    case OBJECT_TYPE_INT64: return INT64_MIN;
    default: return 0;
  }
}

/**
 * @brief Get maximum possible integer value per type
 *
 * @param type
 * @return
 */
uint64_t bosl_object_type_max_int_value( bosl_object_type_t type ) {
  switch ( type ) {
    case OBJECT_TYPE_UINT8: return UINT8_MAX;
    case OBJECT_TYPE_UINT16: return UINT16_MAX;
    case OBJECT_TYPE_UINT32: return UINT32_MAX;
    case OBJECT_TYPE_UINT64: return UINT64_MAX;
    case OBJECT_TYPE_INT8: return INT8_MAX;
    case OBJECT_TYPE_INT16: return INT16_MAX;
    case OBJECT_TYPE_INT32: return INT32_MAX;
    case OBJECT_TYPE_INT64: return INT64_MAX;
    default: return 0;
  }
}

/**
 * @brief Get minimum possible float value
 *
 * @param type
 * @return
 */
long double bosl_object_type_min_float_value( bosl_object_type_t type ) {
  switch ( type ) {
    case OBJECT_TYPE_FLOAT: return LDBL_MIN;
    default: return 0;
  }
}

/**
 * @brief Get maximum possible float value
 *
 * @param type
 * @return
 */
long double bosl_object_type_max_float_value( bosl_object_type_t type ) {
  switch ( type ) {
    case OBJECT_TYPE_FLOAT: return LDBL_MAX;
    default: return 0;
  }
}
