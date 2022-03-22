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
#include "statement.h"

/**
 * @brief Method to allocate ast statement
 *
 * @param type
 * @return
 */
bosl_ast_statement_t* bosl_ast_statement_allocate( bosl_ast_statement_type_t type ) {
  size_t allocated_size = 0;
  void* inner_block = NULL;
  // allocate container
  bosl_ast_statement_t* statement = malloc( sizeof( bosl_ast_statement_t ) );
  if ( ! statement ) {
    return NULL;
  }
  // clear out
  memset( statement, 0, sizeof( bosl_ast_statement_t ) );
  // determine space for inner
  switch ( type ) {
    case STATEMENT_BLOCK:
      allocated_size = sizeof( bosl_ast_statement_block_t );
      break;
    case STATEMENT_EXPRESSION:
      allocated_size = sizeof( bosl_ast_statement_expression_t );
      break;
    case STATEMENT_PARAMETER:
      allocated_size = sizeof( bosl_ast_statement_parameter_t );
      break;
    case STATEMENT_FUNCTION:
      allocated_size = sizeof( bosl_ast_statement_function_t );
      break;
    case STATEMENT_IF:
      allocated_size = sizeof( bosl_ast_statement_if_t );
      break;
    case STATEMENT_PRINT:
      allocated_size = sizeof( bosl_ast_statement_print_t );
      break;
    case STATEMENT_RETURN:
      allocated_size = sizeof( bosl_ast_statement_return_t );
      break;
    case STATEMENT_VARIABLE:
      allocated_size = sizeof( bosl_ast_statement_variable_t );
      break;
    case STATEMENT_CONST:
      allocated_size = sizeof( bosl_ast_statement_const_t );
      break;
    case STATEMENT_WHILE:
      allocated_size = sizeof( bosl_ast_statement_while_t );
      break;
    case STATEMENT_POINTER:
      allocated_size = sizeof( bosl_ast_statement_pointer_t );
      break;
  }
  // handle error
  if ( ! allocated_size ) {
    return NULL;
  }
  // allocate inner structure
  inner_block = malloc( allocated_size );
  if ( ! inner_block ) {
    free( statement );
    return NULL;
  }
  memset( inner_block, 0, allocated_size );
  // set statement content finally
  statement->type = type;
  statement->data = inner_block;
  statement->size = allocated_size;
  // return built statement
  return statement;
}

/**
 * @brief Helper to destroy a statement
 *
 * @param statement
 */
void bosl_ast_statement_destroy( bosl_ast_statement_t* statement ) {
  if ( ! statement ) {
    return;
  }
  if ( statement->data ) {
    switch ( statement->type ) {
      case STATEMENT_BLOCK:
        list_destruct( statement->block->statements );
        break;
      case STATEMENT_EXPRESSION:
        bosl_ast_expression_destroy( statement->expression->expression );
        break;
      case STATEMENT_PARAMETER:
        // do nothing as only token references are in here
        break;
      case STATEMENT_FUNCTION:
        list_destruct( statement->function->parameter );
        bosl_ast_statement_destroy( statement->function->body );
        break;
      case STATEMENT_IF:
        bosl_ast_expression_destroy( statement->if_else->if_condition );
        bosl_ast_statement_destroy( statement->if_else->if_statement );
        bosl_ast_statement_destroy( statement->if_else->else_statement );
        break;
      case STATEMENT_PRINT:
        bosl_ast_expression_destroy( statement->print->expression );
        break;
      case STATEMENT_RETURN:
        bosl_ast_expression_destroy( statement->return_value->value );
        break;
      case STATEMENT_VARIABLE:
        bosl_ast_expression_destroy( statement->variable->initializer );
        break;
      case STATEMENT_CONST:
        bosl_ast_expression_destroy( statement->constant->initializer );
        break;
      case STATEMENT_WHILE:
        bosl_ast_expression_destroy( statement->while_loop->condition );
        bosl_ast_statement_destroy( statement->while_loop->body );
        break;
      case STATEMENT_POINTER:
        bosl_ast_statement_destroy( statement->pointer->statement );
        break;
    }
    // finally free data
    free( statement->data );
  }
  // free statement
  free( statement );
}
