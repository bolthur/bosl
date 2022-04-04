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
#include <stdio.h>
#include <inttypes.h>
#include "parser.h"
#include "scanner.h"
#include "error.h"
#include "ast/expression.h"
#include "ast/statement.h"
#include "ast/common.h"

// necessary forward declaration
static bosl_ast_expression_t* expression( void );
static bosl_ast_node_t* declaration( void );
static bosl_ast_node_t* statement( void );

static bosl_parser_t* parser = NULL;

/**
 * @brief Cleanup helper for list of expressions
 *
 * @param item
 */
static void list_expression_cleanup( list_item_t* item ) {
  // destroy expression
  bosl_ast_expression_destroy( item->data );
  // call default cleanup
  list_default_cleanup( item );
}

/**
 * @brief Cleanup helper for list of statements
 *
 * @param item
 */
static void list_statement_cleanup( list_item_t* item ) {
  // destroy statement
  bosl_ast_statement_destroy( item->data );
  // call default cleanup
  list_default_cleanup( item );
}

/**
 * @brief Cleanup helper for list of nodes
 *
 * @param item
 */
static void list_node_cleanup( list_item_t* item ) {
  // destroy node
  bosl_ast_node_destroy( item->data );
  // call default cleanup
  list_default_cleanup( item );
}

/**
 * @brief Previous token helper
 *
 * @return
 */
static bosl_token_t* previous( void ) {
  return ( bosl_token_t* )( parser->_current->previous->data );
}

/**
 * @brief current token helper
 *
 * @return
 */
static bosl_token_t* current( void ) {
  return ( bosl_token_t* )( parser->_current->data );
}

/**
 * @brief Next token helper
 *
 * @return
 */
static bosl_token_t* next( void ) {
  if ( TOKEN_EOF != parser->current()->type ) {
    parser->_current = parser->_current->next;
  }
  return parser->previous();
}

/**
 * @brief Helper to advance to next token on match with passed type
 *
 * @param type
 * @return
 */
static bool match( bosl_token_type_t type ) {
  // return false if not matching
  if ( type != parser->current()->type ) {
    return false;
  }
  // push to next
  parser->next();
  // return success
  return true;
}

/**
 * @brief Head over to next on match, else raise an error
 *
 * @param type
 * @param error_message
 * @return
 */
static bosl_token_t* consume( bosl_token_type_t type, const char* error_message ) {
  // check for mismatch
  if ( parser->current()->type != type ) {
    // raise error and return false
    bosl_error_raise( parser->current(), error_message );
    // return null
    return NULL;
  }
  // return next
  return parser->next();
}

/**
 * @brief Handle primary expression
 *
 * @return
 *
 * @todo raise errors where NULL is returned if necessary
 */
static bosl_ast_expression_t* expression_primary( void ) {
  if ( match( TOKEN_FALSE ) ) {
    bool b = false;
    return bosl_ast_expression_allocate_literal(
      &b, sizeof( false ), EXPRESSION_LITERAL_TYPE_BOOL );
  }
  if ( match( TOKEN_TRUE ) ) {
    bool b = true;
    return bosl_ast_expression_allocate_literal(
      &b, sizeof( true ), EXPRESSION_LITERAL_TYPE_BOOL );
  }
  if ( match( TOKEN_NULL ) ) {
    return bosl_ast_expression_allocate_literal(
      NULL, 0, EXPRESSION_LITERAL_TYPE_NULL );
  }

  if ( match( TOKEN_STRING ) ) {
    bosl_token_t* token = parser->previous();
    return bosl_ast_expression_allocate_literal(
      token->start,
      sizeof( char ) * ( token->length ),
      EXPRESSION_LITERAL_TYPE_STRING
    );
  }

  if ( match( TOKEN_NUMBER ) ) {
    // get token
    bosl_token_t* token = parser->previous();
    const char* s = token->start;
    const char* se = token->start + token->length;
    // detect float / hex
    bool is_float = false;
    bool is_hex = false;
    while ( s < se ) {
      if ( '.' == *s ) {
        is_float = true;
      }
      if ( 'x' == *s ) {
        is_hex = true;
      }
      s++;
    }
    // float and hex is not possible
    if ( is_float && is_hex ) {
      return NULL;
    }

    char* end;
    // push float literal
    if ( is_float ) {
      // translate to number
      long double num = strtold( token->start, &end );
      if ( end != token->start + token->length ) {
        return NULL;
      }
      // push to literal
      return bosl_ast_expression_allocate_literal(
        &num, sizeof( num ), EXPRESSION_LITERAL_TYPE_NUMBER_FLOAT );
    // push number literal
    } else {
      // translate to number
      uint64_t num = strtoull( token->start, &end, 0 );
      if ( end != token->start + token->length ) {
        return NULL;
      }
      // push to literal
      return bosl_ast_expression_allocate_literal(
        &num, sizeof( num ), is_hex
          ? EXPRESSION_LITERAL_TYPE_NUMBER_HEX
          : EXPRESSION_LITERAL_TYPE_NUMBER_INT );
    }
  }

  if ( match( TOKEN_IDENTIFIER ) ) {
    // create variable expression
    bosl_ast_expression_t* new_e = bosl_ast_expression_allocate(
      EXPRESSION_VARIABLE );
    if ( ! new_e ) {
      return NULL;
    }
    // get pointer to data
    new_e->variable->name = parser->previous();
    // return built expression
    return new_e;
  }

  if ( match( TOKEN_LEFT_PARENTHESIS ) ) {
    // translate expression
    bosl_ast_expression_t* e = expression();
    if ( ! e ) {
      return NULL;
    }
    // expect closing parenthesis
    if ( ! consume( TOKEN_RIGHT_PARENTHESIS, "Expect ')' after expression." ) ) {
      bosl_ast_expression_destroy( e );
      return NULL;
    }
    // create group expression
    bosl_ast_expression_t* new_e = bosl_ast_expression_allocate( EXPRESSION_GROUPING );
    if ( ! new_e ) {
      bosl_ast_expression_destroy( e );
      return NULL;
    }
    // get pointer to data
    new_e->grouping->expression = e;
    // return built expression
    return new_e;
  }
  // raise error
  bosl_error_raise( parser->current(), "Expected expression." );
  return NULL;
}

/**
 * @brief Finish call expression
 *
 * @param e
 * @return
 */
static bosl_ast_expression_t* expression_call_finish(
  bosl_ast_expression_t* callee
) {
  bosl_token_t* current = parser->current();
  // create arguments list
  list_manager_t* arguments = list_construct(
    NULL, list_expression_cleanup, NULL );
  if ( ! arguments ) {
    return NULL;
  }
  // handle possible arguments
  if ( TOKEN_RIGHT_PARENTHESIS != current->type ) {
    do {
      // get argument expression
      bosl_ast_expression_t* arg = expression();
      if ( ! arg ) {
        list_destruct( arguments );
        return NULL;
      }
      // push back
      if ( ! list_push_back_data( arguments, arg ) ) {
        bosl_ast_expression_destroy( arg );
        list_destruct( arguments );
        return NULL;
      }
    } while ( match( TOKEN_COMMA ) );
  }
  bosl_token_t* previous = consume(
    TOKEN_RIGHT_PARENTHESIS, "Expected ')' after arguments." );
  // check for closing parenthesis
  if ( ! previous ) {
    list_destruct( arguments );
    return NULL;
  }
  // allocate call expression
  bosl_ast_expression_t* e = bosl_ast_expression_allocate( EXPRESSION_CALL );
  if ( ! e ) {
    list_destruct( arguments );
    return NULL;
  }
  e->call->callee = callee;
  e->call->paren = previous;
  e->call->arguments = arguments;
  // return call expression
  return e;
}

/**
 * @brief Handle call expression
 *
 * @return
 */
static bosl_ast_expression_t* expression_call( void ) {
  bosl_ast_expression_t* e = expression_primary();
  if ( ! e ) {
    return NULL;
  }
  // handle possible function call
  while ( true ) {
    if ( match( TOKEN_LEFT_PARENTHESIS ) ) {
      bosl_ast_expression_t* new_e = expression_call_finish( e );
      if ( ! new_e ) {
        bosl_ast_expression_destroy( e );
        return NULL;
      }
      // overwrite e
      e = new_e;
    } else {
      break;
    }
  }
  // return expression
  return e;
}

/**
 * @brief Handle load expression
 *
 * @return
 */
static bosl_ast_expression_t* expression_load( void ) {
  if ( match( TOKEN_IDENTIFIER ) ) {
    // create group expression
    bosl_ast_expression_t* new_e = bosl_ast_expression_allocate( EXPRESSION_LOAD );
    if ( ! new_e ) {
      return NULL;
    }
    // populate data
    new_e->load->name = parser->previous();
    // return built expression
    return new_e;
  }
  bosl_error_raise( parser->current(), "Expect identifier after load." );
  return NULL;
}

/**
 * @brief Handle load expression
 *
 * @return
 */
static bosl_ast_expression_t* expression_pointer( void ) {
  if ( match( TOKEN_IDENTIFIER ) ) {
    // create pointer expression
    bosl_ast_expression_t* new_e = bosl_ast_expression_allocate(
      EXPRESSION_POINTER );
    if ( ! new_e ) {
      return NULL;
    }
    // populate data
    new_e->load->name = parser->previous();
    // return built expression
    return new_e;
  }
  bosl_error_raise( parser->current(), "Expect identifier after load." );
  return NULL;
}

/**
 * @brief Handle unary expression
 *
 * @return
 */
static bosl_ast_expression_t* expression_unary( void ) {
  // handle unary
  if (
    match( TOKEN_BANG )
    || match( TOKEN_MINUS )
    || match( TOKEN_PLUS )
    || match( TOKEN_BINARY_ONE_COMPLEMENT )
  ) {
    bosl_token_t* operator = parser->previous();
    bosl_ast_expression_t* right = expression_unary();
    if ( ! right ) {
      return NULL;
    }
    bosl_ast_expression_t* new_e = bosl_ast_expression_allocate( EXPRESSION_UNARY );
    if ( ! new_e ) {
      bosl_ast_expression_destroy( right );
      return NULL;
    }
    // populate
    new_e->unary->operator = operator;
    new_e->unary->right = right;
    // return unary expression
    return new_e;
  }
  // handle load
  if ( match( TOKEN_LOAD ) ) {
    return expression_load();
  }
  // handle pointer
  if ( match ( TOKEN_POINTER ) ) {
    return expression_pointer();
  }
  // continue with call expression
  return expression_call();
}

/**
 * @brief Handle factor expression
 *
 * @return
 */
static bosl_ast_expression_t* expression_factor( void ) {
  // handle possible unary
  bosl_ast_expression_t* e = expression_unary();
  if ( ! e ) {
    return NULL;
  }
  while (
    match( TOKEN_SLASH )
    || match( TOKEN_STAR )
    || match( TOKEN_MODULO )
  ) {
    bosl_token_t* operator = parser->previous();
    bosl_ast_expression_t* right = expression_unary();
    if ( ! right ) {
      bosl_ast_expression_destroy( e );
      return NULL;
    }
    bosl_ast_expression_t* new_e = bosl_ast_expression_allocate_binary(
      e, operator, right );
    if ( ! new_e ) {
      bosl_ast_expression_destroy( e );
      bosl_ast_expression_destroy( right );
      return NULL;
    }
    // overwrite e
    e = new_e;
  }
  // return expression
  return e;
}

/**
 * @brief Handle term expression
 *
 * @return
 */
static bosl_ast_expression_t* expression_term( void ) {
  // handle possible factor
  bosl_ast_expression_t* e = expression_factor();
  if ( ! e ) {
    return NULL;
  }
  while ( match( TOKEN_MINUS ) || match( TOKEN_PLUS ) ) {
    bosl_token_t* operator = parser->previous();
    bosl_ast_expression_t* right = expression_factor();
    if ( ! right ) {
      bosl_ast_expression_destroy( e );
      return NULL;
    }
    bosl_ast_expression_t* new_e = bosl_ast_expression_allocate_binary(
      e, operator, right );
    if ( ! new_e ) {
      bosl_ast_expression_destroy( e );
      bosl_ast_expression_destroy( right );
      return NULL;
    }
    // overwrite e
    e = new_e;
  }
  // return expression
  return e;
}

/**
 * @brief Handle comparison expression
 *
 * @return
 */
static bosl_ast_expression_t* expression_comparison( void ) {
  // handle possible term
  bosl_ast_expression_t* e = expression_term();
  if ( ! e ) {
    return NULL;
  }
  while (
    match( TOKEN_GREATER )
    || match( TOKEN_GREATER_EQUAL )
    || match( TOKEN_LESS )
    || match( TOKEN_LESS_EQUAL )
    || match( TOKEN_SHIFT_LEFT )
    || match( TOKEN_SHIFT_RIGHT )
  ) {
    bosl_token_t* operator = parser->previous();
    bosl_ast_expression_t* right = expression_term();
    if ( ! right ) {
      bosl_ast_expression_destroy( e );
      return NULL;
    }
    bosl_ast_expression_t* new_e = bosl_ast_expression_allocate_binary(
      e, operator, right );
    if ( ! new_e ) {
      bosl_ast_expression_destroy( e );
      bosl_ast_expression_destroy( right );
      return NULL;
    }
    // overwrite e
    e = new_e;
  }
  // return expression
  return e;
}

/**
 * @brief Handle equality expression
 *
 * @return
 */
static bosl_ast_expression_t* expression_equality( void ) {
  // handle possible comparison
  bosl_ast_expression_t* e = expression_comparison();
  if ( ! e ) {
    return NULL;
  }
  while( match( TOKEN_BANG_EQUAL ) || match( TOKEN_EQUAL_EQUAL ) ) {
    bosl_token_t* operator = parser->previous();
    bosl_ast_expression_t* right = expression_comparison();
    if ( ! right ) {
      bosl_ast_expression_destroy( e );
      return NULL;
    }
    bosl_ast_expression_t* new_e = bosl_ast_expression_allocate_binary(
      e, operator, right );
    if ( ! new_e ) {
      bosl_ast_expression_destroy( e );
      bosl_ast_expression_destroy( right );
      return NULL;
    }
    // overwrite e
    e = new_e;
  }
  // return expression
  return e;
}

/**
 * @brief Handle and expression
 *
 * @return
 */
static bosl_ast_expression_t* expression_and( void ) {
  // handle possible equality
  bosl_ast_expression_t* e = expression_equality();
  if ( ! e ) {
    return NULL;
  }
  // loop through all possible anded conditions
  while ( match( TOKEN_AND ) ) {
    return NULL;
    // get operator
    bosl_token_t* operator = parser->previous();
    // evaluate right expression
    bosl_ast_expression_t* right = expression_equality();
    if ( ! right ) {
      bosl_ast_expression_destroy( e );
      return NULL;
    }
    bosl_ast_expression_t* new_e = bosl_ast_expression_allocate_logical(
      e, operator, right );
    if ( ! new_e ) {
      bosl_ast_expression_destroy( e );
      bosl_ast_expression_destroy( right );
      return NULL;
    }
    // overwrite e
    e = new_e;
  }
  return e;
}

/**
 * @brief Handle xor expression
 *
 * @return
 */
static bosl_ast_expression_t* expression_xor( void ) {
  // handle possible equality
  bosl_ast_expression_t* e = expression_and();
  if ( ! e ) {
    return NULL;
  }
  // loop through all possible anded conditions
  while ( match( TOKEN_XOR ) ) {
    return NULL;
    // get operator
    bosl_token_t* operator = parser->previous();
    // evaluate right expression
    bosl_ast_expression_t* right = expression_and();
    if ( ! right ) {
      bosl_ast_expression_destroy( e );
      return NULL;
    }
    bosl_ast_expression_t* new_e = bosl_ast_expression_allocate_logical(
      e, operator, right );
    if ( ! new_e ) {
      bosl_ast_expression_destroy( e );
      bosl_ast_expression_destroy( right );
      return NULL;
    }
    // overwrite e
    e = new_e;
  }
  return e;
}

/**
 * @brief Handle or expression
 *
 * @return
 */
static bosl_ast_expression_t* expression_or( void ) {
  // handle possible equality
  bosl_ast_expression_t* e = expression_xor();
  if ( ! e ) {
    return NULL;
  }
  // loop through all possible anded conditions
  while ( match( TOKEN_OR ) ) {
    return NULL;
    // get operator
    bosl_token_t* operator = parser->previous();
    // evaluate right expression
    bosl_ast_expression_t* right = expression_xor();
    if ( ! right ) {
      bosl_ast_expression_destroy( e );
      return NULL;
    }
    bosl_ast_expression_t* new_e = bosl_ast_expression_allocate_logical(
      e, operator, right );
    if ( ! new_e ) {
      bosl_ast_expression_destroy( e );
      bosl_ast_expression_destroy( right );
      return NULL;
    }
    // overwrite e
    e = new_e;
  }
  return e;
}

/**
 * @brief Handle logical and expression
 *
 * @return
 */
static bosl_ast_expression_t* expression_logic_and( void ) {
  // handle possible equality
  bosl_ast_expression_t* e = expression_or();
  if ( ! e ) {
    return NULL;
  }
  // loop through all possible anded conditions
  while ( match( TOKEN_AND_AND ) ) {
    // get operator
    bosl_token_t* operator = parser->previous();
    // evaluate right expression
    bosl_ast_expression_t* right = expression_or();
    if ( ! right ) {
      bosl_ast_expression_destroy( e );
      return NULL;
    }
    bosl_ast_expression_t* new_e = bosl_ast_expression_allocate_logical(
      e, operator, right );
    if ( ! new_e ) {
      bosl_ast_expression_destroy( e );
      bosl_ast_expression_destroy( right );
      return NULL;
    }
    // overwrite e
    e = new_e;
  }
  return e;
}

/**
 * @brief Handle logical or expression
 *
 * @return
 */
static bosl_ast_expression_t* expression_logic_or( void ) {
  // handle possible and
  bosl_ast_expression_t* e = expression_logic_and();
  if ( ! e ) {
    return NULL;
  }
  // loop through all possible orred conditions
  while ( match( TOKEN_OR_OR ) ) {
    // get operator
    bosl_token_t* operator = parser->previous();
    // evaluate right expression
    bosl_ast_expression_t* right = expression_logic_and();
    if ( ! right ) {
      bosl_ast_expression_destroy( e );
      return NULL;
    }
    bosl_ast_expression_t* new_e = bosl_ast_expression_allocate_logical(
      e, operator, right );
    if ( ! new_e ) {
      bosl_ast_expression_destroy( e );
      bosl_ast_expression_destroy( right );
      return NULL;
    }
    // overwrite e
    e = new_e;
  }
  // return e
  return e;
}

/**
 * @brief Handle assignment expression
 *
 * @return
 */
static bosl_ast_expression_t* expression_assignment( void ) {
  // handle possible or
  bosl_ast_expression_t* e = expression_logic_or();
  if ( ! e ) {
    return NULL;
  }
  // handle assignment
  if ( match( TOKEN_EQUAL ) ) {
    // get previous token
    bosl_token_t* previous = parser->previous();
    // get value for assignment
    bosl_ast_expression_t* value = expression_assignment();
    if ( ! value ) {
      bosl_ast_expression_destroy( e );
      return NULL;
    }

    // handle variable
    if ( EXPRESSION_VARIABLE == e->type ) {
      // build and return assign expression
      bosl_ast_expression_t* new_e = bosl_ast_expression_allocate( EXPRESSION_ASSIGN );
      if ( ! new_e ) {
        bosl_ast_expression_destroy( e );
        bosl_ast_expression_destroy( value );
        return NULL;
      }
      // populate inner data
      new_e->assign->token = e->variable->name;
      new_e->assign->value = value;
      // return assign expression
      bosl_ast_expression_destroy( e );
      return new_e;
    }
    // raise error
    bosl_error_raise( previous, "Invalid assignment target." );
    bosl_ast_expression_destroy( e );
    return NULL;
  }
  // return expression
  return e;
}

/**
 * @brief Handle expression
 *
 * @return
 */
static bosl_ast_expression_t* expression( void ) {
  return expression_assignment();
}

/**
 * @brief Handle if statement
 *
 * @return
 */
static bosl_ast_node_t* statement_if( void ) {
  // allocate new node
  bosl_ast_node_t* node = bosl_ast_node_allocate();
  if ( ! node ) {
    return NULL;
  }
  // allocate new ast node
  node->statement = bosl_ast_statement_allocate( STATEMENT_IF );
  if ( ! node->statement ) {
    bosl_ast_node_destroy( node );
    return NULL;
  }
  // expect opening parenthesis
  if ( ! consume( TOKEN_LEFT_PARENTHESIS, "Expect '(' after 'if'." ) ) {
    bosl_ast_node_destroy( node );
    return NULL;
  }
  // evaluate expression
  bosl_ast_expression_t* if_expression = expression();
  if ( ! if_expression ) {
    bosl_ast_node_destroy( node );
    return NULL;
  }
  // expect parenthesis
  if ( ! consume( TOKEN_RIGHT_PARENTHESIS, "Expect ')' after 'if'." ) ) {
    bosl_ast_node_destroy( node );
    bosl_ast_expression_destroy( if_expression );
    return NULL;
  }
  // consume statement
  bosl_ast_node_t* if_statement = statement();
  if ( ! if_statement ) {
    bosl_ast_node_destroy( node );
    bosl_ast_expression_destroy( if_expression );
    return NULL;
  }
  // populate node
  node->statement->if_else->if_condition = if_expression;
  node->statement->if_else->if_statement = if_statement->statement;
  free( if_statement );
  // get possible else branch
  bosl_ast_node_t* else_branch = NULL;
  if ( match( TOKEN_ELSE ) ) {
    // get else branch
    else_branch = statement();
    if ( ! else_branch ) {
      bosl_ast_node_destroy( node );
      return NULL;
    }
    // add to node
    node->statement->if_else->else_statement = else_branch->statement;
    // free container
    free( else_branch );
  }
  // return node
  return node;
}

/**
 * @brief Handle print statement
 *
 * @return
 */
static bosl_ast_node_t* statement_print( void ) {
  // expect opening parenthesis
  if ( ! consume( TOKEN_LEFT_PARENTHESIS, "Expect '(' after print." ) ) {
    return NULL;
  }
  // get expression
  bosl_ast_expression_t* e = expression();
  if ( ! e ) {
    return NULL;
  }
  // expect closing parenthesis
  if ( ! consume( TOKEN_RIGHT_PARENTHESIS, "Expect ')' after expression." ) ) {
    bosl_ast_expression_destroy( e );
    return NULL;
  }
  // expect semicolon
  if ( ! consume( TOKEN_SEMICOLON, "Expect ';' at end of print." ) ) {
    bosl_ast_expression_destroy( e );
    return NULL;
  }
  // allocate new node
  bosl_ast_node_t* node = bosl_ast_node_allocate();
  if ( ! node ) {
    bosl_ast_expression_destroy( e );
    return NULL;
  }
  // allocate new ast node
  node->statement = bosl_ast_statement_allocate( STATEMENT_PRINT );
  if ( ! node->statement ) {
    bosl_ast_expression_destroy( e );
    bosl_ast_node_destroy( node );
    return NULL;
  }
  // populate
  node->statement->print->expression = e;
  // return
  return node;
}

/**
 * @brief Handle return statement
 *
 * @return
 */
static bosl_ast_node_t* statement_return( void ) {
  // handle return not in function
  if ( ! parser->_in_function ) {
    bosl_error_raise( parser->current(), "Return is only in functions allowed" );
    return NULL;
  }
  // get keyword
  bosl_token_t* keyword = parser->previous();
  // default no return
  bosl_ast_expression_t* value = NULL;
  // evaluate expression
  if ( TOKEN_SEMICOLON != parser->current()->type ) {
    value = expression();
  }
  // expect semicolon
  if ( ! consume( TOKEN_SEMICOLON, "Expect ';' after return value." ) ) {
    if ( value ) {
      bosl_ast_expression_destroy( value );
    }
    return NULL;
  }
  // allocate new node
  bosl_ast_node_t* node = bosl_ast_node_allocate();
  if ( ! node ) {
    if ( value ) {
      bosl_ast_expression_destroy( value );
    }
    return NULL;
  }
  // allocate new ast node
  node->statement = bosl_ast_statement_allocate( STATEMENT_RETURN );
  if ( ! node->statement ) {
    if ( value ) {
      bosl_ast_expression_destroy( value );
    }
    bosl_ast_node_destroy( node );
    return NULL;
  }
  // populate
  node->statement->return_value->keyword = keyword;
  node->statement->return_value->value = value;
  // return
  return node;
}

/**
 * @brief Handle while statement
 *
 * @return
 */
static bosl_ast_node_t* statement_while( void ) {
  if ( ! consume( TOKEN_LEFT_PARENTHESIS, "Expect '(' after 'while'." ) ) {
    return NULL;
  }
  // evaluate expression
  bosl_ast_expression_t* e = expression();
  if ( ! e ) {
    return NULL;
  }
  if ( ! consume( TOKEN_RIGHT_PARENTHESIS, "Expect ')' after condition." ) ) {
    bosl_ast_expression_destroy( e );
    return NULL;
  }
  // get body
  bosl_ast_node_t* body = statement();
  if ( ! body ) {
    bosl_ast_expression_destroy( e );
    return NULL;
  }
  // allocate new node
  bosl_ast_node_t* node = bosl_ast_node_allocate();
  if ( ! node ) {
    bosl_ast_expression_destroy( e );
    bosl_ast_node_destroy( body );
    return NULL;
  }
  // allocate new ast node
  node->statement = bosl_ast_statement_allocate( STATEMENT_WHILE );
  if ( ! node->statement ) {
    bosl_ast_expression_destroy( e );
    bosl_ast_node_destroy( body );
    bosl_ast_node_destroy( node );
    return NULL;
  }
  // populate
  node->statement->while_loop->condition = e;
  node->statement->while_loop->body = body->statement;
  // free container for body statment
  free( body );
  // return
  return node;
}

/**
 * @brief Handle block statement
 *
 * @return
 */
static bosl_ast_node_t* statement_block( void ) {
  // allocate new ast node
  bosl_ast_node_t* node = bosl_ast_node_allocate();
  if ( ! node ) {
    return NULL;
  }
  // allocate new ast node
  node->statement = bosl_ast_statement_allocate( STATEMENT_BLOCK );
  if ( ! node->statement ) {
    bosl_ast_node_destroy( node );
    return NULL;
  }
  // construct list
  node->statement->block->statements = list_construct(
    NULL, list_statement_cleanup, NULL );
  if ( ! node->statement->block->statements ) {
    bosl_ast_node_destroy( node );
    return NULL;
  }
  while (
    TOKEN_RIGHT_BRACE != parser->current()->type
    && TOKEN_EOF != parser->current()->type
  ) {
    // evaluate
    bosl_ast_node_t* inner = declaration();
    if ( ! inner ) {
      bosl_ast_node_destroy( node );
      return NULL;
    }
    if ( ! list_push_back_data(
      node->statement->block->statements,
      inner->statement
    ) ) {
      bosl_ast_node_destroy( node );
      bosl_ast_node_destroy( inner );
      return NULL;
    }
    // free container around statement
    free( inner );
  }
  // expect closing brace
  if ( ! consume( TOKEN_RIGHT_BRACE, "Expect '}' after block." ) ) {
    bosl_ast_node_destroy( node );
    return NULL;
  }
  // return node
  return node;
}

/**
 * @brief Handle pointer statement
 *
 * @return
 */
static bosl_ast_node_t* statement_pointer( void ) {
  // get identifier
  bosl_token_t* identifier = consume(
    TOKEN_IDENTIFIER, "Expect identifier after pointer." );
  if ( ! identifier ) {
    return NULL;
  }
  // get statement
  bosl_ast_node_t* pointer_ast = statement();
  if ( ! pointer_ast ) {
    return NULL;
  }
  // allocate new ast node
  bosl_ast_node_t* node = bosl_ast_node_allocate();
  if ( ! node ) {
    return NULL;
  }
  // allocate new ast node
  node->statement = bosl_ast_statement_allocate( STATEMENT_POINTER );
  if ( ! node->statement ) {
    bosl_ast_node_destroy( node );
    return NULL;
  }
  // populate node
  node->statement->pointer->name = identifier;
  node->statement->pointer->statement = pointer_ast->statement;
  free( pointer_ast );
  // return node
  return node;
}

/**
 * @brief Handle expression statement
 *
 * @return
 */
static bosl_ast_node_t* statement_expression( void ) {
  // expression
  bosl_ast_expression_t* e = expression();
  // check for trailing semicolon
  if ( ! consume( TOKEN_SEMICOLON, "Expect ';' after expression." ) ) {
    bosl_ast_expression_destroy( e );
    return NULL;
  }
  // allocate new ast node
  bosl_ast_node_t* new_node = bosl_ast_node_allocate();
  if ( ! new_node ) {
    bosl_ast_expression_destroy( e );
    return NULL;
  }
  // allocate new ast node
  new_node->statement = bosl_ast_statement_allocate( STATEMENT_EXPRESSION );
  if ( ! new_node->statement ) {
    bosl_ast_expression_destroy( e );
    bosl_ast_node_destroy( new_node );
    return NULL;
  }
  // get pointer to data
  bosl_ast_statement_expression_t* ex = new_node->statement->data;
  // set pointer to expression
  ex->expression = e;
  // return ast node
  return new_node;
}

/**
 * @brief Handle statement
 *
 * @return
 */
static bosl_ast_node_t* statement( void ) {
  if ( match( TOKEN_IF ) ) {
    return statement_if();
  }
  if ( match( TOKEN_PRINT ) ) {
    return statement_print();
  }
  if ( match( TOKEN_RETURN ) ) {
    return statement_return();
  }
  if ( match( TOKEN_WHILE ) ) {
    return statement_while();
  }
  if ( match( TOKEN_LEFT_BRACE ) ) {
    return statement_block();
  }
  if ( match( TOKEN_POINTER ) ) {
    return statement_pointer();
  }
  return statement_expression();
}

/**
 * @brief Handle constant declaration
 *
 * @return
 */
static bosl_ast_node_t* declaration_const( void ) {
  bosl_token_t* name = consume( TOKEN_IDENTIFIER, "Expect variable name." );
  if ( ! name ) {
    return NULL;
  }
  if ( ! consume( TOKEN_COLON, "Expect colon after variable name." ) ) {
    return NULL;
  }
  // get type identifier
  bosl_token_t* type = consume(
    TOKEN_TYPE_IDENTIFIER, "Expect type identifier after colon." );
  if ( ! type ) {
    return NULL;
  }
  // handle missing initializer
  if ( ! match( TOKEN_EQUAL ) ) {
    bosl_error_raise( name, "Constants need an initializer." );
    return NULL;
  }
  // get initializer
  bosl_ast_expression_t* initializer = expression();
  if ( ! initializer ) {
    return NULL;
  }
  // assert semicolon at the end
  if ( ! consume( TOKEN_SEMICOLON, "Expect ';' after variable declaration." ) ) {
    bosl_ast_expression_destroy( initializer );
    return NULL;
  }
  // allocate node
  bosl_ast_node_t* node = bosl_ast_node_allocate();
  if ( ! node ) {
    bosl_ast_expression_destroy( initializer );
    return NULL;
  }
  // allocate statement
  node->statement = bosl_ast_statement_allocate( STATEMENT_CONST );
  if ( ! node->statement ) {
    bosl_ast_expression_destroy( initializer );
    bosl_ast_node_destroy( node );
    return NULL;
  }
  // populate
  node->statement->constant->name = name;
  node->statement->constant->initializer = initializer;
  node->statement->constant->type = type;
  // return built node
  return node;
}

/**
 * @brief Handle variable declaration
 *
 * @return
 */
static bosl_ast_node_t* declaration_let( void ) {
  // cache name
  bosl_token_t* name = consume( TOKEN_IDENTIFIER, "Expect variable name." );
  // expect name
  if ( ! name ) {
    return NULL;
  }
  if ( ! consume( TOKEN_COLON, "Expect colon after variable name." ) ) {
    return NULL;
  }
  // get type identifier
  bosl_token_t* type = consume(
    TOKEN_TYPE_IDENTIFIER, "Expect type identifier after colon." );
  if ( ! type ) {
    return NULL;
  }
  // get possible initializer
  bosl_ast_expression_t* initializer = NULL;
  if ( match( TOKEN_EQUAL ) ) {
    initializer = expression();
  }
  // assert semicolon at the end
  if ( ! consume( TOKEN_SEMICOLON, "Expect ';' after variable declaration." ) ) {
    if ( initializer ) {
      bosl_ast_expression_destroy( initializer );
    }
    return NULL;
  }
  // allocate node
  bosl_ast_node_t* node = bosl_ast_node_allocate();
  if ( ! node ) {
    if ( initializer ) {
      bosl_ast_expression_destroy( initializer );
    }
    return NULL;
  }
  // allocate statement
  node->statement = bosl_ast_statement_allocate( STATEMENT_VARIABLE );
  if ( ! node->statement ) {
    if ( initializer ) {
      bosl_ast_expression_destroy( initializer );
    }
    bosl_ast_node_destroy( node );
    return NULL;
  }
  // populate
  node->statement->variable->name = name;
  node->statement->variable->initializer = initializer;
  node->statement->variable->type = type;
  // return built node
  return node;
}

/**
 * @brief Handle function declaration
 *
 * @return
 */
static bosl_ast_node_t* declaration_function( void ) {
  // handle function in function
  if ( parser->_in_function ) {
    bosl_error_raise( parser->current(), "Function in function is not allowed" );
    return NULL;
  }
  // set flag
  parser->_in_function = true;
  // cache function name
  bosl_token_t* name = consume( TOKEN_IDENTIFIER, "Expect function name." );
  // expect function name
  if ( ! name ) {
    return NULL;
  }
  // create parameter list
  list_manager_t* parameter = list_construct(
    NULL, list_statement_cleanup, NULL );
  if ( ! parameter ) {
    return NULL;
  }
  // expect opening parenthesis
  if ( ! consume( TOKEN_LEFT_PARENTHESIS, "Expect '(' after function name." ) ) {
    list_destruct( parameter );
    return NULL;
  }
  // get current token
  bosl_token_t* current = parser->current();
  // handle possible parameter
  if ( TOKEN_RIGHT_PARENTHESIS != current->type ) {
    do {
      // cache name
      bosl_token_t* parameter_name = consume(
        TOKEN_IDENTIFIER, "Expect parameter name." );
      // expect parameter name
      if ( ! parameter_name ) {
        list_destruct( parameter );
        return NULL;
      }
      // expect colon
      if ( ! consume( TOKEN_COLON, "Expect colon after parameter name." ) ) {
        list_destruct( parameter );
        return NULL;
      }
      // cache parameter type
      bosl_token_t* parameter_type = consume(
        TOKEN_TYPE_IDENTIFIER, "Expect type identifier after colon." );
      // expect type identifier
      if ( ! parameter_type ) {
        list_destruct( parameter );
        return NULL;
      }
      // build object to push
      bosl_ast_statement_t* p = bosl_ast_statement_allocate(
        STATEMENT_PARAMETER );
      if ( ! p ) {
        list_destruct( parameter );
        return NULL;
      }
      // populate
      p->parameter->name = parameter_name;
      p->parameter->type = parameter_type;
      // push back
      if ( ! list_push_back_data( parameter, p ) ) {
        bosl_ast_statement_destroy( p );
        list_destruct( parameter );
        return NULL;
      }
    } while ( match( TOKEN_COMMA ) );
  }
  // check for closing parenthesis
  if ( ! consume( TOKEN_RIGHT_PARENTHESIS, "Expected ')' after arguments." ) ) {
    list_destruct( parameter );
    return NULL;
  }
  // expect colon
  if ( ! consume( TOKEN_COLON, "Expect colon after closing parenthesis." ) ) {
    list_destruct( parameter );
    return NULL;
  }
  // cache return type
  bosl_token_t* return_type = consume(
    TOKEN_TYPE_IDENTIFIER, "Expect return type identifier." );
  // expect type identifier
  if ( ! return_type ) {
    list_destruct( parameter );
    return NULL;
  }
  // check opening brace
  if ( ! consume( TOKEN_LEFT_BRACE, "Expected '{' before body." ) ) {
    list_destruct( parameter );
    return NULL;
  }
  // build object to push
  bosl_ast_statement_t* f = bosl_ast_statement_allocate(
    STATEMENT_FUNCTION );
  if ( ! f ) {
    list_destruct( parameter );
    return NULL;
  }
  // create block for body
  bosl_ast_node_t* body = statement_block();
  if ( ! body ) {
    list_destruct( parameter );
    bosl_ast_statement_destroy( f );
    return NULL;
  }
  // handle possible load
  if ( match( TOKEN_EQUAL ) ) {
    // destroy body again
    bosl_ast_node_destroy( body );
    // after equal a loat has to come
    if ( ! consume( TOKEN_LOAD, "Expect load type after equal." ) ) {
      list_destruct( parameter );
      bosl_ast_statement_destroy( f );
      return NULL;
    }
    // set load identifier
    f->function->load_identifier = consume(
      TOKEN_IDENTIFIER, "Expect identifier after load." );
    if ( ! f->function->load_identifier ) {
      list_destruct( parameter );
      bosl_ast_statement_destroy( f );
      return NULL;
    }
  } else {
    // set body
    f->function->body = body->statement;
    free( body );
  }
  // populate rest of stuff
  f->function->token = name;
  f->function->parameter = parameter;
  f->function->return_type = return_type;
  // allocate node
  bosl_ast_node_t* node = bosl_ast_node_allocate();
  if ( ! node ) {
    bosl_ast_statement_destroy( f );
    return NULL;
  }
  // populate node
  node->statement = f;
  // reset flag
  parser->_in_function = true;
  // finally return it
  return node;
}

/**
 * @brief Scan possible declaration
 *
 * @return
 */
static bosl_ast_node_t* declaration( void ) {
  // handle function
  if ( match( TOKEN_FUNCTION ) ) {
    return declaration_function();
  }
  // handle variable
  if ( match( TOKEN_LET ) ) {
    return declaration_let();
  }
  // handle constant
  if ( match( TOKEN_CONST ) ) {
    return declaration_const();
  }
  // evaluate statement
  return statement();
}

/**
 * @brief Setup parser
 *
 * @param token
 * @return
 */
bool bosl_parser_init( list_manager_t* token ) {
  // handle already initialized
  if ( parser ) {
    return true;
  }
  // allocate parser structure
  parser = malloc( sizeof( *parser ) );
  if ( ! parser ) {
    return false;
  }
  // clear out
  memset( parser, 0, sizeof( *parser ) );
  // construct ast list
  parser->ast = list_construct( NULL, list_node_cleanup, NULL );
  if ( ! parser->ast ) {
    free( parser );
    return false;
  }
  // push in token list and set current to first element
  parser->_token = token;
  parser->_current = token->first;
  parser->_in_function = false;
  // push back convenience helper
  parser->current = current;
  parser->next = next;
  parser->previous = previous;
  // return success
  return true;
}

/**
 * @brief Free parser
 */
void bosl_parser_free( void ) {
  // handle not initialized
  if ( ! parser ) {
    return;
  }
  // free ast
  if ( parser->ast ) {
    list_destruct( parser->ast );
  }
  // just free structure
  free( parser );
}

/**
 * @brief Scan tokens to array of ast
 *
 * @return
 */
list_manager_t* bosl_parser_scan( void ) {
  // handle not initialized
  if ( ! parser ) {
    return NULL;
  }
  // loop until end
  while ( parser->_current && TOKEN_EOF != parser->current()->type ) {
    // handle eof by break
    bosl_token_t* token = parser->current();
    if ( TOKEN_EOF == token->type ) {
      break;
    }
    // evaluate
    bosl_ast_node_t* tmp = declaration();
    if ( ! tmp ) {
      // free parser stuff
      bosl_parser_free();
      // return null
      return NULL;
    }
    // add to list
    if ( ! list_push_back_data( parser->ast, tmp ) ) {
      bosl_error_raise(
        parser->current(),
        "Unable to push back ast node!"
      );
      // destroy node
      bosl_ast_node_destroy( tmp );
      // free parser stuff
      bosl_parser_free();
      // return null
      return NULL;
    }
  }
  // return built byte code
  return parser->ast;
}

/**
 * @brief Helper to print an expression
 *
 * @param e
 * @return
 */
static void print_expression( bosl_ast_expression_t* e ) {
  switch ( e->type ) {
    case EXPRESSION_ASSIGN: {
      // opening block
      fprintf(
        stdout, "(= %*.*s ",
        ( int )e->assign->token->length,
        ( int )e->assign->token->length,
        e->assign->token->start
      );
      // print expression
      print_expression( e->assign->value );
      // closing block
      fprintf( stdout, ")" );
      break;
    }
    case EXPRESSION_BINARY: {
      // opening block
      fprintf(
        stdout, "(%*.*s ",
        ( int )e->binary->operator->length,
        ( int )e->binary->operator->length,
        e->binary->operator->start
      );
      // print expression
      print_expression( e->binary->left );
      fprintf( stdout, " " );
      print_expression( e->binary->right );
      // closing block
      fprintf( stdout, ")" );
      break;
    }
    case EXPRESSION_CALL: {
      // opening block
      fprintf( stdout, "(call " );
      // print callee expression
      print_expression( e->call->callee );
      fprintf( stdout, " " );
      // print parameter
      for ( list_item_t* c = e->call->arguments->first; c; c = c->next ) {
        print_expression( c->data );
      }
      // closing block
      fprintf( stdout, ")" );
      break;
    }
    case EXPRESSION_LOAD: {
      fprintf(
        stdout, "(load %*.*s)",
        ( int )e->load->name->length,
        ( int )e->load->name->length,
        e->load->name->start
      );
      break;
    }
    case EXPRESSION_POINTER: {
      fprintf(
        stdout, "(p %*.*s)",
        ( int )e->load->name->length,
        ( int )e->load->name->length,
        e->load->name->start
      );
      break;
    }
    case EXPRESSION_GROUPING: {
      // opening block
      fprintf( stdout, "(group " );
      // print expression
      print_expression( e->grouping->expression );
      // closing block
      fprintf( stdout, ")" );
      break;
    }
    case EXPRESSION_LITERAL: {
      switch ( e->literal->type ) {
        case EXPRESSION_LITERAL_TYPE_NULL:
          fprintf( stdout, "null" );
          break;
        case EXPRESSION_LITERAL_TYPE_STRING:
          fprintf(
            stdout, "%*.*s",
            ( int )e->literal->size,
            ( int )e->literal->size,
            ( char* )e->literal->value
          );
          break;
        case EXPRESSION_LITERAL_TYPE_BOOL:
          fprintf(
            stdout, "%s",
            ( *( ( bool* )e->literal->value ) ? "true" : "false" )
          );
          break;
        case EXPRESSION_LITERAL_TYPE_NUMBER_FLOAT: {
          long double num;
          memcpy( &num, e->literal->value, sizeof( num ) );
          fprintf( stdout, "%Lf", num );
          break;
        }
        case EXPRESSION_LITERAL_TYPE_NUMBER_INT: {
          uint64_t num;
          memcpy( &num, e->literal->value, sizeof( uint64_t ) );
          fprintf( stdout, "%"PRIu64, num );
          break;
        }
        case EXPRESSION_LITERAL_TYPE_NUMBER_HEX: {
          uint64_t num;
          memcpy( &num, e->literal->value, sizeof( uint64_t ) );
          fprintf( stdout, "%"PRIx64, num );
          break;
        }
      }
      break;
    }
    case EXPRESSION_LOGICAL: {
      // opening block
      fprintf(
        stdout, "(%*.*s ",
        ( int )e->logical->operator->length,
        ( int )e->logical->operator->length,
        e->logical->operator->start
      );
      // print expression
      print_expression( e->logical->left );
      fprintf( stdout, " " );
      print_expression( e->logical->right );
      // closing block
      fprintf( stdout, ")" );
      break;
    }
    case EXPRESSION_UNARY: {
      // opening block
      fprintf(
        stdout, "(%*.*s ",
        ( int )e->unary->operator->length,
        ( int )e->unary->operator->length,
        e->unary->operator->start
      );
      // print expression
      print_expression( e->unary->right );
      // closing block
      fprintf( stdout, ")" );
      break;
    }
    case EXPRESSION_VARIABLE: {
      fprintf(
        stdout, "%*.*s",
        ( int )e->variable->name->length,
        ( int )e->variable->name->length,
        e->variable->name->start
      );
      break;
    }
  }
}

/**
 * @brief Helper to print a statement
 *
 * @param s
 */
static void print_statement( bosl_ast_statement_t* s ) {
  switch ( s->type ) {
    case STATEMENT_FUNCTION: {
      // opening block
      fprintf(
        stdout, "(fn %*.*s (",
        ( int )s->function->token->length,
        ( int )s->function->token->length,
        s->function->token->start
      );
      // print parameter
      for ( list_item_t* c = s->function->parameter->first; c; c = c->next ) {
        if ( c != s->function->parameter->first ) {
          fprintf( stdout, " " );
        }
        bosl_ast_statement_parameter_t* p = c->data;
        fprintf(
          stdout, "%*.*s:%*.*s",
          ( int )p->name->length, ( int )p->name->length, p->name->start,
          ( int )p->type->length, ( int )p->name->length, p->type->start
        );
      }
      // close parameter parenthesis
      fprintf( stdout, ")" );
      // print return type
      if ( s->function->return_type ) {
        fprintf(
          stdout, ": %*.s",
          ( int )s->function->return_type->length,
          s->function->return_type->start
        );
      }
      // print body
      print_statement( s->function->body );
      // print load stuff
      if ( s->function->load_identifier ) {
        fprintf(
          stdout, " = load %*.s",
          ( int )s->function->load_identifier->length,
          s->function->load_identifier->start
        );
      }
      // closing block
      fprintf( stdout, ")" );
      break;
    }
    case STATEMENT_VARIABLE: {
      // opening block
      fprintf(
        stdout, "(let %*.*s",
        ( int )s->variable->name->length,
        ( int )s->variable->name->length,
        s->variable->name->start
      );
      // possible initializer
      if ( s->variable->initializer ) {
        fprintf( stdout, " = " );
        print_expression( s->variable->initializer );
      }
      // closing block
      fprintf( stdout, ")" );
      break;
    }
    case STATEMENT_CONST: {
      // opening block
      fprintf(
        stdout, "(const %*.*s = ",
        ( int )s->constant->name->length,
        ( int )s->constant->name->length,
        s->constant->name->start
      );
      // initializer
      print_expression( s->constant->initializer );
      // closing block
      fprintf( stdout, ")" );
      break;
    }
    case STATEMENT_IF: {
      if ( ! s->if_else->else_statement ) {
        // opening block
        fprintf( stdout, "(if " );
        // condition and statement
        print_expression( s->if_else->if_condition );
        print_statement( s->if_else->if_statement );
      } else {
        // opening block
        fprintf( stdout, "(if-else " );
        // condition and statement
        print_expression( s->if_else->if_condition );
        print_statement( s->if_else->if_statement );
        print_statement( s->if_else->else_statement );
      }
      // closing block
      fprintf( stdout, ")" );
      break;
    }
    case STATEMENT_PRINT: {
      // opening block
      fprintf( stdout, "(print " );
      // print expression
      print_expression( s->print->expression );
      // closing block
      fprintf( stdout, ")" );
      break;
    }
    case STATEMENT_RETURN: {
      if ( ! s->return_value->value ) {
        fprintf( stdout, "(return)" );
      } else {
        // opening block
        fprintf( stdout, "(return " );
        // print expression
        print_expression( s->return_value->value );
        // closing block
        fprintf( stdout, ")" );
      }
      break;
    }
    case STATEMENT_WHILE: {
      // opening block
      fprintf( stdout, "(while " );
      // condition
      print_expression( s->while_loop->condition );
      // space
      fprintf( stdout, " " );
      // body
      print_statement( s->while_loop->body );
      // closing block
      fprintf( stdout, ")" );
      break;
    }
    case STATEMENT_BLOCK: {
      // opening block
      fprintf( stdout, "(block " );
      // print all inner statements
      for ( list_item_t* c = s->block->statements->first; c; c = c->next ) {
        if ( c != s->block->statements->first ) {
          fprintf( stdout, " " );
        }
        print_statement( c->data );
      }
      // closing block
      fprintf( stdout, ")" );
      break;
    }
    case STATEMENT_POINTER: {
      // opening block
      fprintf(
        stdout, "(p %*.*s ",
        ( int )s->pointer->name->length,
        ( int )s->pointer->name->length,
        s->pointer->name->start
      );
      // condition
      print_statement( s->pointer->statement );
      // closing block
      fprintf( stdout, ")" );
      break;
    }
    case STATEMENT_EXPRESSION: {
      // opening block
      fprintf( stdout, "(; " );
      // print expression
      print_expression( s->expression->expression );
      // closing block
      fprintf( stdout, ")" );
      break;
    }
    case STATEMENT_PARAMETER: {
      // handled by function
      break;
    }
  }
}

/**
 * @brief Helper to print a node
 *
 * @param n
 */
static void print_node( bosl_ast_node_t* n ) {
  print_statement( n->statement );
  // flush output
  fflush( stdout );
}

/**
 * @brief Method to print parsed ast
 */
void bosl_parser_print( void ) {
  // handle not initialized
  if ( ! parser ) {
    return;
  }
  // loop through nodes and print them
  for (
    list_item_t* current = parser->ast->first;
    current;
    current = current->next
  ) {
    print_node( current->data );
  }
  // final newline
  fprintf( stdout, "\r\n" );
}
