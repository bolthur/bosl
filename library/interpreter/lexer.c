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

#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "lexer.h"

static bosl_lexer_t* lexer = NULL;

/**
 * @brief Push current to next character and return previous
 *
 * @return
 */
static char advance( void ) {
  lexer->current++;
  return lexer->current[ -1 ];
}

/**
 * @brief Return next character without stepping to next
 *
 * @return
 */
static char next( void ) {
  if ( '\0' == lexer->current[ 0 ] ) {
    return '\0';
  }
  return lexer->current[ 1 ];
}

/**
 * @brief Check if expected character is found
 *
 * @param expected
 * @return
 */
static bool match( char expected ) {
  if (
    '\0' == lexer->current[ 0 ]
    || lexer->current[ 0 ] != expected
  ) {
    return false;
  }
  lexer->current++;
  return true;
}

/**
 * @brief Helper to add token to list of tokens
 *
 * @param type
 * @param message
 * @return
 */
static bool add_token(
  bosl_token_type_t type,
  const char* message
) {
  // handle not initialized
  if ( ! lexer || ! lexer->token ) {
    return false;
  }
  // message parameter is for error token only
  if ( TOKEN_ERROR != type && message ) {
    return false;
  }
  // allocate token
  bosl_token_t* token = malloc( sizeof( *token ) );
  if ( ! token ) {
    return false;
  }
  // clear out
  memset( token, 0, sizeof( *token ) );
  // populate start / message
  if ( message ) {
    size_t len = strlen( message );
    // allocate space for message
    token->start = malloc( sizeof( char ) * ( len + 1 ) );
    if ( ! token->start ) {
      free( token );
      return false;
    }
    // copy message
    strcpy( ( char* )token->start, message );
    token->length = len;
  } else {
    token->start = lexer->start;
    token->length = ( size_t )( lexer->current - lexer->start );
  }
  // fill token
  token->type = type;
  token->line = lexer->line;
  // try to push back
  if ( ! list_push_back_data( lexer->token, token ) ) {
    if ( message ) {
      free( ( void* )token->start );
    }
    free( token );
    return false;
  }
  // return built token
  return true;
}

/**
 * @brief Scan string
 *
 * @return
 */
static bool scan_string( void ) {
  // handle not initialized
  if ( ! lexer || ! lexer->token ) {
    return false;
  }
  // loop until second double quotes
  while ( *lexer->current && '"' != *lexer->current ) {
    // handle line break
    if ( '\n' == *lexer->current ) {
      lexer->line++;
    }
    advance();
  }
  // handle end reached
  if ( '\0' == lexer->current[ 0 ] ) {
    return add_token( TOKEN_ERROR, "Unterminated string found" );
  }
  // get beyond closing double quotes
  advance();
  // return string token
  return add_token( TOKEN_STRING, NULL );
}

/**
 * @brief Scan identifier
 *
 * @return
 */
static bool scan_identifier( void ) {
  // handle not initialized
  if ( ! lexer || ! lexer->token ) {
    return false;
  }
  // get beyond alpha numeric
  while (
    *lexer->current
    && (
      isalnum( ( int )*lexer->current )
      || '_' == *lexer->current
    )
  ) {
    advance();
  }
  // try to get type
  void* raw_type = hashmap_value_get( lexer->keyword, lexer->start );
  // transform to token type
  bosl_token_type_t type = TOKEN_IDENTIFIER;
  if ( raw_type ) {
    type = ( bosl_token_type_t )raw_type;
  }
  // return created type
  return add_token( type, NULL );
}

/**
 * @brief Scan number
 *
 * @return
 */
static bool scan_number( void ) {
  // handle not initialized
  if ( ! lexer || ! lexer->token ) {
    return false;
  }
  // loop until non digit
  while ( isdigit( ( int )*lexer->current ) ) {
    advance();
  }
  bool is_hex = 'x' == *lexer->current || 'X' == *lexer->current;
  bool is_float = '.' == *lexer->current;
  // continue loop if hex or float
  if (
    ( is_float && isdigit( ( int )next() ) )
    || ( is_hex && isalnum( ( int )next() ) )
  ) {
    // skip xX.
    advance();
    // loop until non digit
    while (
      ( is_float && isdigit( ( int )*lexer->current ) )
      || ( is_hex && isalnum( ( int )*lexer->current ) )
    ) {
      advance();
    }
  }
  // add token
  return add_token( TOKEN_NUMBER, NULL );
}

/**
 * @brief Helper to scan a token
 *
 * @return
 */
static bool scan_token( void ) {
  // get character and increase current to next one
  char current = advance();

  // handle start of number
  if ( isdigit( current ) ) {
    return scan_number();
  // handle start of identifier
  } else if ( isalpha( current ) ) {
    return scan_identifier();
  // handle start of string
  } else if ( '"' == current ) {
    return scan_string();
  }

  // handle possible
  switch ( current ) {
    // single character tokens
    case '(': return add_token( TOKEN_LEFT_PARENTHESIS, NULL );
    case ')': return add_token( TOKEN_RIGHT_PARENTHESIS, NULL );
    case '{': return add_token( TOKEN_LEFT_BRACE, NULL );
    case '}': return add_token( TOKEN_RIGHT_BRACE, NULL );
    case ',': return add_token( TOKEN_COMMA, NULL );
    case ':': return add_token( TOKEN_COLON, NULL );
    case ';': return add_token( TOKEN_SEMICOLON, NULL );
    case '-': return add_token( TOKEN_MINUS, NULL );
    case '+': return add_token( TOKEN_PLUS, NULL );
    case '*': return add_token( TOKEN_STAR, NULL );
    case '%': return add_token( TOKEN_MODULO, NULL );
    case '^': return add_token( TOKEN_XOR, NULL );
    case '~': return add_token( TOKEN_BINARY_ONE_COMPLEMENT, NULL );
    // one or two character tokens
    case '/': {
      // handle comment ( skip )
      if ( match( '/' ) ) {
        while ( *lexer->current && '\n' != *lexer->current ) {
          advance();
        }
      // normal slash token
      } else {
        return add_token( TOKEN_SLASH, NULL );
      }
      break;
    }
    case '!': return add_token( match( '=' ) ? TOKEN_BANG_EQUAL : TOKEN_BANG, NULL );
    case '=': return add_token( match( '=' ) ? TOKEN_EQUAL_EQUAL : TOKEN_EQUAL, NULL );
    case '>': {
      if ( match( '>' ) ) {
        return add_token( TOKEN_SHIFT_RIGHT, NULL );
      }
      return add_token( match( '=' ) ? TOKEN_GREATER_EQUAL : TOKEN_GREATER, NULL );
    }
    case '<': {
      if ( match( '<' ) ) {
        return add_token( TOKEN_SHIFT_LEFT, NULL );
      }
      return add_token( match( '=' ) ? TOKEN_LESS_EQUAL : TOKEN_LESS, NULL );
    }
    case '&': return add_token( match( '&' ) ? TOKEN_AND_AND : TOKEN_AND, NULL );
    case '|': return add_token( match( '|' ) ? TOKEN_OR_OR : TOKEN_OR, NULL );
    // whitespace / newline
    case ' ':
    case '\r':
    case '\t':
      // ignore
      break;
    case '\n':
      lexer->line++;
      break;
    // unknown token
    default: return add_token( TOKEN_ERROR, "Unknown token" );
  }
  // return success
  return true;
}

/**
 * @brief Token list cleanup helper
 *
 * @param a
 */
static void token_list_cleanup( list_item_t* a ) {
  bosl_token_t* token = a->data;
  if ( TOKEN_ERROR == token->type ) {
    free( ( void* )token->start );
  }
  // free inner data structure
  free( token );
  // default cleanup
  list_default_cleanup( a );
}

/**
 * @brief Prepare and setup lexer
 *
 * @param source
 * @return
 */
bool lexer_init( const char* source ) {
  // prevent duplicate init
  if ( lexer ) {
    return false;
  }
  // allocate and handle error
  lexer = malloc( sizeof( *lexer ) );
  if ( ! lexer ) {
    return false;
  }
  // clear out
  memset( lexer, 0, sizeof( *lexer ) );
  // populate
  lexer->source = source;
  lexer->start = source;
  lexer->current = source;
  lexer->line = 1;
  // generate token list
  lexer->token = list_construct( NULL, token_list_cleanup, NULL );
  if ( ! lexer->token ) {
    free( lexer );
    return false;
  }
  // create and fill hash map
  lexer->keyword = hashmap_construct();
  if ( ! lexer->keyword ) {
    list_destruct( lexer->token );
    free( lexer );
    return false;
  }
  // populate hashmap with key words
  if (
    ! hashmap_value_set( lexer->keyword, "if", ( void* )TOKEN_IF )
    || ! hashmap_value_set( lexer->keyword, "elseif", ( void* )TOKEN_ELSEIF )
    || ! hashmap_value_set( lexer->keyword, "else", ( void* )TOKEN_ELSE )
    || ! hashmap_value_set( lexer->keyword, "let", ( void* )TOKEN_LET )
    || ! hashmap_value_set( lexer->keyword, "const", ( void* )TOKEN_CONST )
    || ! hashmap_value_set( lexer->keyword, "return", ( void* )TOKEN_RETURN )
    || ! hashmap_value_set( lexer->keyword, "true", ( void* )TOKEN_TRUE )
    || ! hashmap_value_set( lexer->keyword, "false", ( void* )TOKEN_FALSE )
    || ! hashmap_value_set( lexer->keyword, "null", ( void* )TOKEN_NULL )
    || ! hashmap_value_set( lexer->keyword, "fn", ( void* )TOKEN_FUNCTION )
    || ! hashmap_value_set( lexer->keyword, "for", ( void* )TOKEN_FOR )
    || ! hashmap_value_set( lexer->keyword, "while", ( void* )TOKEN_WHILE )
    || ! hashmap_value_set( lexer->keyword, "print", ( void* )TOKEN_PRINT )
    || ! hashmap_value_set( lexer->keyword, "pointer", ( void* )TOKEN_POINTER )
    || ! hashmap_value_set( lexer->keyword, "string", ( void* )TOKEN_TYPE_STRING )
    || ! hashmap_value_set( lexer->keyword, "bool", ( void* )TOKEN_TYPE_BOOL )
    || ! hashmap_value_set( lexer->keyword, "void", ( void* )TOKEN_TYPE_VOID )
    || ! hashmap_value_set( lexer->keyword, "int8", ( void* )TOKEN_TYPE_INT8 )
    || ! hashmap_value_set( lexer->keyword, "int16", ( void* )TOKEN_TYPE_INT16 )
    || ! hashmap_value_set( lexer->keyword, "int32", ( void* )TOKEN_TYPE_INT32 )
    || ! hashmap_value_set( lexer->keyword, "int64", ( void* )TOKEN_TYPE_INT64 )
    || ! hashmap_value_set( lexer->keyword, "uint8", ( void* )TOKEN_TYPE_UINT8 )
    || ! hashmap_value_set( lexer->keyword, "uint16", ( void* )TOKEN_TYPE_UINT16 )
    || ! hashmap_value_set( lexer->keyword, "uint32", ( void* )TOKEN_TYPE_UINT32 )
    || ! hashmap_value_set( lexer->keyword, "uint64", ( void* )TOKEN_TYPE_UINT64 )
    || ! hashmap_value_set( lexer->keyword, "float8", ( void* )TOKEN_TYPE_FLOAT8 )
    || ! hashmap_value_set( lexer->keyword, "float16", ( void* )TOKEN_TYPE_FLOAT16 )
    || ! hashmap_value_set( lexer->keyword, "float32", ( void* )TOKEN_TYPE_FLOAT32 )
    || ! hashmap_value_set( lexer->keyword, "float64", ( void* )TOKEN_TYPE_FLOAT64 )
    || ! hashmap_value_set( lexer->keyword, "ufloat8", ( void* )TOKEN_TYPE_UFLOAT8 )
    || ! hashmap_value_set( lexer->keyword, "ufloat16", ( void* )TOKEN_TYPE_UFLOAT16 )
    || ! hashmap_value_set( lexer->keyword, "ufloat32", ( void* )TOKEN_TYPE_UFLOAT32 )
    || ! hashmap_value_set( lexer->keyword, "ufloat64", ( void* )TOKEN_TYPE_UFLOAT64 )
  ) {
    hashmap_destruct( lexer->keyword );
    list_destruct( lexer->token );
    free( lexer );
    return false;
  }
  // return success
  return true;
}

/**
 * @brief Destroy lexer if initialized
 */
void lexer_free( void ) {
  // skip if no instance allocated
  if ( ! lexer ) {
    return;
  }
  // destroy list and hash map
  list_destruct( lexer->token );
  hashmap_destruct( lexer->keyword );
  // finally free instance
  free( lexer );
}

/**
 * @brief Scan and return list of tokens
 *
 * @return
 */
list_manager_t* lexer_scan( void ) {
  // handle call without init
  if ( ! lexer ) {
    return NULL;
  }
  // loop until end
  while ( *lexer->current ) {
    // set start to current
    lexer->start = lexer->current;
    // scan token
    if ( ! scan_token() ) {
      lexer_free();
      return NULL;
    }
  }
  // push lexer start one more time
  lexer->start = lexer->current;
  // add eof token
  if ( ! add_token( TOKEN_EOF, NULL ) ) {
    lexer_free();
    return NULL;
  }
  // return list of tokens
  return lexer->token;
}
