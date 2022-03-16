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
#include "parser.h"
#include "lexer.h"
#include "error.h"

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
  if ( type != get_token( parser->current->data )->type ) {
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
    error_raise( token, error_message );
    return false;
  }
  // head over to next
  advance();
  // return success
  return true;
}

static bool handle_print( void ) {
  // consume left parenthesis
  if ( ! consume( TOKEN_LEFT_PARENTHESIS, "Expect '(' after print." ) ) {
    return false;
  }
  // FIXME: CONSUME EXPRESSION
  // consume right parenthesis
  if ( ! consume( TOKEN_RIGHT_PARENTHESIS, "Expect ')' after parameter." ) ) {
    return false;
  }
  // consume semicolon
  if ( ! consume( TOKEN_SEMICOLON, "Expect ';' at the end." ) ) {
    return false;
  }
  // FIXME: PUSH SOME OP CODE STUFF
  // return success
  return true;
}

static bool handle_for( void ) {
  return true;
}

static bool handle_while( void ) {
  return true;
}

static bool handle_if( void ) {
  return true;
}

static bool handle_return( void ) {
  return true;
}

static bool handle_block( void ) {
  return true;
}

static bool handle_expression( void ) {
  return true;
}

static bool handle_statement( void ) {
  // handle print
  if ( match( TOKEN_PRINT ) ) {
    return handle_print();
  // handle if
  } else if ( match( TOKEN_IF ) ) {
    return handle_if();
  // handle for
  } else if ( match( TOKEN_FOR ) ) {
    return handle_for();
  // handle while
  } else if ( match( TOKEN_WHILE ) ) {
    return handle_while();
  // handle return
  } else if ( match( TOKEN_RETURN ) ) {
    return handle_return();
  // handle group start
  } else if ( match( TOKEN_LEFT_BRACE ) ) {
    return handle_block();
  // handle expression
  } else {
    return handle_expression();
  }
  return false;
}

static bool handle_variable_declaration( void ) {
  return true;
}

static bool handle_constant_declaration( void ) {
  return true;
}

static bool handle_function_declaration( void ) {
  return true;
}

/**
 * @brief Process the sequence starting at current
 *
 * @return
 */
static bool process_sequence( void ) {
  // handle variable creation
  if ( match( TOKEN_LET ) ) {
    return handle_variable_declaration();
  // handle constant creation
  } else if ( match( TOKEN_CONST ) ) {
    return handle_constant_declaration();
  // handle function declaration
  } else if ( match( TOKEN_FUNCTION ) ) {
    return handle_function_declaration();
  } else {
    return handle_statement();
  }
}

/**
 * @brief Setup parser
 *
 * @param token
 * @return
 */
bool parser_init( list_manager_t* token ) {
  // allocate parser structure
  parser = malloc( sizeof( bosl_parser_t ) );
  if ( ! parser ) {
    return false;
  }
  // clear out
  memset( parser, 0, sizeof( bosl_parser_t ) );
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
  // loop until eof
  while ( ! match( TOKEN_EOF ) ) {
    // try to process sequence
    if ( ! process_sequence() ) {
      return false;
    }
  }
  // return built byte code
  return parser->ast;
}
