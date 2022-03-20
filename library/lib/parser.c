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
#include "parser.h"
#include "scanner.h"
#include "error.h"
#include "ast/expression.h"
#include "ast/statement.h"
#include "ast/common.h"

// necessary forward declaration
static bosl_ast_expression_t* expression( void );

static bosl_parser_t* parser = NULL;

/**
 * @brief Get the token object
 *
 * @param item
 * @return
 */
static bosl_token_t* get_token( list_item_t* item ) {
  return item->data;
}

/**
 * @brief Method to push to next token
 */
static void advance( void ) {
  parser->current = parser->current->next;
}

/**
 * @brief Helper to advance to next token on match with passed type
 *
 * @param type
 * @return
 */
static bool match( bosl_token_type_t type ) {
  // return false if not matching
  if ( type != get_token( parser->current )->type ) {
    return false;
  }
  // push to next
  advance();
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
static bool consume( bosl_token_type_t type, const char* error_message ) {
  // get token
  bosl_token_t* token = get_token( parser->current );
  // check for mismatch
  if ( token->type != type ) {
    // raise error and return false
    bosl_error_raise( token, error_message );
    return false;
  }
  // head over to next
  advance();
  // return success
  return true;
}

/**
 * @brief Handle primary expression
 *
 * @return
 */
static bosl_ast_expression_t* expression_primary( void ) {
  if ( match( TOKEN_FALSE ) ) {
    return bosl_ast_expression_allocate_literal( ( void* )false, sizeof( false ) );
  }
  if ( match( TOKEN_TRUE ) ) {
    return bosl_ast_expression_allocate_literal( ( void* )true, sizeof( true ) );
  }
  if ( match( TOKEN_NULL ) ) {
    return bosl_ast_expression_allocate_literal( NULL, sizeof( NULL ) );
  }

  if ( match( TOKEN_NUMBER ) || match( TOKEN_STRING ) ) {
    bosl_token_t* token = get_token( parser->current->previous );
    // FIXME: TRANSLATE TOKEN TO NUMBER IN CASE OF NUMBER
    return bosl_ast_expression_allocate_literal(
      token->start,
      sizeof( char ) * ( token->length )
    );
  }

  if ( match( TOKEN_IDENTIFIER ) ) {
    // create group expression
    bosl_ast_expression_t* new_e = bosl_ast_expression_allocate( EXPRESSION_VARIABLE );
    if ( ! new_e ) {
      // FIXME: DESTROY e
      return NULL;
    }
    // get pointer to data
    new_e->variable->name = get_token( parser->current->previous );
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
      // FIXME: DESTROY e
      return NULL;
    }
    // create group expression
    bosl_ast_expression_t* new_e = bosl_ast_expression_allocate( EXPRESSION_GROUPING );
    if ( ! new_e ) {
      // FIXME: DESTROY e
      return NULL;
    }
    // get pointer to data
    new_e->grouping->expression = e;
    // return built expression
    return new_e;
  }
  // raise error
  bosl_error_raise( get_token( parser->current ), "Expected expression." );
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
  bosl_token_t* current = get_token( parser->current );
  // create arguments list
  list_manager_t* arguments = list_construct( NULL, NULL, NULL );
  if ( ! arguments ) {
    return NULL;
  }
  // handle possible arguments
  if ( TOKEN_RIGHT_PARENTHESIS != current->type ) {
    do {
      // get argument expression
      bosl_ast_expression_t *arg = expression();
      if ( ! arg ) {
        // FIXME: DESTROY arguments
        return NULL;
      }
      // push back
      if ( ! list_push_back_data( arguments, arg ) ) {
        // FIXME: DESTROY arguments and arg
        return NULL;
      }
    } while ( match( TOKEN_COMMA ) );
  }
  // check for closing parenthesis
  if ( ! consume( TOKEN_RIGHT_PARENTHESIS, "Expected ')' after arguments." ) ) {
    return NULL;
  }
  // get closing parenthesis token
  bosl_token_t* previous = get_token( parser->current->previous );
  // allocate call expression
  bosl_ast_expression_t* e = bosl_ast_expression_allocate( EXPRESSION_CALL );
  if ( ! e ) {
    // FIXME: DESTROY arguments
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
        // FIXME: DESTROY e
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
    new_e->load->name = get_token( parser->current->previous );
    // return built expression
    return new_e;
  }
  bosl_error_raise( get_token( parser->current ), "Expect identifier after load." );
  //bosl_ast_expression_t
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
    bosl_token_t* operator = get_token( parser->current->previous );
    bosl_ast_expression_t* right = expression_unary();
    if ( ! right ) {
      // FIXME: DESTROY e
      return NULL;
    }
    bosl_ast_expression_t* new_e = bosl_ast_expression_allocate( EXPRESSION_UNARY );
    if ( ! new_e ) {
      // FIXME: DESTROY e and right
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
    bosl_token_t* operator = get_token( parser->current->previous );
    bosl_ast_expression_t* right = expression_unary();
    if ( ! right ) {
      // FIXME: DESTROY e
      return NULL;
    }
    bosl_ast_expression_t* new_e = bosl_ast_expression_allocate_binary(
      e, operator, right );
    if ( ! new_e ) {
      // FIXME: DESTROY e and right
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
    bosl_token_t* operator = get_token( parser->current->previous );
    bosl_ast_expression_t* right = expression_factor();
    if ( ! right ) {
      // FIXME: DESTROY e
      return NULL;
    }
    bosl_ast_expression_t* new_e = bosl_ast_expression_allocate_binary(
      e, operator, right );
    if ( ! new_e ) {
      // FIXME: DESTROY e and right
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
    bosl_token_t* operator = get_token( parser->current->previous );
    bosl_ast_expression_t* right = expression_term();
    if ( ! right ) {
      // FIXME: DESTROY e
      return NULL;
    }
    bosl_ast_expression_t* new_e = bosl_ast_expression_allocate_binary(
      e, operator, right );
    if ( ! new_e ) {
      // FIXME: DESTROY e and right
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
    bosl_token_t* operator = get_token( parser->current->previous );
    bosl_ast_expression_t* right = expression_comparison();
    if ( ! right ) {
      // FIXME: DESTROY e
      return NULL;
    }
    bosl_ast_expression_t* new_e = bosl_ast_expression_allocate_binary(
      e, operator, right );
    if ( ! new_e ) {
      // FIXME: DESTROY e and right
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
    bosl_token_t* operator = get_token( parser->current->previous );
    // evaluate right expression
    bosl_ast_expression_t* right = expression_equality();
    if ( ! right ) {
      // FIXME: DESTROY e
      return NULL;
    }
    bosl_ast_expression_t* new_e = bosl_ast_expression_allocate_logical(
      e, operator, right );
    if ( ! new_e ) {
      // FIXME: DESTROY e and right
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
    bosl_token_t* operator = get_token( parser->current->previous );
    // evaluate right expression
    bosl_ast_expression_t* right = expression_and();
    if ( ! right ) {
      // FIXME: DESTROY e
      return NULL;
    }
    bosl_ast_expression_t* new_e = bosl_ast_expression_allocate_logical(
      e, operator, right );
    if ( ! new_e ) {
      // FIXME: DESTROY e and right
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
    bosl_token_t* operator = get_token( parser->current->previous );
    // evaluate right expression
    bosl_ast_expression_t* right = expression_xor();
    if ( ! right ) {
      // FIXME: DESTROY e
      return NULL;
    }
    bosl_ast_expression_t* new_e = bosl_ast_expression_allocate_logical(
      e, operator, right );
    if ( ! new_e ) {
      // FIXME: DESTROY e and right
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
    bosl_token_t* operator = get_token( parser->current->previous );
    // evaluate right expression
    bosl_ast_expression_t* right = expression_or();
    if ( ! right ) {
      // FIXME: DESTROY e
      return NULL;
    }
    bosl_ast_expression_t* new_e = bosl_ast_expression_allocate_logical(
      e, operator, right );
    if ( ! new_e ) {
      // FIXME: DESTROY e and right
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
    bosl_token_t* operator = get_token( parser->current->previous );
    // evaluate right expression
    bosl_ast_expression_t* right = expression_logic_and();
    if ( ! right ) {
      // FIXME: DESTROY e
      return NULL;
    }
    bosl_ast_expression_t* new_e = bosl_ast_expression_allocate_logical(
      e, operator, right );
    if ( ! new_e ) {
      // FIXME: DESTROY e and right
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
    bosl_token_t* previous = get_token( parser->current->previous );
    // get value for assignment
    bosl_ast_expression_t* value = expression_assignment();
    if ( ! value ) {
      // FIXME: DESTROY e
      return NULL;
    }

    // handle variable
    if ( EXPRESSION_VARIABLE == e->type ) {
      // build and return assign expression
      bosl_ast_expression_t* new_e = bosl_ast_expression_allocate( EXPRESSION_ASSIGN );
      if ( ! new_e ) {
        // FIXME: DESTROY e
        return NULL;
      }
      // populate inner data
      new_e->assign->token = e->variable->name;
      new_e->assign->value = value;
      // return assign expression
      // FIXME: DESTROY e
      return new_e;
    }

    bosl_error_raise( previous, "Invalid assignment target." );
    // FIXME: DESTROY e
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
 * @brief Handle for statement
 *
 * @return
 *
 * @todo add logic
 */
static bosl_ast_node_t* statement_for( void ) {
  //bosl_ast_statement_t
  return NULL;
}

/**
 * @brief Handle if statement
 *
 * @return
 *
 * @todo add logic
 */
static bosl_ast_node_t* statement_if( void ) {
  //bosl_ast_statement_t
  return NULL;
}

/**
 * @brief Handle print statement
 *
 * @return
 *
 * @todo add logic
 */
static bosl_ast_node_t* statement_print( void ) {
  //bosl_ast_statement_t
  return NULL;
}

/**
 * @brief Handle return statement
 *
 * @return
 *
 * @todo add logic
 */
static bosl_ast_node_t* statement_return( void ) {
  //bosl_ast_statement_t
  return NULL;
}

/**
 * @brief Handle while statement
 *
 * @return
 *
 * @todo add logic
 */
static bosl_ast_node_t* statement_while( void ) {
  //bosl_ast_statement_t
  return NULL;
}

/**
 * @brief Handle block statement
 *
 * @return
 *
 * @todo add logic
 */
static bosl_ast_node_t* statement_block( void ) {
  //bosl_ast_statement_t
  return NULL;
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
    // FIXME: DESTROY EXPRESSION
    return NULL;
  }
  // allocate new ast node
  bosl_ast_node_t* new_node = bosl_ast_node_allocate( NODE_STATEMENT );
  if ( ! new_node ) {
    // FIXME: DESTROY EXPRESSION
    return NULL;
  }
  // allocate new ast node
  new_node->statement = bosl_ast_statement_allocate( STATEMENT_EXPRESSION );
  if ( ! new_node->statement ) {
    // FIXME: DESTROY EXPRESSION
    // FIXME: DESTROY NEW NODE
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
  if ( match( TOKEN_FOR ) ) {
    return statement_for();
  }
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
  return statement_expression();
}

/**
 * @brief Handle constant declaration
 *
 * @return
 */
static bosl_ast_node_t* declaration_const( void ) {
  if ( ! consume( TOKEN_IDENTIFIER, "Expect variable name." ) ) {
    return NULL;
  }
  bosl_token_t* name = get_token( parser->current->previous );
  if ( ! consume( TOKEN_COLON, "Expect colon after variable name." ) ) {
    return NULL;
  }
  if ( ! consume( TOKEN_TYPE_IDENTIFIER, "Expect type identifier after colon." ) ) {
    return NULL;
  }
  // get type identifier
  bosl_token_t* type = get_token( parser->current->previous );
  // handle missing initializer
  if ( ! match( TOKEN_EQUAL ) ) {
    bosl_error_raise( name, "Constants need an initializer." );
    return NULL;
  }
  // get initializer
  bosl_ast_expression_t* initializer = expression();
  // assert semicolon at the end
  if ( ! consume( TOKEN_SEMICOLON, "Expect ';' after variable declaration." ) ) {
    return NULL;
  }
  // allocate node
  bosl_ast_node_t* node = bosl_ast_node_allocate( NODE_STATEMENT );
  if ( ! node ) {
    return NULL;
  }
  // allocate statement
  node->statement = bosl_ast_statement_allocate( STATEMENT_VARIABLE );
  if ( ! node->statement ) {
    // FIXME: DESTROY node
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
 * @brief Handle variable declaration
 *
 * @return
 */
static bosl_ast_node_t* declaration_let( void ) {
  if ( ! consume( TOKEN_IDENTIFIER, "Expect variable name." ) ) {
    return NULL;
  }
  bosl_token_t* name = get_token( parser->current->previous );
  if ( ! consume( TOKEN_COLON, "Expect colon after variable name." ) ) {
    return NULL;
  }
  if ( ! consume( TOKEN_TYPE_IDENTIFIER, "Expect type identifier after colon." ) ) {
    return NULL;
  }
  // get type identifier
  bosl_token_t* type = get_token( parser->current->previous );
  // get possible initializer
  bosl_ast_expression_t* initializer = NULL;
  if ( match( TOKEN_EQUAL ) ) {
    initializer = expression();
  }
  // assert semicolon at the end
  if ( ! consume( TOKEN_SEMICOLON, "Expect ';' after variable declaration." ) ) {
    return NULL;
  }
  // allocate node
  bosl_ast_node_t* node = bosl_ast_node_allocate( NODE_STATEMENT );
  if ( ! node ) {
    return NULL;
  }
  // allocate statement
  node->statement = bosl_ast_statement_allocate( STATEMENT_VARIABLE );
  if ( ! node->statement ) {
    // FIXME: DESTROY node
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
 *
 * @todo add logic
 */
static bosl_ast_node_t* declaration_function( void ) {
  //bosl_ast_statement_t
  return NULL;
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
  //bosl_ast_expression_t
  //return NULL;
}

/**
 * @brief Setup parser
 *
 * @param token
 * @return
 */
bool bosl_parser_init( list_manager_t* token ) {
  // allocate parser structure
  parser = malloc( sizeof( *parser ) );
  if ( ! parser ) {
    return false;
  }
  // clear out
  memset( parser, 0, sizeof( *parser ) );
  // push in token list
  parser->token = token;
  parser->current = token->first;
  parser->ast = list_construct( NULL, NULL, NULL );
  if ( ! parser->ast ) {
    free( parser );
    return false;
  }
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
  while ( parser->current ) {
    // handle eof by break
    bosl_token_t* token = get_token( parser->current );
    if ( TOKEN_EOF == token->type ) {
      break;
    }
    // evaluate
    bosl_ast_node_t* tmp = declaration();
    if ( ! tmp ) {
      fprintf( stderr, "no ast node returned!\r\n" );
      // FIXME: CLEAR AST LIST
      return NULL;
    }
    // add to list
    if ( ! list_push_back_data( parser->ast, tmp ) ) {
      bosl_error_raise(
        get_token( parser->current ),
        "Unable to push back ast node!"
      );
      // FIXME: CLEAR AST LIST
      return NULL;
    }
  }
  // return built byte code
  return parser->ast;
}

/**
 * @brief Method to print parsed ast
 */
void bosl_parser_print( void ) {
}
