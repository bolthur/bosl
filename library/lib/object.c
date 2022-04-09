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
#include "error.h"
#include "environment.h"

// for debug purposes only
#include <stdio.h>
#include <inttypes.h>

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
    ! hashmap_value_set( type_map, "int8", ( void* )BOSL_OBJECT_TYPE_INT8 )
    || ! hashmap_value_set( type_map, "int16", ( void* )BOSL_OBJECT_TYPE_INT16 )
    || ! hashmap_value_set( type_map, "int32", ( void* )BOSL_OBJECT_TYPE_INT32 )
    || ! hashmap_value_set( type_map, "int64", ( void* )BOSL_OBJECT_TYPE_INT64 )
    || ! hashmap_value_set( type_map, "uint8", ( void* )BOSL_OBJECT_TYPE_UINT8 )
    || ! hashmap_value_set( type_map, "uint16", ( void* )BOSL_OBJECT_TYPE_UINT16 )
    || ! hashmap_value_set( type_map, "uint32", ( void* )BOSL_OBJECT_TYPE_UINT32 )
    || ! hashmap_value_set( type_map, "uint64", ( void* )BOSL_OBJECT_TYPE_UINT64 )
    || ! hashmap_value_set( type_map, "float", ( void* )BOSL_OBJECT_TYPE_FLOAT )
    || ! hashmap_value_set( type_map, "string", ( void* )BOSL_OBJECT_TYPE_STRING )
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
 * @param value_type
 * @param object_type
 * @param data
 * @param size
 * @return
 */
bosl_object_t* bosl_object_allocate(
  bosl_object_value_type_t value_type,
  bosl_object_type_t object_type,
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
  o->type = object_type;
  o->environment = false;
  o->constant = false;
  o->is_return = false;
  o->is_break = false;
  o->is_continue = false;
  // copy over
  if ( BOSL_OBJECT_VALUE_BOOL == value_type ) {
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
 * @param closure
 * @return
 *
 * @todo add support for load statement
 */
bosl_object_t* bosl_object_allocate_callable(
  bosl_ast_statement_function_t* statement,
  bosl_callback_t callback,
  bosl_environment_t* closure
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
  o->value_type = BOSL_OBJECT_VALUE_CALLABLE;
  o->type = BOSL_OBJECT_TYPE_UNDEFINED;
  o->environment = false;
  o->constant = false;
  // populate nested data
  bosl_object_callable_t* callable = o->data;
  callable->statement = statement;
  callable->callback = callback;
  callable->closure = closure;
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
  bosl_object_t* duplicate = bosl_object_allocate(
    obj->value_type,
    obj->type,
    obj->data,
    obj->size
  );
  // handle error
  if ( ! duplicate ) {
    return NULL;
  }
  // copy over rest of necessary properties
  duplicate->constant = obj->constant;
  duplicate->is_return = obj->is_return;
  duplicate->is_break = obj->is_break;
  duplicate->is_continue = obj->is_continue;
  // return duplicate
  return duplicate;
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
    return BOSL_OBJECT_TYPE_UNDEFINED;
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
    case BOSL_OBJECT_TYPE_UINT8: return 0;
    case BOSL_OBJECT_TYPE_UINT16: return 0;
    case BOSL_OBJECT_TYPE_UINT32: return 0;
    case BOSL_OBJECT_TYPE_UINT64: return 0;
    case BOSL_OBJECT_TYPE_INT8: return INT8_MIN;
    case BOSL_OBJECT_TYPE_INT16: return INT16_MIN;
    case BOSL_OBJECT_TYPE_INT32: return INT32_MIN;
    case BOSL_OBJECT_TYPE_INT64: return INT64_MIN;
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
    case BOSL_OBJECT_TYPE_UINT8: return UINT8_MAX;
    case BOSL_OBJECT_TYPE_UINT16: return UINT16_MAX;
    case BOSL_OBJECT_TYPE_UINT32: return UINT32_MAX;
    case BOSL_OBJECT_TYPE_UINT64: return UINT64_MAX;
    case BOSL_OBJECT_TYPE_INT8: return INT8_MAX;
    case BOSL_OBJECT_TYPE_INT16: return INT16_MAX;
    case BOSL_OBJECT_TYPE_INT32: return INT32_MAX;
    case BOSL_OBJECT_TYPE_INT64: return INT64_MAX;
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
    case BOSL_OBJECT_TYPE_FLOAT: return LDBL_MIN;
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
    case BOSL_OBJECT_TYPE_FLOAT: return LDBL_MAX;
    default: return 0;
  }
}

/**
 * @brief Helper to assign / push a value
 *
 * @param environment
 * @param object
 * @param value
 * @param push
 * @return
 */
bool bosl_object_assign_push_value(
  bosl_environment_t* environment,
  bosl_token_t* name,
  bosl_token_t* type,
  bosl_object_t* value,
  bool push
) {
  char buffer[ 100 ];
  // variable for object type
  bosl_object_type_t object_type;
  // check for constant if not pushing a variable
  if ( ! push ) {
    // get current value
    bosl_object_t* current = bosl_environment_get_value( environment, name );
    // handle error
    if ( ! current ) {
      bosl_error_raise( name, "Variable not found." );
      return false;
    }
    // check for constant
    if ( current->constant ) {
      bosl_error_raise( name, "Change a constant is not allowed." );
      return false;
    }
    // use object type from current
    object_type = current->type;
  } else {
    // handle missing type name
    if ( ! type ) {
      bosl_error_raise( name, "Type token internally not passed." );
      return false;
    }
    // set variable
    object_type = bosl_object_str_to_type( type->start, type->length );
  }

  // handle string expected but no string
  if (
    BOSL_OBJECT_TYPE_STRING == object_type
    && BOSL_OBJECT_TYPE_STRING != value->type
  ) {
    bosl_error_raise( NULL, "Cannot assign non-string to string." );
    return false;
  }

  if ( object_type != value->type ) {
    // extract numbers
    int64_t snum;
    uint64_t unum;
    long double dnum;
    if ( ! bosl_object_extract_number( value, &unum, &snum, &dnum ) ) {
      bosl_error_raise( NULL, "Unable to extract value number." );
      return false;
    }

    // output just for testing
    //fprintf( stdout, "---------------------------------------\r\n%d => ", object_type );
    //fprintf( stdout, "%"PRId64" / %"PRIu64" / %Lf\r\n", snum, unum, dnum );
    // handle not equal types with additional checks

    // get min and max values for type checking
    __unused long double dmin = bosl_object_type_min_float_value( object_type );
    __unused long double dmax = bosl_object_type_max_float_value( object_type );
    __unused int64_t imin = bosl_object_type_min_int_value( object_type );
    __unused uint64_t imax = bosl_object_type_max_int_value( object_type );
    // perform validation depending on value type
    switch ( value->value_type ) {
      case BOSL_OBJECT_VALUE_FLOAT:
        break;
      case BOSL_OBJECT_VALUE_INT_SIGNED:
        break;
      case BOSL_OBJECT_VALUE_INT_UNSIGNED:
        break;
      case BOSL_OBJECT_VALUE_BOOL:
        break;
      case BOSL_OBJECT_VALUE_STRING:
        break;
      case BOSL_OBJECT_VALUE_NULL:
        break;
      default:
        sprintf( buffer, "Unable to assign type %d.\r\n", value->type );
        bosl_error_raise( NULL, buffer );
        return NULL;
    }
    // FIXME: DO SOME VALIDATION!
    //fprintf( stdout, "Do some assignment validation!\r\n" );
    //fprintf( stdout, "%Lf - %Lf, %"PRId64" - %"PRIu64"\r\n", dmin, dmax, imin, imax );
  }

  // set type of value
  value->type = object_type;
  // push value if set to true
  if ( push ) {
    return bosl_environment_push_value( environment, name, value );
  }
  // otherwise try to assign it
  return bosl_environment_assign_value( environment, name, value );
}

/**
 * @brief Helper to extract number from object
 *
 * @param object
 * @param unum
 * @param num
 * @param dnum
 * @return
 */
bool bosl_object_extract_number(
  bosl_object_t* object,
  uint64_t* unum,
  int64_t* num,
  long double* dnum
) {
  // only numbers are handled here
  if ( object->value_type > BOSL_OBJECT_VALUE_INT_UNSIGNED ) {
    bosl_error_raise( NULL, "Invalid object passed to extract_number." );
    return false;
  }
  // extract numbers
  switch ( object->value_type ) {
    case BOSL_OBJECT_VALUE_FLOAT:
      memcpy( dnum, object->data, object->size );
      break;
    case BOSL_OBJECT_VALUE_INT_UNSIGNED:
      memcpy( unum, object->data, object->size );
      break;
    case BOSL_OBJECT_VALUE_INT_SIGNED:
      memcpy( num, object->data, object->size );
      break;
    default:
      // should not happen due to if
      return false;
  }
  // return success
  return true;
}
