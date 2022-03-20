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
#include "expression.h"

/**
 * @brief Method to allocate ast expression
 *
 * @param type
 * @return
 */
void* bosl_ast_expression_allocate( bosl_ast_expression_type_t type ) {
  size_t allocated_size = 0;
  void* inner_block = NULL;
  // allocate container
  bosl_ast_expression_t* expression = malloc( sizeof( bosl_ast_expression_t ) );
  if ( ! expression ) {
    return NULL;
  }
  // clear out
  memset( expression, 0, sizeof( bosl_ast_expression_t ) );
  // determine space for inner
  switch ( type ) {
    case EXPRESSION_ASSIGN:
      allocated_size = sizeof( bosl_ast_expression_assign_t );
      break;
    case EXPRESSION_BINARY:
      allocated_size = sizeof( bosl_ast_expression_binary_t );
      break;
    case EXPRESSION_CALL:
      allocated_size = sizeof( bosl_ast_expression_call_t );
      break;
    case EXPRESSION_LOAD:
      allocated_size = sizeof( bosl_ast_expression_load_t );
      break;
    case EXPRESSION_GROUPING:
      allocated_size = sizeof( bosl_ast_expression_grouping_t );
      break;
    case EXPRESSION_LITERAL:
      allocated_size = sizeof( bosl_ast_expression_literal_t );
      break;
    case EXPRESSION_LOGICAL:
      allocated_size = sizeof( bosl_ast_expression_logical_t );
      break;
    case EXPRESSION_UNARY:
      allocated_size = sizeof( bosl_ast_expression_unary_t );
      break;
    case EXPRESSION_VARIABLE:
      allocated_size = sizeof( bosl_ast_expression_variable_t );
      break;
  }
  // handle error
  if ( ! allocated_size ) {
    free( expression );
    return NULL;
  }
  // allocate inner structure
  inner_block = malloc( allocated_size );
  if ( ! inner_block ) {
    free( expression );
    return NULL;
  }
  memset( inner_block, 0, allocated_size );
  // set expression content finally
  expression->type = type;
  expression->data = inner_block;
  expression->size = allocated_size;
  // return built expression
  return expression;
}

/**
 * @brief Helper to allocate and populate binary expression
 *
 * @param left
 * @param right
 * @return
 */
bosl_ast_expression_t* bosl_ast_expression_allocate_binary(
  bosl_ast_expression_t* left,
  bosl_token_t* operator,
  bosl_ast_expression_t* right
) {
  bosl_ast_expression_t* e = bosl_ast_expression_allocate( EXPRESSION_BINARY );
  if ( ! e ) {
    // FIXME: DESTROY e and right
    return NULL;
  }
  // get inner data
  bosl_ast_expression_binary_t* binary = e->data;
  // populate data
  binary->left = left;
  binary->operator = operator;
  binary->right = right;
  // return expression
  return e;
}

/**
 * @brief Helper to allocate and populate logical expression
 *
 * @param left
 * @param right
 * @return
 */
bosl_ast_expression_t* bosl_ast_expression_allocate_logical(
  bosl_ast_expression_t* left,
  bosl_token_t* operator,
  bosl_ast_expression_t* right
) {
  bosl_ast_expression_t* e = bosl_ast_expression_allocate( EXPRESSION_LOGICAL );
  if ( ! e ) {
    // FIXME: DESTROY e and right
    return NULL;
  }
  // get inner data
  bosl_ast_expression_logical_t* logical = e->data;
  // populate data
  logical->left = left;
  logical->operator = operator;
  logical->right = right;
  // return expression
  return e;
}

/**
 * @brief Helper to allocate and populate literal expression
 *
 * @param data
 * @param size
 * @return
 */
bosl_ast_expression_t* bosl_ast_expression_allocate_literal(
  const void* data,
  size_t size
) {
  // allocate expression
  bosl_ast_expression_t* e = bosl_ast_expression_allocate( EXPRESSION_LITERAL );
  if ( ! e ) {
    return NULL;
  }
  // get inner type
  bosl_ast_expression_literal_t* literal = e->data;
  // allocate space for literal
  literal->value = malloc( size );
  if ( ! literal->value ) {
    // FIXME: DESTROY e
    return NULL;
  }
  // copy over data
  memcpy( literal->value, data, size );
  // set size
  literal->size = size;
  // return succes
  return e;
}
