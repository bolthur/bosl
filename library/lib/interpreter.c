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
#include "object.h"

// necessary forward declarations
static bosl_object_t* evaluate_expression( bosl_ast_expression_t* );
static bosl_object_t* execute_function( bosl_object_t*, list_manager_t* );

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
static void destroy_object( bosl_object_t* object ) {
  // handle invalid object passed
  if ( ! object ) {
    return;
  }
  // handle part of environment
  if ( object->environment ) {
    return;
  }
  // free object
  bosl_object_destroy( object );
}

/**
 * @brief Object list cleanup helper
 *
 * @param item
 */
static void object_list_cleanup( list_item_t* item ) {
  if ( ! item ) {
    return;
  }
  if ( ! item->data ) {
    return;
  }
  destroy_object( item->data );
  list_default_cleanup( item );
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
static char* stringify( bosl_object_t* object ) {
  const char* buffer;
  // determine buffer size
  size_t buffer_size = sizeof( char );
  if (
    OBJECT_VALUE_FLOAT == object->value_type
    || OBJECT_VALUE_INT_SIGNED == object->value_type
    || OBJECT_VALUE_INT_UNSIGNED == object->value_type
    || OBJECT_VALUE_HEX_SIGNED == object->value_type
    || OBJECT_VALUE_HEX_UNSIGNED == object->value_type
  ) {
    buffer_size *= 100;
  } else if (
    OBJECT_VALUE_BOOL == object->value_type
  ) {
    buffer_size *= 6;
  } else if (
    OBJECT_VALUE_NULL == object->value_type
  ) {
    buffer_size *= 5;
  } else if (
    OBJECT_VALUE_STRING == object->value_type
  ) {
    buffer_size *= ( object->size + 1 );
  } else if (
    OBJECT_VALUE_CALLABLE == object->value_type
  ) {
    buffer_size =
      ( ( bosl_object_callable_t* )object->data )->statement->token->length
      + 6;
  }
  // allocate buffer
  buffer = malloc( buffer_size );
  if ( ! buffer ) {
    return NULL;
  }
  memset( buffer, 0, buffer_size );
  if ( OBJECT_VALUE_FLOAT == object->value_type ) {
    long double num;
    memcpy( &num, object->data, sizeof( num ) );
    sprintf( buffer, "%Lf", num );
  } else if ( OBJECT_VALUE_INT_SIGNED == object->value_type ) {
    int64_t num;
    memcpy( &num, object->data, sizeof( num ) );
    sprintf( buffer, "%"PRId64, num );
  } else if ( OBJECT_VALUE_INT_UNSIGNED == object->value_type ) {
    uint64_t num;
    memcpy( &num, object->data, sizeof( num ) );
    sprintf( buffer, "%"PRIu64, num );
  } else if ( OBJECT_VALUE_HEX_SIGNED == object->value_type ) {
    int64_t num;
    memcpy( &num, object->data, sizeof( num ) );
    sprintf( buffer, "%"PRIx64, num );
  } else if ( OBJECT_VALUE_HEX_UNSIGNED == object->value_type ) {
    uint64_t num;
    memcpy( &num, object->data, sizeof( num ) );
    sprintf( buffer, "%"PRIx64, num );
  } else if ( OBJECT_VALUE_BOOL == object->value_type ) {
    bool flag;
    memcpy( &flag, object->data, sizeof( flag ) );
    sprintf( buffer, "%s", flag ? "true": "false" );
  } else if ( OBJECT_VALUE_STRING == object->value_type ) {
    strncpy( buffer, object->data, object->size );
  } else if ( OBJECT_VALUE_NULL == object->value_type ) {
    sprintf( buffer, "null" );
  } else if ( OBJECT_VALUE_CALLABLE == object->value_type ) {
    bosl_token_t* token = ( ( bosl_object_callable_t* )object->data )
      ->statement->token;
    sprintf(
      buffer, "<fn %*.*s>", ( int )token->length,
      ( int )token->length, token->start );
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
static bosl_object_t* object_truthy(
  bosl_object_t* object,
  bool negotiate
) {
  // handle invalid
  if ( ! object || ! object->data ) {
    raise_error( NULL, "Broken object passed to truthy." );
    return NULL;
  }
  bool flag = true;
  // handle null
  if ( OBJECT_VALUE_NULL == object->value_type ) {
    flag = false;
  // evaluate boolean
  } else if ( OBJECT_VALUE_BOOL == object->value_type ) {
    flag = *( ( bool* )( object->data ) );
  }
  // handle negotiation
  if ( negotiate ) {
    flag = ! flag;
  }
  // build and return object
  return bosl_object_allocate(
    OBJECT_VALUE_BOOL,
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
static bosl_object_t* object_equal(
  bosl_object_t* left,
  bosl_object_t* right,
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
    OBJECT_VALUE_NULL == left->value_type
    && OBJECT_VALUE_NULL == right->value_type
  ) {
    flag = true;
  // handle comparison
  } else if ( left->value_type == right->value_type ) {
    if ( OBJECT_VALUE_BOOL == left->value_type ) {
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
  return bosl_object_allocate(
    OBJECT_VALUE_BOOL,
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
  bosl_object_t* object,
  uint64_t* unum,
  int64_t* num,
  long double* dnum
) {
  // only numbers are handled here
  if ( object->value_type > OBJECT_VALUE_HEX_UNSIGNED ) {
    raise_error( NULL, "Invalid object passed to extract_number." );
    return false;
  }
  // extract numbers
  switch ( object->value_type ) {
    case OBJECT_VALUE_FLOAT:
      memcpy( dnum, object->data, object->size );
      break;
    case OBJECT_VALUE_INT_UNSIGNED:
    case OBJECT_VALUE_HEX_UNSIGNED:
      memcpy( unum, object->data, object->size );
      break;
    case OBJECT_VALUE_INT_SIGNED:
    case OBJECT_VALUE_HEX_SIGNED:
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
static bosl_object_t* evaluate_binary( bosl_ast_expression_binary_t* b ) {
  // evaluate left
  bosl_object_t* left = evaluate_expression( b->left );
  if ( ! left ) {
    raise_error( b->operator, "Unable to evaluate left expression" );
    return NULL;
  }
  // evaluate right
  bosl_object_t* right = evaluate_expression( b->right );
  if ( ! right ) {
    raise_error( b->operator, "Unable to evaluate right expression" );
    destroy_object( left );
    return NULL;
  }
  // flag indicating different types
  bool same_type = left->value_type == right->value_type;
  // enforce same type and number
  if (
    TOKEN_BANG_EQUAL != b->operator->type
    && TOKEN_EQUAL_EQUAL != b->operator->type
    && ! same_type
  ) {
    // change right if left is signed
    if (
      OBJECT_VALUE_HEX_SIGNED == left->value_type
      || OBJECT_VALUE_INT_SIGNED == left->value_type
    ) {
      right->value_type = left->value_type;
      same_type = true;
    // change left if right is signed
    } else if (
      OBJECT_VALUE_HEX_SIGNED == right->value_type
      || OBJECT_VALUE_INT_SIGNED == right->value_type
    ) {
      left->value_type = right->value_type;
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
    bosl_object_value_type_t type = left->value_type;
    destroy_object( left );
    destroy_object( right );
    // handle float
    if ( type == OBJECT_VALUE_FLOAT ) {
      long double result = ldnum - rdnum;
      return bosl_object_allocate(
        OBJECT_VALUE_FLOAT,
        &result,
        sizeof( result )
      );
    }
    // handle unsigned int / hex
    if (
      type == OBJECT_VALUE_INT_UNSIGNED
      || type == OBJECT_VALUE_HEX_UNSIGNED
    ) {
      uint64_t result = lunum - runum;
      return bosl_object_allocate(
        type,
        &result,
        sizeof( result )
      );
    }
    // handle signed int / hex
    if (
      type == OBJECT_VALUE_INT_SIGNED
      || type == OBJECT_VALUE_HEX_SIGNED
    ) {
      int64_t result = lsnum - rsnum;
      return bosl_object_allocate(
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
    bosl_object_value_type_t type = left->value_type;
    destroy_object( left );
    destroy_object( right );
    // handle float
    if ( type == OBJECT_VALUE_FLOAT ) {
      long double result = ldnum + rdnum;
      return bosl_object_allocate(
        OBJECT_VALUE_FLOAT,
        &result,
        sizeof( result )
      );
    }
    // handle unsigned int / hex
    if (
      type == OBJECT_VALUE_INT_UNSIGNED
      || type == OBJECT_VALUE_HEX_UNSIGNED
    ) {
      uint64_t result = lunum + runum;
      return bosl_object_allocate(
        type,
        &result,
        sizeof( result )
      );
    }
    // handle signed int / hex
    if (
      type == OBJECT_VALUE_INT_SIGNED
      || type == OBJECT_VALUE_HEX_SIGNED
    ) {
      int64_t result = lsnum + rsnum;
      return bosl_object_allocate(
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
    bosl_object_value_type_t type = left->value_type;
    destroy_object( left );
    destroy_object( right );
    // handle float
    if ( type == OBJECT_VALUE_FLOAT ) {
      long double result = ldnum / rdnum;
      return bosl_object_allocate(
        OBJECT_VALUE_FLOAT,
        &result,
        sizeof( result )
      );
    }
    // handle unsigned int / hex
    if (
      type == OBJECT_VALUE_INT_UNSIGNED
      || type == OBJECT_VALUE_HEX_UNSIGNED
    ) {
      uint64_t result = lunum / runum;
      return bosl_object_allocate(
        type,
        &result,
        sizeof( result )
      );
    }
    // handle signed int / hex
    if (
      type == OBJECT_VALUE_INT_SIGNED
      || type == OBJECT_VALUE_HEX_SIGNED
    ) {
      int64_t result = lsnum / rsnum;
      return bosl_object_allocate(
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
    bosl_object_value_type_t type = left->value_type;
    destroy_object( left );
    destroy_object( right );
    // handle float
    if ( type == OBJECT_VALUE_FLOAT ) {
      long double result = ldnum * rdnum;
      return bosl_object_allocate(
        OBJECT_VALUE_FLOAT,
        &result,
        sizeof( result )
      );
    }
    // handle unsigned int / hex
    if (
      type == OBJECT_VALUE_INT_UNSIGNED
      || type == OBJECT_VALUE_HEX_UNSIGNED
    ) {
      uint64_t result = lunum * runum;
      return bosl_object_allocate(
        type,
        &result,
        sizeof( result )
      );
    }
    // handle signed int / hex
    if (
      type == OBJECT_VALUE_INT_SIGNED
      || type == OBJECT_VALUE_HEX_SIGNED
    ) {
      int64_t result = lsnum * rsnum;
      return bosl_object_allocate(
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
    bosl_object_value_type_t type = left->value_type;
    destroy_object( left );
    destroy_object( right );
    // handle float
    if ( type == OBJECT_VALUE_FLOAT ) {
      bool result = ldnum > rdnum;
      return bosl_object_allocate(
        OBJECT_VALUE_BOOL, &result, sizeof( result ) );
    }
    // handle unsigned int / hex
    if (
      type == OBJECT_VALUE_INT_UNSIGNED
      || type == OBJECT_VALUE_HEX_UNSIGNED
    ) {
      bool result = lunum > runum;
      return bosl_object_allocate(
        OBJECT_VALUE_BOOL, &result, sizeof( result ) );
    }
    // handle signed int / hex
    if (
      type == OBJECT_VALUE_INT_SIGNED
      || type == OBJECT_VALUE_HEX_SIGNED
    ) {
      bool result = lsnum > rsnum;
      return bosl_object_allocate(
        OBJECT_VALUE_BOOL, &result, sizeof( result ) );
    }
    // unsupported
    raise_error( b->operator, "Unknown error" );
    return NULL;
  } else if ( TOKEN_GREATER_EQUAL == b->operator->type ) {
    // save type and destroy objects
    bosl_object_value_type_t type = left->value_type;
    destroy_object( left );
    destroy_object( right );
    // handle float
    if ( type == OBJECT_VALUE_FLOAT ) {
      bool result = ldnum >= rdnum;
      return bosl_object_allocate(
        OBJECT_VALUE_BOOL, &result, sizeof( result ) );
    }
    // handle unsigned int / hex
    if (
      type == OBJECT_VALUE_INT_UNSIGNED
      || type == OBJECT_VALUE_HEX_UNSIGNED
    ) {
      bool result = lunum >= runum;
      return bosl_object_allocate(
        OBJECT_VALUE_BOOL, &result, sizeof( result ) );
    }
    // handle signed int / hex
    if (
      type == OBJECT_VALUE_INT_SIGNED
      || type == OBJECT_VALUE_HEX_SIGNED
    ) {
      bool result = lsnum >= rsnum;
      return bosl_object_allocate(
        OBJECT_VALUE_BOOL, &result, sizeof( result ) );
    }
    // unsupported
    raise_error( b->operator, "Unknown error" );
    return NULL;
  } else if ( TOKEN_LESS == b->operator->type ) {
    // save type and destroy objects
    bosl_object_value_type_t type = left->value_type;
    destroy_object( left );
    destroy_object( right );
    // handle float
    if ( type == OBJECT_VALUE_FLOAT ) {
      bool result = ldnum < rdnum;
      return bosl_object_allocate(
        OBJECT_VALUE_BOOL, &result, sizeof( result ) );
    }
    // handle unsigned int / hex
    if (
      type == OBJECT_VALUE_INT_UNSIGNED
      || type == OBJECT_VALUE_HEX_UNSIGNED
    ) {
      bool result = lunum < runum;
      return bosl_object_allocate(
        OBJECT_VALUE_BOOL, &result, sizeof( result ) );
    }
    // handle signed int / hex
    if (
      type == OBJECT_VALUE_INT_SIGNED
      || type == OBJECT_VALUE_HEX_SIGNED
    ) {
      bool result = lsnum < rsnum;
      return bosl_object_allocate(
        OBJECT_VALUE_BOOL, &result, sizeof( result ) );
    }
    // unsupported
    raise_error( b->operator, "Unknown error" );
    return NULL;
  } else if ( TOKEN_LESS_EQUAL == b->operator->type ) {
    // save type and destroy objects
    bosl_object_value_type_t type = left->value_type;
    destroy_object( left );
    destroy_object( right );
    // handle float
    if ( type == OBJECT_VALUE_FLOAT ) {
      bool result = ldnum <= rdnum;
      return bosl_object_allocate(
        OBJECT_VALUE_BOOL, &result, sizeof( result ) );
    }
    // handle unsigned int / hex
    if (
      type == OBJECT_VALUE_INT_UNSIGNED
      || type == OBJECT_VALUE_HEX_UNSIGNED
    ) {
      bool result = lunum <= runum;
      return bosl_object_allocate(
        OBJECT_VALUE_BOOL, &result, sizeof( result ) );
    }
    // handle signed int / hex
    if (
      type == OBJECT_VALUE_INT_SIGNED
      || type == OBJECT_VALUE_HEX_SIGNED
    ) {
      bool result = lsnum <= rsnum;
      return bosl_object_allocate(
        OBJECT_VALUE_BOOL, &result, sizeof( result ) );
    }
    // unsupported
    raise_error( b->operator, "Unknown error" );
    return NULL;
  } else if ( TOKEN_BANG_EQUAL == b->operator->type ) {
    // get equality status
    bosl_object_t* equality = object_equal( left, right, true );
    // destroy object
    destroy_object( left );
    destroy_object( right );
    // return result
    return equality;
  } else if ( TOKEN_EQUAL_EQUAL == b->operator->type ) {
    // get equality status
    bosl_object_t* equality = object_equal( left, right, false );
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
static bosl_object_t* evaluate_unary( bosl_ast_expression_unary_t* u ) {
  // evaluate right
  bosl_object_t* right = evaluate_expression( u->right );
  if ( ! right ) {
    raise_error( u->operator, "Unable to evaluate right expression" );
    return NULL;
  }
  // apply operators
  if ( TOKEN_BANG == u->operator->type ) {
    // truthy flag
    bosl_object_t* truthy = object_truthy( right, true );
    // free object
    destroy_object( right );
    // return result
    return truthy;
  } else if ( TOKEN_MINUS == u->operator->type ) {
    // validate type
    if ( right->value_type > OBJECT_VALUE_HEX_UNSIGNED ) {
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
    bosl_object_value_type_t type = right->value_type;
    destroy_object( right );
    if ( OBJECT_VALUE_FLOAT == type ) {
      dnum = -dnum;
      return bosl_object_allocate( type, &dnum, sizeof( dnum ) );
    } else if (
      OBJECT_VALUE_INT_SIGNED == type
      || OBJECT_VALUE_HEX_SIGNED == type
    ) {
      snum = -snum;
      return bosl_object_allocate( type, &snum, sizeof( snum ) );
    } else if (
      OBJECT_VALUE_INT_UNSIGNED == type
      || OBJECT_VALUE_HEX_UNSIGNED == type
    ) {
      snum = -( ( int64_t )unum );
      return bosl_object_allocate(
        OBJECT_VALUE_INT_UNSIGNED == type
          ? OBJECT_VALUE_INT_SIGNED
          : OBJECT_VALUE_HEX_SIGNED,
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
      right->value_type >= OBJECT_VALUE_FLOAT
      && right->value_type <= OBJECT_VALUE_HEX_UNSIGNED
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
      right->value_type >= OBJECT_VALUE_INT_SIGNED
      && right->value_type <= OBJECT_VALUE_HEX_UNSIGNED
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
    bosl_object_value_type_t type = right->value_type;
    destroy_object( right );
    if (
      OBJECT_VALUE_INT_SIGNED == type
      || OBJECT_VALUE_HEX_SIGNED == type
    ) {
      snum = ~snum;
      return bosl_object_allocate( type, &snum, sizeof( snum ) );
    } else if (
      OBJECT_VALUE_INT_UNSIGNED == type
      || OBJECT_VALUE_HEX_UNSIGNED == type
    ) {
      unum = ~unum;
      return bosl_object_allocate( type, &unum, sizeof( unum ) );
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
static bosl_object_t* evaluate_literal( bosl_ast_expression_literal_t* l ) {
  // determine type
  bosl_object_value_type_t type;
  switch ( l->type ) {
    case EXPRESSION_LITERAL_TYPE_BOOL:
      type = OBJECT_VALUE_BOOL;
      break;
    case EXPRESSION_LITERAL_TYPE_NULL:
      type = OBJECT_VALUE_NULL;
      break;
    case EXPRESSION_LITERAL_TYPE_NUMBER_FLOAT:
      type = OBJECT_VALUE_FLOAT;
      break;
    case EXPRESSION_LITERAL_TYPE_NUMBER_HEX:
      type = OBJECT_VALUE_HEX_UNSIGNED;
      break;
    case EXPRESSION_LITERAL_TYPE_NUMBER_INT:
      type = OBJECT_VALUE_INT_UNSIGNED;
      break;
    case EXPRESSION_LITERAL_TYPE_STRING:
      type = OBJECT_VALUE_STRING;
      break;
    default:
      raise_error( NULL, "Unsupported object type in literal." );
      return NULL;
  }
  // allocate object
  return bosl_object_allocate( type, l->value, l->size );
}

/**
 * @brief Evaluates given expression
 *
 * @param e
 * @return
 */
static bosl_object_t* evaluate_expression( bosl_ast_expression_t* e ) {
  switch ( e->type ) {
    case EXPRESSION_ASSIGN: {
      /// FIXME: CHECK WHETHER CONSTANT SHALL BE SET OR NOT
      // evaluate expression and duplicate if from environment
      bosl_object_t* value = bosl_object_duplicate_environment(
        evaluate_expression( e->assign->value ) );
      // handle error
      if ( ! value ) {
        raise_error( e->assign->token, "Unable to evaluate assign expression." );
        return NULL;
      }
      // get current value
      bosl_object_t* current = bosl_environment_get_value(
        interpreter->env, e->assign->token );
      // handle error
      if ( ! current ) {
        raise_error( e->assign->token, "Variable not existing." );
        destroy_object( value );
        return NULL;
      }
      // check for constant
      if ( current->constant ) {
        raise_error( e->assign->token, "Change a constant is not allowed." );
        destroy_object( value );
        return NULL;
      }
      // try to assign value
      if ( ! bosl_environment_assign_value(
        interpreter->env,
        e->assign->token,
        value
      ) ) {
        raise_error( e->assign->token, "Assignment failed." );
        destroy_object( value );
        return NULL;
      }
      // NULL return is okay here
      return NULL;
    }
    case EXPRESSION_BINARY: return evaluate_binary( e->binary );
    case EXPRESSION_CALL: {
      // evaluate callee expression
      bosl_object_t* object = evaluate_expression(
        e->call->callee
      );
      // handle error
      if ( ! object ) {
        raise_error( e->call->paren, "Unable to evaluate callee expression." );
        return NULL;
      }
      // handle not a callable
      if ( OBJECT_VALUE_CALLABLE != object->value_type ) {
        raise_error( e->call->paren, "Not a callable function." );
        return NULL;
      }
      // extract callee information
      bosl_object_callable_t* callee = object->data;
      // build list of arguments
      list_manager_t* argument_list = list_construct(
        NULL, object_list_cleanup, NULL );
      if ( ! argument_list ) {
        raise_error( e->call->paren, "Unable to allocate list for arguments." );
        destroy_object( object );
        return NULL;
      }
      // evaluate arguments
      for (
        list_item_t* current = e->call->arguments->first;
        current;
        current = current->next
      ) {
        // evaluate argument
        bosl_object_t* argument = evaluate_expression( current->data );
        if ( ! argument ) {
          raise_error(
            e->call->paren, "Unable to evaluate parameter expression." );
          list_destruct( argument_list );
          destroy_object( object );
          return NULL;
        }
        argument = bosl_object_duplicate_environment( argument );
        if ( ! argument ) {
          raise_error(
            e->call->paren, "Unable to duplicate parameter object." );
          list_destruct( argument_list );
          destroy_object( object );
          return NULL;
        }
        // push back
        if ( ! list_push_back_data( argument_list, argument ) ) {
          raise_error(
            e->call->paren, "Unable to push back parameter object." );
          list_destruct( argument_list );
          destroy_object( object );
          return NULL;
        }
      }
      // get expected and passed parameters
      size_t expected = list_count_item( callee->statement->parameter );
      size_t passed = list_count_item( argument_list );
      // check amount of passed arguments
      if ( expected != passed ) {
        raise_error(
          e->call->paren,
          "Argument mismatch, to less or much parameters passed."
        );
        list_destruct( argument_list );
        destroy_object( object );
        return NULL;
      }
      // FIXME: ADD TYPE CHECK
      // call function
      bosl_object_t* result = callee->callback( object, argument_list );
      // cleanup
      argument_list->cleanup = list_default_cleanup;
      list_destruct( argument_list );
      // destroy object
      destroy_object( object );
      // return result
      return result;
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
      bosl_object_t* left = evaluate_expression(
        e->logical->left
      );
      // handle error
      if ( ! left ) {
        raise_error( e->logical->operator, "Unable to evaluate left side." );
        return NULL;
      }
      bosl_object_t* truthy = object_truthy( left, false );
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
  bosl_object_t* object = evaluate_expression(
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
static bosl_object_t* execute( bosl_ast_statement_t* s ) {
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
        bosl_object_t* r = execute( statement );
        // handle return
        /// FIXME: CHECK FOR ACTIVE FUNCTION!
        if ( r && r->is_return ) {
          // duplicate return if environment variable
          bosl_object_t* copy = bosl_object_duplicate_environment( r );
          if ( ! copy ) {
            raise_error( NULL, "Unable to duplicate return object.\r\n" );
            break;
          }
          // restore previous environment
          interpreter->env = previous;
          // destroy nested environment
          bosl_environment_free( inner );
          // return object
          return copy;
        }
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
      // allocate callable object
      bosl_object_t* f = bosl_object_allocate_callable(
        s->function, execute_function );
      if ( ! f ) {
        raise_error(
          s->function->token,
          "Unable to allocate function object."
        );
        break;
      }
      // push to environment
      bosl_environment_push_value( interpreter->env, s->function->token, f );
      break;
    }
    case STATEMENT_IF: {
      // evaluate condition
      bosl_object_t* condition = evaluate_expression(
        s->if_else->if_condition );
      if ( ! condition ) {
        raise_error( NULL, "Unable to evaluate condition." );
        break;
      }
      // check condition for truthy
      bosl_object_t* truthy = object_truthy( condition, false );
      if ( ! truthy ) {
        raise_error( NULL, "Unable to allocate truthy object." );
        destroy_object( condition );
        break;
      }
      bosl_object_t* r = NULL;
      // execute statements depending on condition
      if ( *( ( bool* )( truthy->data ) ) ) {
        r = execute( s->if_else->if_statement );
      } else {
        if ( s->if_else->else_statement ) {
          r = execute( s->if_else->else_statement );
        }
      }
      // destroy condition again
      destroy_object( condition );
      destroy_object( truthy );
      if ( r && r->is_return ) {
        bosl_object_t* copy = bosl_object_duplicate_environment( r );
        if ( ! copy ) {
          raise_error( NULL, "Unable to duplicate return object.\r\n" );
          break;
        }
        return copy;
      }
      break;
    }
    case STATEMENT_PRINT:
      execute_print( s );
      break;
    case STATEMENT_RETURN: {
      bosl_object_t* value= NULL;
      if ( s->return_value->value ) {
        // evaluate expression
        value = evaluate_expression( s->return_value->value );
        if ( ! value ) {
          raise_error(
            s->return_value->keyword,
            "Unable to evaluate return expression."
          );
        }
      } else {
        // build null return
        const char n[] = "NULL";
        value = bosl_object_allocate( OBJECT_VALUE_NULL, n, strlen( n ) + 1 );
        if ( ! value ) {
          raise_error(
            s->return_value->keyword,
            "Unable to allocate return object."
          );
          return NULL;
        }
      }
      // duplicate if environment variable
      bosl_object_t* copy = bosl_object_duplicate_environment( value );
      if ( ! copy ) {
        raise_error( NULL, "Unable to duplicate return object.\r\n" );
        break;
      }
      // set return flag
      copy->is_return = true;
      return copy;
    }
    case STATEMENT_VARIABLE: {
      bosl_object_t* value = NULL;
      // evaluate initializer
      if ( s->variable->initializer ) {
        value = bosl_object_duplicate_environment(
          evaluate_expression( s->variable->initializer )
        );
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
        value = bosl_object_allocate(
          OBJECT_VALUE_NULL, n, strlen( n ) + 1 );
        if ( ! value ) {
          raise_error(
            s->variable->name,
            "Unable to allocate default initializer."
          );
          break;
        }
      }
      // push to environment
      bosl_environment_push_value( interpreter->env, s->variable->name, value );
      break;
    }
    case STATEMENT_CONST: {
      // evaluate initializer
      bosl_object_t* value = bosl_object_duplicate_environment(
        evaluate_expression( s->variable->initializer )
      );
      // handle error
      if ( ! value ) {
        raise_error(
          s->variable->name,
          "Unable to evaluate initializer expression."
        );
        break;
      }
      // set constant flag
      value->constant = true;
      // push to environment
      bosl_environment_push_value( interpreter->env, s->variable->name, value );
      break;
    }
    case STATEMENT_WHILE: {
      while ( true ) {
        // evaluate condition
        bosl_object_t* condition = evaluate_expression(
          s->while_loop->condition );
        // handle error
        if ( ! condition ) {
          raise_error( NULL, "Unable to evaluate condition." );
          break;
        }
        // check condition for truthy
        bosl_object_t* truthy = object_truthy( condition, false );
        if ( ! truthy ) {
          raise_error( NULL, "Unable to allocate truthy object." );
          destroy_object( condition );
          break;
        }
        // break if not true any longer
        if ( ! *( ( bool* )( truthy->data ) ) ) {
          destroy_object( truthy );
          destroy_object( condition );
          break;
        }
        // execute while body
        bosl_object_t* r = execute( s->while_loop->body );
        // destroy truthy and condition again
        destroy_object( truthy );
        destroy_object( condition );
        // handle return
        if ( r && r->is_return ) {
          bosl_object_t* copy = bosl_object_duplicate_environment( r );
          if ( ! copy ) {
            raise_error( NULL, "Unable to duplicate return object.\r\n" );
            break;
          }
          return copy;
        }
      }
      break;
    }
    case STATEMENT_POINTER: {
      break;
    }
    default:
      raise_error( NULL, "Unknown ast statement" );
  }
  return NULL;
}

/**
 * @brief Helper to execute a script function
 *
 * @param object
 * @param parameter
 * @return
 */
static bosl_object_t* execute_function(
  bosl_object_t* object,
  list_manager_t* parameter
) {
  // create new closure environment
  bosl_environment_t* closure = bosl_environment_init( interpreter->global );
  if ( ! closure ) {
    raise_error( NULL, "Unable to allocate closure for function execution." );
    return NULL;
  }
  // backup current environment
  bosl_environment_t* previous = interpreter->env;
  // temporarily overwrite current
  interpreter->env = closure;
  // FIXME: ENSURE CORRECT VALUE TYPE
  // extract callable
  bosl_object_callable_t* callable = object->data;
  // push parameter to environment
  for ( size_t index = 0, max = list_count_item( parameter ); index < max; index++ ) {
    // get argument and value from list
    list_item_t* argument_item = list_get_item_at_pos(
      callable->statement->parameter, index );
    if ( ! argument_item ) {
      // restore interpreter environment
      interpreter->env = previous;
      // destroy closure
      bosl_environment_free( closure );
      raise_error( NULL, "Unable to get parameter name from callable." );
      return NULL;
    }
    // get value from list
    list_item_t* value_item = list_get_item_at_pos( parameter, index );
    if ( ! value_item ) {
      // restore interpreter environment
      interpreter->env = previous;
      // destroy closure
      bosl_environment_free( closure );
      raise_error( NULL, "Unable to get parameter value for callable." );
      return NULL;
    }
    // extract values
    bosl_ast_statement_t* argument = argument_item->data;
    bosl_object_t* value = value_item->data;
    // push to environment
    bosl_environment_push_value( closure, argument->parameter->name, value );
  }
  // execute function
  bosl_object_t* o = execute( callable->statement->body );
  // handle return
  if ( o && o->is_return ) {
    // duplicate if environment object
    bosl_object_t* copy = bosl_object_duplicate_environment( o );
    if ( ! copy ) {
      // restore interpreter environment
      interpreter->env = previous;
      // destroy closure
      bosl_environment_free( closure );
      raise_error( NULL, "Unable to duplicate return object after function." );
      return NULL;
    }
    o = copy;
  }
  // restore interpreter environment
  interpreter->env = previous;
  // destroy closure
  bosl_environment_free( closure );
  // return copy
  return o;
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
  interpreter->global = interpreter->env;
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
