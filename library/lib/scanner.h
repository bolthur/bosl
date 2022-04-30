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

#include <stdint.h>
#include <stdbool.h>

#if defined( _COMPILING_BOSL )
  #include "collection/list.h"
  #include "collection/hashmap.h"
#else
  #include <bosl/collection/list.h>
  #include <bosl/collection/hashmap.h>
#endif

#if !defined( BOSL_SCANNER_H )
#define BOSL_SCANNER_H

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
  // single character tokens
  TOKEN_LEFT_PARENTHESIS,
  TOKEN_RIGHT_PARENTHESIS,
  TOKEN_LEFT_BRACE,
  TOKEN_RIGHT_BRACE,
  TOKEN_COMMA,
  TOKEN_COLON,
  TOKEN_SEMICOLON,
  TOKEN_MINUS,
  TOKEN_PLUS,
  TOKEN_STAR,
  TOKEN_SLASH,
  TOKEN_MODULO,
  TOKEN_XOR,
  TOKEN_BINARY_ONE_COMPLEMENT,

  // one or two character tokens
  TOKEN_BANG,
  TOKEN_BANG_EQUAL,
  TOKEN_EQUAL,
  TOKEN_EQUAL_EQUAL,
  TOKEN_GREATER,
  TOKEN_GREATER_EQUAL,
  TOKEN_LESS,
  TOKEN_LESS_EQUAL,
  TOKEN_AND,
  TOKEN_AND_AND,
  TOKEN_OR,
  TOKEN_OR_OR,

  // two character tokens
  TOKEN_SHIFT_LEFT,
  TOKEN_SHIFT_RIGHT,

  // literals
  TOKEN_TYPE_IDENTIFIER,
  TOKEN_IDENTIFIER,
  TOKEN_STRING,
  TOKEN_NUMBER,

  // keywords
  TOKEN_LET,
  TOKEN_CONST,
  TOKEN_POINTER,
  TOKEN_TRUE,
  TOKEN_FALSE,
  TOKEN_NULL,
  TOKEN_IF,
  TOKEN_ELSE,
  TOKEN_WHILE,
  TOKEN_BREAK,
  TOKEN_CONTINUE,
  TOKEN_FUNCTION,
  TOKEN_RETURN,
  TOKEN_LOAD,

  // built-in functions
  TOKEN_PRINT,

  // error
  TOKEN_ERROR,
  // end of file
  TOKEN_EOF,
} bosl_token_type_t;

typedef struct bosl_token {
  bosl_token_type_t type;
  const char* start;
  uint32_t line;
  size_t length;
} bosl_token_t;

typedef struct bosl_scanner {
  const char* source;
  const char* start;
  const char* current;
  uint32_t line;
  list_manager_t* token;
  hashmap_table_t* keyword;
} bosl_scanner_t;

bool bosl_scanner_init( const char* );
void bosl_scanner_free( void );
list_manager_t* bosl_scanner_scan( void );

#ifdef __cplusplus
}
#endif

#endif
