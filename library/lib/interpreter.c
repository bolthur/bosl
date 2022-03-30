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
#include <stdlib.h>
#include <stdio.h>
#include <inttypes.h>
#include "ast/common.h"
#include "ast/expression.h"
#include "ast/statement.h"
#include "interpreter.h"
#include "error.h"
#include "environment.h"

// necessary forward declarations
static bosl_interpreter_object_t* evaluate_expression( bosl_ast_expression_t* );

static bosl_interpreter_t* interpreter = NULL;

/**
 * @brief Helper to get previous ast statement
 *
 * @return
 */
static bosl_ast_statement_t* previous( void ) {
  return ( bosl_ast_statement_t* )( interpreter->_current->previous->data );
}

/**
 * @brief Helper to get current ast statement
 *
 * @return
 */
static bosl_ast_statement_t* current( void ) {
  return ( bosl_ast_statement_t* )( interpreter->_current->data );
}

/**
 * @brief Helper to get next ast statement
 *
 * @return
 */
static bosl_ast_statement_t* next( void ) {
  if ( interpreter->_current->next ) {
    interpreter->_current = interpreter->_current->next;
  }
  return interpreter->previous();
}

/**
 * @brief Destroy object wrapper to destroy if not from environment
 *
 * @param object
 */
static void destroy_object( bosl_interpreter_object_t* object ) {
  // handle invalid object passed
  if ( ! object ) {
    return;
  }
  // handle part of environment
  if ( object->environment ) {
    return;
  }
  // free object
  bosl_interpreter_destroy_object( object );
}

/**
 * @brief Wrapper for bosl error raise including set of error flag
 *
 * @param token
 * @param message
 */
static void raise_error( bosl_token_t* token, const char* message ) {
  bosl_error_raise( token, message );
  interpreter->error = true;
}

/**
 * @brief Convert object to string
 *
 * @param object
 * @return
 */
static char* stringify( bosl_interpreter_object_t* object ) {
  const char* buffer;
  // determine buffer size
  size_t buffer_size = sizeof( char );
  if (
    INTERPRETER_OBJECT_FLOAT == object->type
    || INTERPRETER_OBJECT_INT_SIGNED == object->type
    || INTERPRETER_OBJECT_INT_UNSIGNED == object->type
    || INTERPRETER_OBJECT_HEX_SIGNED == object->type
    || INTERPRETER_OBJECT_HEX_UNSIGNED == object->type
  ) {
    buffer_size *= 100;
  } else if (
    INTERPRETER_OBJECT_BOOL == object->type
  ) {
    buffer_size *= 6;
  } else if (
    INTERPRETER_OBJECT_NULL == object->type
  ) {
    buffer_size *= 5;
  } else if (
    INTERPRETER_OBJECT_STRING == object->type
  ) {
    buffer_size *= ( object->size + 1 );
  }
  // allocate buffer
  buffer = malloc( buffer_size );
  if ( ! buffer ) {
    return NULL;
  }
  memset( buffer, 0, buffer_size );
  if ( INTERPRETER_OBJECT_FLOAT == object->type ) {
    long double num;
    memcpy( &num, object->data, sizeof( num ) );
    sprintf( buffer, "%Lf", num );
  } else if ( INTERPRETER_OBJECT_INT_SIGNED == object->type ) {
    int64_t num;
    memcpy( &num, object->data, sizeof( num ) );
    sprintf( buffer, "%"PRId64, num );
  } else if ( INTERPRETER_OBJECT_INT_UNSIGNED == object->type ) {
    uint64_t num;
    memcpy( &num, object->data, sizeof( num ) );
    sprintf( buffer, "%"PRIu64, num );
  } else if ( INTERPRETER_OBJECT_HEX_SIGNED == object->type ) {
    int64_t num;
    memcpy( &num, object->data, sizeof( num ) );
    sprintf( buffer, "%"PRIx64, num );
  } else if ( INTERPRETER_OBJECT_HEX_UNSIGNED == object->type ) {
    uint64_t num;
    memcpy( &num, object->data, sizeof( num ) );
    sprintf( buffer, "%"PRIx64, num );
  } else if ( INTERPRETER_OBJECT_BOOL == object->type ) {
    bool flag;
    memcpy( &flag, object->data, sizeof( flag ) );
    sprintf( buffer, "%s", flag ? "true": "false" );
  } else if ( INTERPRETER_OBJECT_STRING == object->type ) {
    strncpy( buffer, object->data, object->size );
  } else if ( INTERPRETER_OBJECT_NULL == object->type ) {
    sprintf( buffer, "null" );
  }
  // return buffer
  return buffer;
}

/**
 * @brief Helper to check whether object is truthy value
 *
 * @param object
 * @return
 */
static bosl_interpreter_object_t* object_truthy(
  bosl_interpreter_object_t* object,
  bool negotiate
) {
  // handle invalid
  if ( ! object || ! object->data ) {
    raise_error( NULL, "Broken object passed to truthy." );
    return NULL;
  }
  bool flag = true;
  // handle null
  if ( INTERPRETER_OBJECT_NULL == object->type ) {
    flag = false;
  // evaluate boolean
  } else if ( INTERPRETER_OBJECT_BOOL == object->type ) {
    flag = *( ( bool* )( object->data ) );
  }
  // handle negotiation
  if ( negotiate ) {
    flag = ! flag;
  }
  // build and return object
  return bosl_interpreter_allocate_object(
    INTERPRETER_OBJECT_BOOL,
    &flag,
    sizeof( flag )
  );
}

/**
 * @brief Helper to check whether two objects are equal
 *
 * @param left
 * @param right
 * @param negotiate
 * @return
 */
static bosl_interpreter_object_t* object_equal(
  bosl_interpreter_object_t* left,
  bosl_interpreter_object_t* right,
  bool negotiate
) {
  // handle invalid
  if ( ! left || ! left->data || ! right || ! right->data ) {
    raise_error( NULL, "Broken objects passed to truthy." );
    return NULL;
  }
  bool flag = false;
  // handle both null
  if (
    INTERPRETER_OBJECT_NULL == left->type
    && INTERPRETER_OBJECT_NULL == right->type
  ) {
    flag = true;
  // handle comparison
  } else if ( left->type == right->type ) {
    if ( INTERPRETER_OBJECT_BOOL == left->type ) {
      flag = *( ( bool* )( left->data ) ) == *( ( bool* )( right->data ) );
    } else {
      flag = 0 == memcmp(
        left->data,
        right->data,
        right->size > left->size
          ? left->size
          : right->size
      );
    }
  }
  // handle negotiation
  if ( negotiate ) {
    flag = ! flag;
  }
  // return result
  return bosl_interpreter_allocate_object(
    INTERPRETER_OBJECT_BOOL,
    &flag,
    sizeof( flag )
  );
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
static bool extract_number(
  bosl_interpreter_object_t* object,
  uint64_t* unum,
  int64_t* num,
  long double* dnum
) {
  // only numbers are handled here
  if ( object->type > INTERPRETER_OBJECT_HEX_UNSIGNED ) {
    raise_error( NULL, "Invalid object passed to extract_number." );
    return false;
  }
  // extract numbers
  switch ( object->type ) {
    case INTERPRETER_OBJECT_FLOAT:
      memcpy( dnum, object->data, object->size );
      break;
    case INTERPRETER_OBJECT_INT_UNSIGNED:
    case INTERPRETER_OBJECT_HEX_UNSIGNED:
      memcpy( unum, object->data, object->size );
      break;
    case INTERPRETER_OBJECT_INT_SIGNED:
    case INTERPRETER_OBJECT_HEX_SIGNED:
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
 * @brief Helper to evaluate binary
 *
 * @param b
 * @return
 */
static bosl_interpreter_object_t* evaluate_binary( bosl_ast_expression_binary_t* b ) {
  // evaluate left
  bosl_interpreter_object_t* left = evaluate_expression( b->left );
  if ( ! left ) {
    raise_error( b->operator, "Unable to evaluate left expression" );
    return NULL;
  }
  // evaluate right
  bosl_interpreter_object_t* right = evaluate_expression( b->right );
  if ( ! right ) {
    raise_error( b->operator, "Unable to evaluate right expression" );
    destroy_object( left );
    return NULL;
  }
  // flag indicating different types
  bool same_type = left->type == right->type;
  // enforce same type and number
  if (
    TOKEN_BANG_EQUAL != b->operator->type
    && TOKEN_EQUAL_EQUAL != b->operator->type
    && ! same_type
  ) {
    // change right if left is signed
    if (
      INTERPRETER_OBJECT_HEX_SIGNED == left->type
      || INTERPRETER_OBJECT_INT_SIGNED == left->type
    ) {
      right->type = left->type;
      same_type = true;
    // change left if right is signed
    } else if (
      INTERPRETER_OBJECT_HEX_SIGNED == right->type
      || INTERPRETER_OBJECT_INT_SIGNED == right->type
    ) {
      left->type = right->type;
      same_type = true;
    } else {
      raise_error( b->operator, "Different types for binary." );
      destroy_object( left );
      destroy_object( right );
      return NULL;
    }
  }
  // variables for values
  int64_t lsnum = 0;
  uint64_t lunum = 0;
  long double ldnum = 0;
  int64_t rsnum = 0;
  uint64_t runum = 0;
  long double rdnum = 0;
  // extract stuff
  if (
    b->operator->type != TOKEN_EQUAL_EQUAL
    && b->operator->type != TOKEN_BANG_EQUAL
    && b->operator->type != TOKEN_BANG
    && (
      ! extract_number( left, &lunum, &lsnum, &ldnum )
      || ! extract_number( right, &runum, &rsnum, &rdnum )
    )
  ) {
    raise_error( b->operator, "Number extraction failed." );
    destroy_object( left );
    destroy_object( right );
    return NULL;
  }

  // apply operator
  if ( TOKEN_MINUS == b->operator->type ) {
    // save type and destroy objects
    bosl_interpreter_object_type_t type = left->type;
    destroy_object( left );
    destroy_object( right );
    // handle float
    if ( type == INTERPRETER_OBJECT_FLOAT ) {
      long double result = ldnum - rdnum;
      return bosl_interpreter_allocate_object(
        INTERPRETER_OBJECT_FLOAT,
        &result,
        sizeof( result )
      );
    }
    // handle unsigned int / hex
    if (
      type == INTERPRETER_OBJECT_INT_UNSIGNED
      || type == INTERPRETER_OBJECT_HEX_UNSIGNED
    ) {
      uint64_t result = lunum - runum;
      return bosl_interpreter_allocate_object(
        type,
        &result,
        sizeof( result )
      );
    }
    // handle signed int / hex
    if (
      type == INTERPRETER_OBJECT_INT_SIGNED
      || type == INTERPRETER_OBJECT_HEX_SIGNED
    ) {
      int64_t result = lsnum - rsnum;
      return bosl_interpreter_allocate_object(
        type,
        &result,
        sizeof( result )
      );
    }
    // unsupported
    raise_error( b->operator, "Unknown error" );
    return NULL;
  } else if ( TOKEN_PLUS == b->operator->type ) {
    // save type and destroy objects
    bosl_interpreter_object_type_t type = left->type;
    destroy_object( left );
    destroy_object( right );
    // handle float
    if ( type == INTERPRETER_OBJECT_FLOAT ) {
      long double result = ldnum + rdnum;
      return bosl_interpreter_allocate_object(
        INTERPRETER_OBJECT_FLOAT,
        &result,
        sizeof( result )
      );
    }
    // handle unsigned int / hex
    if (
      type == INTERPRETER_OBJECT_INT_UNSIGNED
      || type == INTERPRETER_OBJECT_HEX_UNSIGNED
    ) {
      uint64_t result = lunum + runum;
      return bosl_interpreter_allocate_object(
        type,
        &result,
        sizeof( result )
      );
    }
    // handle signed int / hex
    if (
      type == INTERPRETER_OBJECT_INT_SIGNED
      || type == INTERPRETER_OBJECT_HEX_SIGNED
    ) {
      int64_t result = lsnum + rsnum;
      return bosl_interpreter_allocate_object(
        type,
        &result,
        sizeof( result )
      );
    }
    // unsupported
    raise_error( b->operator, "Unknown error" );
    return NULL;
  } else if ( TOKEN_SLASH == b->operator->type ) {
    // save type and destroy objects
    bosl_interpreter_object_type_t type = left->type;
    destroy_object( left );
    destroy_object( right );
    // handle float
    if ( type == INTERPRETER_OBJECT_FLOAT ) {
      long double result = ldnum / rdnum;
      return bosl_interpreter_allocate_object(
        INTERPRETER_OBJECT_FLOAT,
        &result,
        sizeof( result )
      );
    }
    // handle unsigned int / hex
    if (
      type == INTERPRETER_OBJECT_INT_UNSIGNED
      || type == INTERPRETER_OBJECT_HEX_UNSIGNED
    ) {
      uint64_t result = lunum / runum;
      return bosl_interpreter_allocate_object(
        type,
        &result,
        sizeof( result )
      );
    }
    // handle signed int / hex
    if (
      type == INTERPRETER_OBJECT_INT_SIGNED
      || type == INTERPRETER_OBJECT_HEX_SIGNED
    ) {
      int64_t result = lsnum / rsnum;
      return bosl_interpreter_allocate_object(
        type,
        &result,
        sizeof( result )
      );
    }
    // unsupported
    raise_error( b->operator, "Unknown error" );
    return NULL;
  } else if ( TOKEN_STAR == b->operator->type ) {
    // save type and destroy objects
    bosl_interpreter_object_type_t type = left->type;
    destroy_object( left );
    destroy_object( right );
    // handle float
    if ( type == INTERPRETER_OBJECT_FLOAT ) {
      long double result = ldnum * rdnum;
      return bosl_interpreter_allocate_object(
        INTERPRETER_OBJECT_FLOAT,
        &result,
        sizeof( result )
      );
    }
    // handle unsigned int / hex
    if (
      type == INTERPRETER_OBJECT_INT_UNSIGNED
      || type == INTERPRETER_OBJECT_HEX_UNSIGNED
    ) {
      uint64_t result = lunum * runum;
      return bosl_interpreter_allocate_object(
        type,
        &result,
        sizeof( result )
      );
    }
    // handle signed int / hex
    if (
      type == INTERPRETER_OBJECT_INT_SIGNED
      || type == INTERPRETER_OBJECT_HEX_SIGNED
    ) {
      int64_t result = lsnum * rsnum;
      return bosl_interpreter_allocate_object(
        type,
        &result,
        sizeof( result )
      );
    }
    // unsupported
    raise_error( b->operator, "Unknown error" );
    return NULL;
  } else if ( TOKEN_GREATER == b->operator->type ) {
    // save type and destroy objects
    bosl_interpreter_object_type_t type = left->type;
    destroy_object( left );
    destroy_object( right );
    // handle float
    if ( type == INTERPRETER_OBJECT_FLOAT ) {
      bool result = ldnum > rdnum;
      return bosl_interpreter_allocate_object(
        INTERPRETER_OBJECT_BOOL, &result, sizeof( result ) );
    }
    // handle unsigned int / hex
    if (
      type == INTERPRETER_OBJECT_INT_UNSIGNED
      || type == INTERPRETER_OBJECT_HEX_UNSIGNED
    ) {
      bool result = lunum > runum;
      return bosl_interpreter_allocate_object(
        INTERPRETER_OBJECT_BOOL, &result, sizeof( result ) );
    }
    // handle signed int / hex
    if (
      type == INTERPRETER_OBJECT_INT_SIGNED
      || type == INTERPRETER_OBJECT_HEX_SIGNED
    ) {
      bool result = lsnum > rsnum;
      return bosl_interpreter_allocate_object(
        INTERPRETER_OBJECT_BOOL, &result, sizeof( result ) );
    }
    // unsupported
    raise_error( b->operator, "Unknown error" );
    return NULL;
  } else if ( TOKEN_GREATER_EQUAL == b->operator->type ) {
    // save type and destroy objects
    bosl_interpreter_object_type_t type = left->type;
    destroy_object( left );
    destroy_object( right );
    // handle float
    if ( type == INTERPRETER_OBJECT_FLOAT ) {
      bool result = ldnum >= rdnum;
      return bosl_interpreter_allocate_object(
        INTERPRETER_OBJECT_BOOL, &result, sizeof( result ) );
    }
    // handle unsigned int / hex
    if (
      type == INTERPRETER_OBJECT_INT_UNSIGNED
      || type == INTERPRETER_OBJECT_HEX_UNSIGNED
    ) {
      bool result = lunum >= runum;
      return bosl_interpreter_allocate_object(
        INTERPRETER_OBJECT_BOOL, &result, sizeof( result ) );
    }
    // handle signed int / hex
    if (
      type == INTERPRETER_OBJECT_INT_SIGNED
      || type == INTERPRETER_OBJECT_HEX_SIGNED
    ) {
      bool result = lsnum >= rsnum;
      return bosl_interpreter_allocate_object(
        INTERPRETER_OBJECT_BOOL, &result, sizeof( result ) );
    }
    // unsupported
    raise_error( b->operator, "Unknown error" );
    return NULL;
  } else if ( TOKEN_LESS == b->operator->type ) {
    // save type and destroy objects
    bosl_interpreter_object_type_t type = left->type;
    destroy_object( left );
    destroy_object( right );
    // handle float
    if ( type == INTERPRETER_OBJECT_FLOAT ) {
      bool result = ldnum < rdnum;
      return bosl_interpreter_allocate_object(
        INTERPRETER_OBJECT_BOOL, &result, sizeof( result ) );
    }
    // handle unsigned int / hex
    if (
      type == INTERPRETER_OBJECT_INT_UNSIGNED
      || type == INTERPRETER_OBJECT_HEX_UNSIGNED
    ) {
      bool result = lunum < runum;
      return bosl_interpreter_allocate_object(
        INTERPRETER_OBJECT_BOOL, &result, sizeof( result ) );
    }
    // handle signed int / hex
    if (
      type == INTERPRETER_OBJECT_INT_SIGNED
      || type == INTERPRETER_OBJECT_HEX_SIGNED
    ) {
      bool result = lsnum < rsnum;
      return bosl_interpreter_allocate_object(
        INTERPRETER_OBJECT_BOOL, &result, sizeof( result ) );
    }
    // unsupported
    raise_error( b->operator, "Unknown error" );
    return NULL;
  } else if ( TOKEN_LESS_EQUAL == b->operator->type ) {
    // save type and destroy objects
    bosl_interpreter_object_type_t type = left->type;
    destroy_object( left );
    destroy_object( right );
    // handle float
    if ( type == INTERPRETER_OBJECT_FLOAT ) {
      bool result = ldnum <= rdnum;
      return bosl_interpreter_allocate_object(
        INTERPRETER_OBJECT_BOOL, &result, sizeof( result ) );
    }
    // handle unsigned int / hex
    if (
      type == INTERPRETER_OBJECT_INT_UNSIGNED
      || type == INTERPRETER_OBJECT_HEX_UNSIGNED
    ) {
      bool result = lunum <= runum;
      return bosl_interpreter_allocate_object(
        INTERPRETER_OBJECT_BOOL, &result, sizeof( result ) );
    }
    // handle signed int / hex
    if (
      type == INTERPRETER_OBJECT_INT_SIGNED
      || type == INTERPRETER_OBJECT_HEX_SIGNED
    ) {
      bool result = lsnum <= rsnum;
      return bosl_interpreter_allocate_object(
        INTERPRETER_OBJECT_BOOL, &result, sizeof( result ) );
    }
    // unsupported
    raise_error( b->operator, "Unknown error" );
    return NULL;
  } else if ( TOKEN_BANG_EQUAL == b->operator->type ) {
    // get equality status
    bosl_interpreter_object_t* equality = object_equal( left, right, true );
    // destroy object
    destroy_object( left );
    destroy_object( right );
    // return result
    return equality;
  } else if ( TOKEN_EQUAL_EQUAL == b->operator->type ) {
    // get equality status
    bosl_interpreter_object_t* equality = object_equal( left, right, false );
    // destroy object
    destroy_object( left );
    destroy_object( right );
    // return result
    return equality;
  }
  // destroy object
  destroy_object( left );
  destroy_object( right );
  // raise error and return NULL
  raise_error( b->operator, "Unknown binary token." );
  // any thing else is an error
  return NULL;
}

/**
 * @brief Helper to evaluate unary
 *
 * @param u
 * @return
 */
static bosl_interpreter_object_t* evaluate_unary( bosl_ast_expression_unary_t* u ) {
  // evaluate right
  bosl_interpreter_object_t* right = evaluate_expression( u->right );
  if ( ! right ) {
    raise_error( u->operator, "Unable to evaluate right expression" );
    return NULL;
  }
  // apply operators
  if ( TOKEN_BANG == u->operator->type ) {
    // truthy flag
    bosl_interpreter_object_t* truthy = object_truthy( right, true );
    // free object
    destroy_object( right );
    // return result
    return truthy;
  } else if ( TOKEN_MINUS == u->operator->type ) {
    // validate type
    if ( right->type > INTERPRETER_OBJECT_HEX_UNSIGNED ) {
      // raise error and return NULL
      raise_error( u->operator, "Expect numeric" );
      return NULL;
    }
    // variables for values
    int64_t snum = 0;
    uint64_t unum = 0;
    long double dnum = 0;
    // extract stuff
    if ( ! extract_number( right, &unum, &snum, &dnum ) ) {
      raise_error( u->operator, "Runtime error unable to extract number" );
      destroy_object( right );
      return NULL;
    }
    bosl_interpreter_object_type_t type = right->type;
    destroy_object( right );
    if ( INTERPRETER_OBJECT_FLOAT == type ) {
      dnum = -dnum;
      return bosl_interpreter_allocate_object( type, &dnum, sizeof( dnum ) );
    } else if (
      INTERPRETER_OBJECT_INT_SIGNED == type
      || INTERPRETER_OBJECT_HEX_SIGNED == type
    ) {
      snum = -snum;
      return bosl_interpreter_allocate_object( type, &snum, sizeof( snum ) );
    } else if (
      INTERPRETER_OBJECT_INT_UNSIGNED == type
      || INTERPRETER_OBJECT_HEX_UNSIGNED == type
    ) {
      snum = -( ( int64_t )unum );
      return bosl_interpreter_allocate_object(
        INTERPRETER_OBJECT_INT_UNSIGNED == type
          ? INTERPRETER_OBJECT_INT_SIGNED
          : INTERPRETER_OBJECT_HEX_SIGNED,
        &snum,
        sizeof( snum )
      );
    }
    // just return right
    raise_error( u->operator, "Runtime error unknown" );
    return NULL;
  } else if ( TOKEN_PLUS == u->operator->type ) {
    // validate type
    if (
      right->type >= INTERPRETER_OBJECT_FLOAT
      && right->type <= INTERPRETER_OBJECT_HEX_UNSIGNED
    ) {
      // raise error and return NULL
      raise_error( u->operator, "Expect numeric" );
      return NULL;
    }
    // just return right
    return right;
  } else if ( TOKEN_BINARY_ONE_COMPLEMENT == u->operator->type ) {
    // validate type
    if (
      right->type >= INTERPRETER_OBJECT_INT_SIGNED
      && right->type <= INTERPRETER_OBJECT_HEX_UNSIGNED
    ) {
      // raise error and return NULL
      raise_error( u->operator, "Expect numeric integer" );
      return NULL;
    }
    // variables for values
    int64_t snum = 0;
    uint64_t unum = 0;
    long double dnum = 0;
    // extract stuff
    if ( ! extract_number( right, &unum, &snum, &dnum ) ) {
      raise_error( u->operator, "Runtime error unable to extract number" );
      destroy_object( right );
      return NULL;
    }
    bosl_interpreter_object_type_t type = right->type;
    destroy_object( right );
    if (
      INTERPRETER_OBJECT_INT_SIGNED == type
      || INTERPRETER_OBJECT_HEX_SIGNED == type
    ) {
      snum = ~snum;
      return bosl_interpreter_allocate_object( type, &snum, sizeof( snum ) );
    } else if (
      INTERPRETER_OBJECT_INT_UNSIGNED == type
      || INTERPRETER_OBJECT_HEX_UNSIGNED == type
    ) {
      unum = ~unum;
      return bosl_interpreter_allocate_object( type, &unum, sizeof( unum ) );
    }
    // just return right
    raise_error( u->operator, "Runtime error unknown" );
    return NULL;
  }
  // destroy right
  destroy_object( right );
  // raise error and return NULL
  raise_error( u->operator, "Unknown unary token." );
  // any thing else is an error
  return NULL;
}

/**
 * @brief Helper to evaluate given literal
 *
 * @param l
 * @return
 */
static bosl_interpreter_object_t* evaluate_literal( bosl_ast_expression_literal_t* l ) {
  // allocate object for return
  bosl_interpreter_object_t* obj = malloc( sizeof( bosl_interpreter_object_t ) );
  if ( ! obj ) {
    raise_error( NULL, "Unable to allocate object for literal." );
    return NULL;
  }
  // clearout
  memset( obj, 0, sizeof( bosl_interpreter_object_type_t ) );
  // populate type
  switch ( l->type ) {
    case EXPRESSION_LITERAL_TYPE_BOOL:
      obj->type = INTERPRETER_OBJECT_BOOL;
      break;
    case EXPRESSION_LITERAL_TYPE_NULL:
      obj->type = INTERPRETER_OBJECT_NULL;
      break;
    case EXPRESSION_LITERAL_TYPE_NUMBER_FLOAT:
      obj->type = INTERPRETER_OBJECT_FLOAT;
      break;
    case EXPRESSION_LITERAL_TYPE_NUMBER_HEX:
      obj->type = INTERPRETER_OBJECT_HEX_UNSIGNED;
      break;
    case EXPRESSION_LITERAL_TYPE_NUMBER_INT:
      obj->type = INTERPRETER_OBJECT_INT_UNSIGNED;
      break;
    case EXPRESSION_LITERAL_TYPE_STRING:
      obj->type = INTERPRETER_OBJECT_STRING;
      break;
  }
  // allocate
  obj->data = malloc( l->size );
  if ( ! obj->data ) {
    raise_error( NULL, "Unable to allocate object data for literal." );
    free( obj );
    return NULL;
  }
  // copy content
  if ( INTERPRETER_OBJECT_BOOL != obj->type ) {
    memcpy( obj->data, l->value, l->size );
  } else {
    *( ( bool* )( obj->data ) ) = *( ( bool* )( l->value ) );
  }
  // set size and environment property
  obj->size = l->size;
  obj->environment = false;
  // return built object
  return obj;
}

/**
 * @brief Evaluates given expression
 *
 * @param e
 * @return
 */
static bosl_interpreter_object_t* evaluate_expression( bosl_ast_expression_t* e ) {
  switch ( e->type ) {
    case EXPRESSION_ASSIGN: {
      // evaluate assignment expression
      bosl_interpreter_object_t* value = evaluate_expression( e->assign->value );
      if ( ! value ) {
        raise_error( e->assign->token, "Unable to evaluate assign expression." );
        return NULL;
      }
      // Duplicate if expression is from a variable to prevent accidental free
      // of a value that might be referenced somewhere else again.
      if ( value->environment ) {
        value = bosl_interpreter_allocate_object(
          value->type,
          value->data,
          value->size
        );
        if ( ! value ) {
          raise_error( e->assign->token, "Unable to duplicate environment object." );
          return NULL;
        }
      }
      // try to assign value
      if ( ! bosl_environment_assign_value(
        interpreter->env,
        e->assign->token,
        value
      ) ) {
        raise_error( e->assign->token, "Assignment failed." );
        return NULL;
      }
      // NULL return is okay here
      return NULL;
    }
    case EXPRESSION_BINARY: return evaluate_binary( e->binary );
    case EXPRESSION_CALL: {
      break;
    }
    case EXPRESSION_LOAD: {
      break;
    }
    case EXPRESSION_POINTER: {
      break;
    }
    // grouping is just a expression container
    case EXPRESSION_GROUPING: return evaluate_expression( e->grouping->expression );
    // literal evaluation
    case EXPRESSION_LITERAL: return evaluate_literal( e->literal );
    case EXPRESSION_LOGICAL: {
      // evaluate left side
      bosl_interpreter_object_t* left = evaluate_expression(
        e->logical->left
      );
      // handle error
      if ( ! left ) {
        raise_error( e->logical->operator, "Unable to evaluate left side." );
        return NULL;
      }
      bosl_interpreter_object_t* truthy = object_truthy( left, false );
      if ( ! truthy ) {
        raise_error( e->logical->operator, "Unable to allocate truthy object." );
        destroy_object( left );
        return NULL;
      }
      // bool pointer to access information
      bool* flag = truthy->data;
      // handle logical or
      if ( TOKEN_OR_OR == e->logical->operator->type && *flag ) {
        destroy_object( truthy );
        return left;
      } else if ( TOKEN_AND_AND == e->logical->operator->type && ! *flag ) {
        destroy_object( truthy );
        return left;
      }
      // destroy object
      destroy_object( truthy );
      destroy_object( left );
      // return evaluation of right side
      return evaluate_expression( e->logical->right );
    }
    case EXPRESSION_UNARY: return evaluate_unary( e->unary );
    case EXPRESSION_VARIABLE: return bosl_environment_get_value(
      interpreter->env,
      e->variable->name );
  }
  raise_error( NULL, "Unknown expression." );
  return NULL;
}

/**
 * @brief Helper to execute print
 *
 * @param s
 */
static void execute_print( bosl_ast_statement_t* s ) {
  // evaluate print expression
  bosl_interpreter_object_t* object = evaluate_expression(
    s->print->expression
  );
  // handle error
  if ( ! object ) {
    raise_error( NULL, "Evaluate of inner expression for print failed." );
    return;
  }
  // print via stringify
  char* str = stringify( object );
  if ( ! str ) {
    raise_error( NULL, "Stringify of evaluated object failed." );
    return;
  }
  // print string
  fprintf( stdout, "%s\r\n", str );
  // destroy object and string
  destroy_object( object );
  free( str );
}

/**
 * @brief Executes given statement
 *
 * @param s
 * @return
 */
static void execute( bosl_ast_statement_t* s ) {
  switch ( s->type ) {
    case STATEMENT_BLOCK: {
      // create new nested environment
      bosl_environment_t* inner = bosl_environment_init(
        interpreter->env );
      if ( ! inner ) {
        raise_error( NULL, "Unable to allocate nested environment." );
        break;
      }
      // backup current environment
      bosl_environment_t* previous = interpreter->env;
      // temporarily overwrite current
      interpreter->env = inner;
      // execute statement per statement
      list_item_t* current = s->block->statements->first;
      while ( current ) {
        // get statement
        bosl_ast_statement_t* statement = current->data;
        // execute it
        execute( statement );
        // handle error
        if ( interpreter->error ) {
          raise_error( NULL, "Unable to execute block statement." );
          break;
        }
        // get to next
        current = current->next;
      }
      // restore interpreter environment
      interpreter->env = previous;
      // destroy nested environment
      bosl_environment_free( inner );
      break;
    }
    // expressions are just evaluated
    case STATEMENT_EXPRESSION:
      evaluate_expression( s->expression->expression );
      break;
    // handled by function shouldn't appear alone
    case STATEMENT_PARAMETER:
      raise_error(
        s->parameter->name,
        "Parameter statement is standalone not possible."
      );
      break;
    case STATEMENT_FUNCTION: {
      break;
    }
    case STATEMENT_IF: {
      // evaluate condition
      bosl_interpreter_object_t* condition = evaluate_expression(
        s->if_else->if_condition );
      if ( ! condition ) {
        raise_error( NULL, "Unable to evaluate condition." );
        return;
      }
      // check condition for truthy
      bosl_interpreter_object_t* truthy = object_truthy( condition, false );
      if ( ! truthy ) {
        raise_error( NULL, "Unable to allocate truthy object." );
        destroy_object( condition );
        return;
      }
      // execute statements depending on condition
      if ( *( ( bool* )( truthy->data ) ) ) {
        execute( s->if_else->if_statement );
      } else {
        if ( s->if_else->else_statement ) {
          execute( s->if_else->else_statement );
        }
      }
      // destroy condition again
      destroy_object( condition );
      destroy_object( truthy );
      break;
    }
    case STATEMENT_PRINT:
      execute_print( s );
      break;
    case STATEMENT_RETURN: {
      break;
    }
    case STATEMENT_VARIABLE: {
      bosl_interpreter_object_t* value = NULL;
      // evaluate initializer
      if ( s->variable->initializer ) {
        value = evaluate_expression( s->variable->initializer );
        if ( ! value ) {
          raise_error(
            s->variable->name,
            "Unable to evaluate initializer expression."
          );
          break;
        }
      // default initializer null
      } else {
        const char n[] = "NULL";
        value = bosl_interpreter_allocate_object( INTERPRETER_OBJECT_NULL, n, strlen( n ) + 1 );
        if ( ! value ) {
          raise_error(
            s->variable->name,
            "Unable to allocate default initializer."
          );
          break;
        }
      }
      bosl_environment_push_value(
        interpreter->env, s->variable->name, value );
      break;
    }
    case STATEMENT_CONST: {
      break;
    }
    case STATEMENT_WHILE: {
      while ( true ) {
        // evaluate condition
        bosl_interpreter_object_t* condition = evaluate_expression(
          s->while_loop->condition );
        if ( ! condition ) {
          raise_error( NULL, "Unable to evaluate condition." );
          return;
        }
        // check condition for truthy
        bosl_interpreter_object_t* truthy = object_truthy( condition, false );
        if ( ! truthy ) {
          raise_error( NULL, "Unable to allocate truthy object." );
          destroy_object( condition );
          return;
        }
        // break if not true any longer
        if ( ! *( ( bool* )( truthy->data ) ) ) {
          destroy_object( truthy );
          destroy_object( condition );
          break;
        }
        // execute while body
        execute( s->while_loop->body );
        // destroy truthy and condition again
        destroy_object( truthy );
        destroy_object( condition );
      }
      break;
    }
    case STATEMENT_POINTER: {
      break;
    }
    default:
      raise_error( NULL, "Unknown ast statement" );
  }
}

/**
 * @brief Executed given node
 *
 * @param node
 * @return
 */
static void execute_ast_node( bosl_ast_node_t* node ) {
  // handle no statement
  if ( ! node->statement ) {
    raise_error( NULL, "Invalid ast node" );
    return;
  }
  // execute statement
  execute( node->statement );
}

/**
 * @brief Initialize interpreter
 *
 * @param ast
 * @return
 */
bool bosl_interpreter_init( list_manager_t* ast ) {
  // handle already initialized
  if ( interpreter ) {
    return true;
  }
  // allocate interpreter structure
  interpreter = malloc( sizeof( bosl_interpreter_t ) );
  if ( ! interpreter ) {
    return false;
  }
  // clear out
  memset( interpreter, 0, sizeof( bosl_interpreter_t ) );
  // allocate environment
  interpreter->env = bosl_environment_init( NULL );
  if ( ! interpreter->env ) {
    free( interpreter );
    return false;
  }
  // populate
  interpreter->_ast = ast;
  interpreter->_current = ast->first;
  interpreter->current = current;
  interpreter->next = next;
  interpreter->previous = previous;
  interpreter->error = false;
  // return success
  return true;
}

/**
 * @brief Free interpreter
 */
void bosl_interpreter_free( void ) {
  // handle not initialized
  if ( ! interpreter ) {
    return;
  }
  if ( interpreter->env ) {
    bosl_environment_free( interpreter->env );
  }
  // just free structure
  free( interpreter );
}

/**
 * @brief Perform interpreter
 *
 * @return
 */
bool bosl_interpreter_run( void ) {
  // start with first ast item
  list_item_t* current = interpreter->_ast->first;
  // loop as long as current is valid
  while( current ) {
    // execute
    execute_ast_node( current->data );
    // handle error
    if ( interpreter->error ) {
      fprintf( stderr, "Some interpreter error occurred!\r\n" );
      return false;
    }
    // get to next
    current = current->next;
  }
  // return success
  return true;
}

/**
 * @brief Destroy an interpreter object
 *
 * @param object
 */
void bosl_interpreter_destroy_object( bosl_interpreter_object_t* object ) {
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
 * @brief Allocate an interpreter object
 *
 * @param type
 * @param data
 * @param size
 * @return
 */
bosl_interpreter_object_t* bosl_interpreter_allocate_object(
  bosl_interpreter_object_type_t type,
  void* data,
  size_t size
) {
  // allocate object
  bosl_interpreter_object_t* o = malloc( sizeof( bosl_interpreter_object_t ) );
  if ( ! o ) {
    raise_error( NULL, "Object allocation failed." );
    return NULL;
  }
  // allocate data
  o->data = malloc( size );
  if ( ! o->data ) {
    raise_error( NULL, "Object data allocation failed." );
    bosl_interpreter_destroy_object( o );
    return NULL;
  }
  // populate data
  o->size = size;
  o->type = type;
  o->environment = false;
  // copy over
  if ( INTERPRETER_OBJECT_BOOL == type ) {
    *( ( bool* )( o->data ) ) = *( ( bool* )data );
  } else {
    memcpy( o->data, data, size );
  }
  // return object
  return o;
}
