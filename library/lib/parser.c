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
__unused static bool match( bosl_token_type_t type ) {
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
__unused static bool consume( bosl_token_type_t type, const char* error_message ) {
  // get token
  bosl_token_t* token = get_token( parser->current );
  // check for mismatch
  if ( token->type != type ) {
    // raise error and return false
    error_raise( token, error_message );
    return false;
  }
  // head over to next
  advance();
  // return success
  return true;
}

__unused static bosl_ast_expression_t* primary( void ) {
  return NULL;
}

/**
 * @brief Setup parser
 *
 * @param token
 * @return
 */
bool parser_init( list_manager_t* token ) {
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
void parser_free( void ) {
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
list_manager_t* parser_scan( void ) {
  // handle not initialized
  if ( ! parser ) {
    return NULL;
  }
  // return built byte code
  return parser->ast;
}

/**
 * @brief Method to print parsed ast
 */
void parser_print( void ) {
}
