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
 * @brief object type to string
 *
 * @param type
 * @return
 */
const char* bosl_object_type_to_str( bosl_object_type_t type ) {
  // setup hashmap iterator
  hashmap_iterator_t it = hashmap_iterator( type_map );
  // convert type to void pointer
  void* vtype = ( void* )type;
  // loop through hashmap
  while ( hashmap_next( &it ) ) {
    if ( vtype == it.value ) {
      return it.key;
    }
  }
  return NULL;
}

/**
 * @brief Helper to check whether value fits into float
 *
 * @param name
 * @param value
 * @param object_type
 * @return
 */
static bool value_fits_float(
  bosl_token_t* name,
  bosl_object_t* value,
  bosl_object_type_t object_type
) {
  // extract numbers
  int64_t snum;
  uint64_t unum;
  long double dnum;
  if ( ! bosl_object_extract_number( value, &unum, &snum, &dnum ) ) {
    bosl_error_raise( name, "Unable to extract value number." );
    return false;
  }
  // uint to double
  if (
    BOSL_OBJECT_TYPE_UINT8 <= value->type
    && BOSL_OBJECT_TYPE_UINT64 >= value->type
  ) {
    // test whether value can be stored savely by converting with conversion
    // back and comparison
    long double repr;
    *( ( volatile long double* )&repr ) = ( long double )unum;
    uint64_t round_trip_value = ( uint64_t )repr;
    // handle no assign possible
    if ( round_trip_value != unum ) {
      bosl_error_raise(
        name, "Cannot assign value %"PRIu64" with type %s to %s "
        "( cannot be converted savely ).", unum,
        bosl_object_type_to_str( value->type ),
        bosl_object_type_to_str( object_type )
      );
      return false;
    }
  // int to double
  } else {
    // test whether value can be stored savely by converting with conversion
    // back and comparison
    long double repr;
    *( ( volatile long double* )&repr ) = ( long double )snum;
    int64_t round_trip_value = ( int64_t )repr;
    // handle no assign possible
    if ( round_trip_value != snum ) {
      bosl_error_raise(
        name, "Cannot assign value %"PRId64" with type %s to %s "
        "( cannot be converted savely ).", unum,
        bosl_object_type_to_str( value->type ),
        bosl_object_type_to_str( object_type )
      );
      return false;
    }
  }
  return true;
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

  // Check usual incompatibilites
  if (
    // handle string expected but no string in value
    (
      BOSL_OBJECT_TYPE_STRING == object_type
      && BOSL_OBJECT_TYPE_STRING != value->type
    // handle integer expected but decimal received
    ) || (
      BOSL_OBJECT_TYPE_UINT8 <= object_type
      && BOSL_OBJECT_TYPE_INT64 >= object_type
      && (
        BOSL_OBJECT_TYPE_BOOL == value->type
        || BOSL_OBJECT_TYPE_FLOAT == value->type
        || BOSL_OBJECT_TYPE_STRING == value->type
      )
    )
  ) {
    bosl_error_raise(
      name, "Cannot assign %s to %s.",
      bosl_object_type_to_str( value->type ),
      bosl_object_type_to_str( object_type )
    );
    return false;
  }

  // check whether it's possible to convert between types
  if ( object_type != value->type ) {
    // extract numbers
    int64_t snum;
    uint64_t unum;
    long double dnum;
    if ( ! bosl_object_extract_number( value, &unum, &snum, &dnum ) ) {
      bosl_error_raise( name, "Unable to extract value number." );
      return false;
    }

    // signed / unsigned integer to float conversion
    if (
      BOSL_OBJECT_TYPE_FLOAT == object_type
      && BOSL_OBJECT_TYPE_UINT8 <= value->type
      && BOSL_OBJECT_TYPE_INT64 >= value->type
    ) {
      if ( ! value_fits_float( name, value, object_type ) ) {
        return NULL;
      }
      // allocate new data
      void* new_data = malloc( sizeof( long double ) );
      if ( ! new_data ) {
        bosl_error_raise(
          name, "Not enough memory for object type %s.",
          bosl_object_type_to_str( object_type )
        );
        return NULL;
      }
      // clear out
      memset( new_data, 0, sizeof( long double ) );
      // transform to bool
      long double* new_val = new_data;
      if (
        BOSL_OBJECT_TYPE_UINT8 <= value->type
        && BOSL_OBJECT_TYPE_UINT64 >= value->type
      ) {
        *new_val = ( long double )unum;
      } else {
        *new_val = ( long double )dnum;
      }
      // free current data
      free( value->data );
      // set to new data
      value->data = new_data;
      value->size = sizeof( long double );
    } else if (
      BOSL_OBJECT_TYPE_UINT8 <= object_type
      && BOSL_OBJECT_TYPE_INT64 >= object_type
      && BOSL_OBJECT_TYPE_UINT8 <= value->type
      && BOSL_OBJECT_TYPE_INT64 >= value->type
    ) {
      bosl_object_type_t backup = value->type;
      // transform value to string
      char* value_str = bosl_object_stringify( value );
      if ( ! value_str ) {
        // raise error
        bosl_error_raise( name, "Not enough memory for stringify value." );
        // return null
        return NULL;
      }
      // backup type
      backup = value->type;
      // temporary change type to possible new type
      value->type = object_type;
      // convert to string
      char* converted_str = bosl_object_stringify( value );
      // restore type
      value->type = backup;
      // handle error
      if ( ! converted_str ) {
        // free string
        free( value_str );
        // raise error
        bosl_error_raise( name, "Not enough memory for stringify value." );
        // return null
        return NULL;
      }
      // treat different string lengths or comparison mismatch as incompatible
      if (
        strlen( value_str ) != strlen( converted_str )
        || 0 != strcmp( value_str, converted_str )
      ) {
        // raise error
        bosl_error_raise(
          name, "Range error: %s is not in range of type %s.",
          value_str,
          bosl_object_type_to_str( object_type )
        );
        // free strings
        free( value_str );
        free( converted_str );
        // return null
        return NULL;
      }
      // free strings again
      free( value_str );
      free( converted_str );
    }
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

/**
 * @brief Convert object to string
 *
 * @param object
 * @return
 */
char* bosl_object_stringify( bosl_object_t* object ) {
  const char* buffer;
  // determine buffer size
  size_t buffer_size = sizeof( char );
  char count_buffer[ 8 ];
  int ndigits = 0;
  // variables for numbers
  long double dnum;
  int64_t snum;
  uint64_t unum;
  bool flag;
  bosl_token_t* token;
  // count by utilizing snprintf
  switch ( object->type ) {
    case BOSL_OBJECT_TYPE_BOOL:
      memcpy( &flag, object->data, sizeof( flag ) );
      ndigits = ( flag ? strlen( "true" ) : strlen( "false" ) );
      break;
    case BOSL_OBJECT_TYPE_UINT8: {
      // copy number
      memcpy( &unum, object->data, sizeof( unum ) );
      // enforce 8 bit unsigned integer
      unum = ( uint8_t )unum;
      // determine digits
      ndigits = snprintf( count_buffer, 7, "%"PRIu8, ( uint8_t )unum );
      break;
    }
    case BOSL_OBJECT_TYPE_UINT16: {
      // copy number
      memcpy( &unum, object->data, sizeof( unum ) );
      // enforce 16 bit unsigned integer
      unum = ( uint16_t )unum;
      // determine digits
      ndigits = snprintf( count_buffer, 7, "%"PRIu16, ( uint16_t )unum );
      break;
    }
    case BOSL_OBJECT_TYPE_UINT32: {
      // copy number
      memcpy( &unum, object->data, sizeof( unum ) );
      // enforce 32 bit unsigned integer
      unum = ( uint32_t )unum;
      // determine digits
      ndigits = snprintf( count_buffer, 7, "%"PRIu32, ( uint32_t )unum );
      break;
    }
    case BOSL_OBJECT_TYPE_UINT64: {
      memcpy( &unum, object->data, sizeof( unum ) );
      // determine digits
      ndigits = snprintf( count_buffer, 7, "%"PRIu64, unum );
      break;
    }
    case BOSL_OBJECT_TYPE_INT8: {
      // copy number
      memcpy( &snum, object->data, sizeof( snum ) );
      // enforce 8 bit signed integer
      snum = ( int8_t )snum;
      // determine digits
      ndigits = snprintf( count_buffer, 7, "%"PRId8, ( int8_t )snum );
      break;
    }
    case BOSL_OBJECT_TYPE_INT16: {
      // copy number
      memcpy( &snum, object->data, sizeof( snum ) );
      // enforce 16 bit signed integer
      snum = ( int16_t )snum;
      // determine digits
      ndigits = snprintf( count_buffer, 7, "%"PRId16, ( int16_t )snum );
      break;
    }
    case BOSL_OBJECT_TYPE_INT32: {
      // copy number
      memcpy( &snum, object->data, sizeof( snum ) );
      // enforce 32 bit signed integer
      snum = ( int32_t )snum;
      // determine digits
      ndigits = snprintf( count_buffer, 7, "%"PRId32, ( int32_t )snum );
      break;
    }
    case BOSL_OBJECT_TYPE_INT64: {
      // copy number
      memcpy( &snum, object->data, sizeof( snum ) );
      // determine digits
      ndigits = snprintf( count_buffer, 7, "%"PRId64, snum );
      break;
    }
    case BOSL_OBJECT_TYPE_STRING:
      ndigits = ( int )object->size;
      break;
    case BOSL_OBJECT_TYPE_FLOAT:
      memcpy( &dnum, object->data, sizeof( dnum ) );
      ndigits = snprintf( count_buffer, 7, "%Lf", dnum );
      break;
    case BOSL_OBJECT_TYPE_UNDEFINED:
      if ( BOSL_OBJECT_VALUE_NULL == object->value_type ) {
        ndigits = strlen( "null" );
      } else if ( BOSL_OBJECT_VALUE_CALLABLE == object->value_type ) {
        token = ( ( bosl_object_callable_t* )object->data )->statement->token;
        int length = ( int )token->length;
        ndigits = snprintf( count_buffer, 7, "<fn %*.*s>", length, length, token->start );
      }
      break;
    default: return NULL;
  }
  // handle error
  if ( 0 >= ndigits ) {
    return NULL;
  }
  // multiply buffer size with digit amount
  buffer_size *= ( size_t )( ndigits + 1 );
  // allocate buffer
  buffer = malloc( buffer_size );
  if ( ! buffer ) {
    return NULL;
  }
  // clear out
  memset( buffer, 0, buffer_size );
  // print to allocated buffer
  switch ( object->type ) {
    case BOSL_OBJECT_TYPE_BOOL:
      sprintf( buffer, "%s", flag ? "true": "false" );
      break;
    case BOSL_OBJECT_TYPE_UINT8:
      sprintf( buffer, "%"PRIu8, ( uint8_t )unum );
      break;
    case BOSL_OBJECT_TYPE_UINT16:
      sprintf( buffer, "%"PRIu16, ( uint16_t )unum );
      break;
    case BOSL_OBJECT_TYPE_UINT32:
      sprintf( buffer, "%"PRIu32, ( uint32_t )unum );
      break;
    case BOSL_OBJECT_TYPE_UINT64:
      sprintf( buffer, "%"PRIu64, unum );
      break;
    case BOSL_OBJECT_TYPE_INT8:
      sprintf( buffer, "%"PRId8, ( int8_t )snum );
      break;
    case BOSL_OBJECT_TYPE_INT16:
      sprintf( buffer, "%"PRId16, ( int16_t )snum );
      break;
    case BOSL_OBJECT_TYPE_INT32:
      sprintf( buffer, "%"PRId32, ( int32_t )snum );
      break;
    case BOSL_OBJECT_TYPE_INT64:
      sprintf( buffer, "%"PRId64, snum );
      break;
    case BOSL_OBJECT_TYPE_STRING:
      strncpy( buffer, object->data, object->size );
      break;
    case BOSL_OBJECT_TYPE_FLOAT:
      sprintf( buffer, "%Lf", dnum );
      break;
    case BOSL_OBJECT_TYPE_UNDEFINED:
      if ( BOSL_OBJECT_VALUE_NULL == object->value_type ) {
        sprintf( buffer, "null" );
      } else if ( BOSL_OBJECT_VALUE_CALLABLE == object->value_type ) {
        sprintf(
          buffer, "<fn %*.*s>", ( int )token->length,
          ( int )token->length, token->start
        );
      } else {
        free( buffer );
        return NULL;
      }
      break;
    default:
      free( buffer );
      return NULL;
  }
  // return buffer
  return buffer;
}
