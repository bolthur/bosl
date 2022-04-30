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

#include <stdbool.h>

#if defined( _COMPILING_BOSL )
  #include "collection/list.h"
  #include "scanner.h"
#else
  #include <bosl/collection/list.h>
  #include <bosl/scanner.h>
#endif

#if !defined( BOSL_PARSER_H )
#define BOSL_PARSER_H

#ifdef __cplusplus
extern "C" {
#endif

typedef bosl_token_t* ( * parser_previous_t )( void );
typedef bosl_token_t* ( * parser_current_t )( void );
typedef bosl_token_t* ( * parser_next_t )( void );

typedef struct {
  parser_previous_t previous;
  parser_current_t current;
  parser_next_t next;

  list_manager_t* ast;
  list_manager_t* token;
  list_item_t* current_item;

  bool in_function;
  bool in_loop;
  size_t depth;
} bosl_parser_t;

bool bosl_parser_init( list_manager_t* );
void bosl_parser_free( void );
list_manager_t* bosl_parser_scan( void );
void bosl_parser_print( void );

#ifdef __cplusplus
}
#endif

#endif
