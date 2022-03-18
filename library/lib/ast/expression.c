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
void* ast_expression_allocate( bosl_ast_expression_type_t type ) {
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
  if ( ! inner_block ) {
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
  // return built expression
  return expression;
}

/**
 * @brief Push literal to expression
 *
 * @param expression
 * @param data
 * @param size
 * @return
 */
bool ast_expression_push_literal(
  bosl_ast_expression_t* expression,
  const void* data,
  size_t size
) {
  // handle invalid type
  if ( EXPRESSION_LITERAL != expression->type ) {
    return false;
  }
  // get inner type
  bosl_ast_expression_literal_t* literal = expression->data;
  // allocate space for literal
  literal->value = malloc( size );
  if ( ! literal->value ) {
    return false;
  }
  // copy over data
  memcpy( literal->value, data, size );
  // return succes
  return true;
}
