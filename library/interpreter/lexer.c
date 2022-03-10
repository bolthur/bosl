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
 * @fn char advance(void)
 * @brief Push current to next character and return previous
 *
 * @return
 */
static char advance( void ) {
  lexer->current++;
  return lexer->current[ -1 ];
}

/**
 * @fn char next(void)
 * @brief Return next character without stepping to next
 *
 * @return
 */
static char next( void ) {
  if ( ! *lexer->current ) {
    return '\0';
  }
  return lexer->current[ 1 ];
}

/**
 * @fn bool match(char)
 * @brief Check if expected character is found
 *
 * @param expected
 * @return
 */
static bool match( char expected ) {
  if ( ! *lexer->current ) {
    return false;
  }
  if ( lexer->current[ 0 ] != expected ) {
    return false;
  }
  lexer->current++;
  return true;
}

/**
 * @fn bool add_token(bosl_token_type_t, const char*)
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
    token->start = lexer->current;
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
 * @fn bool handle_possible_keyword(uint32_t, uint32_t, const char*, bosl_token_type_t)
 * @brief Handle possible keyword and generate token
 *
 * @param start
 * @param length
 * @param compare
 * @param type
 * @return
 */
static bool handle_possible_keyword(
  uint32_t start,
  uint32_t length,
  const char* compare,
  bosl_token_type_t type
) {
  if (
    ( uint32_t )( lexer->current - lexer->start ) == start + length
    && ! memcmp( lexer->start + start, compare, length )
  ) {
    return add_token( type, NULL );
  }
  return add_token( TOKEN_IDENTIFIER, NULL );
}

/**
 * @fn bool determine_identifier_token(void)
 * @brief Try to determine possible identifier
 *
 * @return
 */
static bool determine_identifier_token( void ) {
  switch ( *lexer->start ) {
    // if elseif else
    case 'i':
      if ( 'f' == lexer->start[ 1 ] ) {
        return handle_possible_keyword( 1, 1, "f", TOKEN_IF );
      } else if ( 2 < lexer->current - lexer->start && 'n' == lexer->start[ 1 ] ) {
        switch( *( lexer->start + 3 ) ) {
          case '8': return handle_possible_keyword( 1, 3, "nt8", TOKEN_TYPE_INT8 );
          case '1': return handle_possible_keyword( 1, 4, "nt16", TOKEN_TYPE_INT16 );
          case '3': return handle_possible_keyword( 1, 4, "nt32", TOKEN_TYPE_INT32 );
          case '6': return handle_possible_keyword( 1, 4, "nt64", TOKEN_TYPE_INT64 );
        }
      }
      break;
    case 'e': return handle_possible_keyword( 1, 3, "lse", TOKEN_ELSE );
    // let / const
    case 'l': return handle_possible_keyword( 1, 2, "et", TOKEN_IF );
    case 'c': return handle_possible_keyword( 1, 4, "onst", TOKEN_CONST );
      break;
    // reference / return
    case 'r': {
      if ( 2 < lexer->current - lexer->start && 'e' == lexer->start[ 1 ] ) {
        switch( *( lexer->start + 2 ) ) {
          case 'f': return handle_possible_keyword( 1, 2, "ef", TOKEN_REFERENCE );
          case 't': return handle_possible_keyword( 3, 3, "urn", TOKEN_REFERENCE );
        }
      }
      break;
    }
    // null
    case 'n': return handle_possible_keyword( 1, 3, "ull", TOKEN_NULL );
    // true
    case 't': return handle_possible_keyword( 1, 3, "rue", TOKEN_TRUE );
      break;
    // false / for / fn
    case 'f': {
      if ( 'n' == lexer->start[ 1 ] ) {
        return handle_possible_keyword( 1, 1, "n", TOKEN_FUNCTION );
      }
      if ( 1 < lexer->current - lexer->start ) {
        switch ( lexer->start[ 1 ] ) {
          case 'a': return handle_possible_keyword( 2, 3, "lse", TOKEN_FALSE );
          case 'o': return handle_possible_keyword( 2, 1, "r", TOKEN_FOR );
          case 'l': {
            if ( 5 < lexer->current - lexer->start ) {
              switch( *( lexer->start + 5 ) ) {
                case '8': return handle_possible_keyword( 3, 4, "oat8", TOKEN_TYPE_FLOAT8 );
                case '1': return handle_possible_keyword( 3, 5, "oat16", TOKEN_TYPE_FLOAT16 );
                case '3': return handle_possible_keyword( 3, 5, "oat32", TOKEN_TYPE_FLOAT32 );
                case '6': return handle_possible_keyword( 3, 5, "oat64", TOKEN_TYPE_FLOAT64 );
              }
            }
            break;
          }
        }
      }
      break;
    }
    // handle while
    case 'w': return handle_possible_keyword( 1, 4, "hile", TOKEN_WHILE );
    // handle print
    case 'p': return handle_possible_keyword( 1, 4, "rint", TOKEN_PRINT );
    // handle data type string
    case 's': return handle_possible_keyword( 1, 5, "tring", TOKEN_TYPE_STRING );
    // unsigned data types
    case 'u':
      if ( 4 < lexer->current - lexer->start && 'i' == lexer->start[ 1 ] ) {
        switch ( *( lexer->start + 4 ) ) {
          case '8': return handle_possible_keyword( 2, 3, "nt8", TOKEN_TYPE_UINT8 );
          case '1': return handle_possible_keyword( 2, 4, "nt16", TOKEN_TYPE_UINT16 );
          case '3': return handle_possible_keyword( 2, 4, "nt32", TOKEN_TYPE_UINT32 );
          case '6': return handle_possible_keyword( 2, 4, "nt64", TOKEN_TYPE_UINT64 );
        }
      }
      if ( 5 < lexer->current - lexer->start && 'f' == lexer->start[ 1 ] ) {
        switch ( *( lexer->start + 6 ) ) {
          case '8': return handle_possible_keyword( 2, 5, "loat8", TOKEN_TYPE_UFLOAT8 );
          case '1': return handle_possible_keyword( 2, 6, "loat16", TOKEN_TYPE_UFLOAT16 );
          case '3': return handle_possible_keyword( 2, 6, "loat32", TOKEN_TYPE_UFLOAT32 );
          case '6': return handle_possible_keyword( 2, 6, "loat64", TOKEN_TYPE_UFLOAT64 );
        }
      }
      break;
    // void type
    case 'v': return handle_possible_keyword( 1, 3, "oid", TOKEN_TYPE_VOID );
    // bool type
    case 'b': return handle_possible_keyword( 1, 3, "ool", TOKEN_TYPE_BOOL );
  }
  // token is a normal identifier
  return add_token( TOKEN_IDENTIFIER, NULL );
}

/**
 * @fn bool scan_string(void)
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
  if ( ! *lexer->current ) {
    return add_token( TOKEN_ERROR, "Unterminated string found" );
  }
  // get beyond closing double quotes
  advance();
  // return string token
  return add_token( TOKEN_STRING, NULL );
}

/**
 * @fn bool scan_identifier(void)
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
  // add token
  return determine_identifier_token();
}

/**
 * @fn bool scan_number(void)
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
  // continue loop if hex or float
  if (
    isdigit( ( int )next() )
    && (
      'x' == *lexer->current
      || 'X' == *lexer->current
      || '.' == *lexer->current
    )
  ) {
    // skip xX.
    advance();
    // loop until non digit
    while ( isdigit( ( int )*lexer->current ) ) {
      advance();
    }
  }
  // add token
  return add_token( TOKEN_NUMBER, NULL );
}

/**
 * @fn bool scan_token(void)
 * @brief Helper to scan a token
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
    case '.': return add_token( TOKEN_DOT, NULL );
    case ':': return add_token( TOKEN_COLON, NULL );
    case ';': return add_token( TOKEN_SEMICOLON, NULL );
    case '-': return add_token( TOKEN_MINUS, NULL );
    case '+': return add_token( TOKEN_PLUS, NULL );
    case '*': return add_token( TOKEN_STAR, NULL );
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
    case '|': return add_token( match( '&' ) ? TOKEN_OR_OR : TOKEN_OR, NULL );
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
 * @fn void token_list_cleanup(list_item_t*)
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
 * @fn bool lexer_init(const char*)
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
  // return success
  return true;
}

/**
 * @fn void lexer_free(void)
 * @brief Destroy lexer if initialized
 */
void lexer_free( void ) {
  // skip if no instance allocated
  if ( ! lexer ) {
    return;
  }
  // destroy list and free instance
  list_destruct( lexer->token );
  free( lexer );
}

/**
 * @fn list_manager_t* lexer_scan(void)
 * @brief Scan and return list of tokens
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
  // add eof token
  if ( ! add_token( TOKEN_EOF, NULL ) ) {
    lexer_free();
    return NULL;
  }
  // return list of tokens
  return lexer->token;
}
