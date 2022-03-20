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
#include "scanner.h"

static bosl_scanner_t* scanner = NULL;

/**
 * @brief Push current to next character and return previous
 *
 * @return
 */
static char advance( void ) {
  scanner->current++;
  return scanner->current[ -1 ];
}

/**
 * @brief Return next character without stepping to next
 *
 * @return
 */
static char next( void ) {
  if ( '\0' == scanner->current[ 0 ] ) {
    return '\0';
  }
  return scanner->current[ 1 ];
}

/**
 * @brief Check if expected character is found
 *
 * @param expected
 * @return
 */
static bool match( char expected ) {
  if (
    '\0' == scanner->current[ 0 ]
    || scanner->current[ 0 ] != expected
  ) {
    return false;
  }
  scanner->current++;
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
  if ( ! scanner || ! scanner->token ) {
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
    token->start = scanner->start;
    token->length = ( size_t )( scanner->current - scanner->start );
  }
  // fill token
  token->type = type;
  token->line = scanner->line;
  // try to push back
  if ( ! list_push_back_data( scanner->token, token ) ) {
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
  if ( ! scanner || ! scanner->token ) {
    return false;
  }
  // loop until second double quotes
  while ( *scanner->current && '"' != *scanner->current ) {
    // handle line break
    if ( '\n' == *scanner->current ) {
      scanner->line++;
    }
    advance();
  }
  // handle end reached
  if ( '\0' == scanner->current[ 0 ] ) {
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
  if ( ! scanner || ! scanner->token ) {
    return false;
  }
  // get beyond alpha numeric
  while (
    *scanner->current
    && (
      isalnum( ( int )*scanner->current )
      || '_' == *scanner->current
    )
  ) {
    advance();
  }
  // try to get type
  void* raw_type = hashmap_value_get( scanner->keyword, scanner->start );
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
  if ( ! scanner || ! scanner->token ) {
    return false;
  }
  // loop until non digit
  while ( isdigit( ( int )*scanner->current ) ) {
    advance();
  }
  bool is_hex = 'x' == *scanner->current || 'X' == *scanner->current;
  bool is_float = '.' == *scanner->current;
  // continue loop if hex or float
  if (
    ( is_float && isdigit( ( int )next() ) )
    || ( is_hex && isalnum( ( int )next() ) )
  ) {
    // skip xX.
    advance();
    // loop until non digit
    while (
      ( is_float && isdigit( ( int )*scanner->current ) )
      || ( is_hex && isalnum( ( int )*scanner->current ) )
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
        while ( *scanner->current && '\n' != *scanner->current ) {
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
      scanner->line++;
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
 * @brief Prepare and setup scanner
 *
 * @param source
 * @return
 */
bool bosl_scanner_init( const char* source ) {
  // prevent duplicate init
  if ( scanner ) {
    return false;
  }
  // allocate and handle error
  scanner = malloc( sizeof( *scanner ) );
  if ( ! scanner ) {
    return false;
  }
  // clear out
  memset( scanner, 0, sizeof( *scanner ) );
  // populate
  scanner->source = source;
  scanner->start = source;
  scanner->current = source;
  scanner->line = 1;
  // generate token list
  scanner->token = list_construct( NULL, token_list_cleanup, NULL );
  if ( ! scanner->token ) {
    free( scanner );
    return false;
  }
  // create and fill hash map
  scanner->keyword = hashmap_construct();
  if ( ! scanner->keyword ) {
    list_destruct( scanner->token );
    free( scanner );
    return false;
  }
  // populate hashmap with key words
  if (
    ! hashmap_value_set( scanner->keyword, "let", ( void* )TOKEN_LET )
    || ! hashmap_value_set( scanner->keyword, "const", ( void* )TOKEN_CONST )
    || ! hashmap_value_set( scanner->keyword, "pointer", ( void* )TOKEN_POINTER )
    || ! hashmap_value_set( scanner->keyword, "true", ( void* )TOKEN_TRUE )
    || ! hashmap_value_set( scanner->keyword, "false", ( void* )TOKEN_FALSE )
    || ! hashmap_value_set( scanner->keyword, "null", ( void* )TOKEN_NULL )
    || ! hashmap_value_set( scanner->keyword, "if", ( void* )TOKEN_IF )
    || ! hashmap_value_set( scanner->keyword, "elseif", ( void* )TOKEN_ELSEIF )
    || ! hashmap_value_set( scanner->keyword, "else", ( void* )TOKEN_ELSE )
    || ! hashmap_value_set( scanner->keyword, "while", ( void* )TOKEN_WHILE )
    || ! hashmap_value_set( scanner->keyword, "for", ( void* )TOKEN_FOR )
    || ! hashmap_value_set( scanner->keyword, "fn", ( void* )TOKEN_FUNCTION )
    || ! hashmap_value_set( scanner->keyword, "return", ( void* )TOKEN_RETURN )
    || ! hashmap_value_set( scanner->keyword, "load", ( void* )TOKEN_LOAD )
    || ! hashmap_value_set( scanner->keyword, "int8", ( void* )TOKEN_TYPE_IDENTIFIER )
    || ! hashmap_value_set( scanner->keyword, "int16", ( void* )TOKEN_TYPE_IDENTIFIER )
    || ! hashmap_value_set( scanner->keyword, "int32", ( void* )TOKEN_TYPE_IDENTIFIER )
    || ! hashmap_value_set( scanner->keyword, "int64", ( void* )TOKEN_TYPE_IDENTIFIER )
    || ! hashmap_value_set( scanner->keyword, "uint8", ( void* )TOKEN_TYPE_IDENTIFIER )
    || ! hashmap_value_set( scanner->keyword, "uint16", ( void* )TOKEN_TYPE_IDENTIFIER )
    || ! hashmap_value_set( scanner->keyword, "uint32", ( void* )TOKEN_TYPE_IDENTIFIER )
    || ! hashmap_value_set( scanner->keyword, "uint64", ( void* )TOKEN_TYPE_IDENTIFIER )
    || ! hashmap_value_set( scanner->keyword, "float", ( void* )TOKEN_TYPE_IDENTIFIER )
    || ! hashmap_value_set( scanner->keyword, "string", ( void* )TOKEN_TYPE_IDENTIFIER )
    || ! hashmap_value_set( scanner->keyword, "void", ( void* )TOKEN_TYPE_IDENTIFIER )
    || ! hashmap_value_set( scanner->keyword, "bool", ( void* )TOKEN_TYPE_IDENTIFIER )
    || ! hashmap_value_set( scanner->keyword, "print", ( void* )TOKEN_PRINT )
  ) {
    hashmap_destruct( scanner->keyword );
    list_destruct( scanner->token );
    free( scanner );
    return false;
  }
  // return success
  return true;
}

/**
 * @brief Destroy scanner if initialized
 */
void bosl_scanner_free( void ) {
  // skip if no instance allocated
  if ( ! scanner ) {
    return;
  }
  // destroy list and hash map
  list_destruct( scanner->token );
  hashmap_destruct( scanner->keyword );
  // finally free instance
  free( scanner );
}

/**
 * @brief Scan and return list of tokens
 *
 * @return
 */
list_manager_t* bosl_scanner_scan( void ) {
  // handle call without init
  if ( ! scanner ) {
    return NULL;
  }
  // loop until end
  while ( *scanner->current ) {
    // set start to current
    scanner->start = scanner->current;
    // scan token
    if ( ! scan_token() ) {
      bosl_scanner_free();
      return NULL;
    }
  }
  // push scanner start one more time
  scanner->start = scanner->current;
  // add eof token
  if ( ! add_token( TOKEN_EOF, NULL ) ) {
    bosl_scanner_free();
    return NULL;
  }
  // return list of tokens
  return scanner->token;
}
