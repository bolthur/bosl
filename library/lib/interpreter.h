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
  #include "ast/statement.h"
  typedef struct bosl_environment bosl_environment_t;
#else
  #include <bosl/collection/list.h>
  #include <bosl/ast/statement.h>
  typedef struct bosl_environment bosl_environment_t;
#endif

#if !defined( BOSL_INTERPRETER_H )
#define BOSL_INTERPRETER_H

#ifdef __cplusplus
extern "C" {
#endif

typedef bosl_ast_statement_t* ( *interpreter_previous_t )( void );
typedef bosl_ast_statement_t* ( *interpreter_current_t )( void );
typedef bosl_ast_statement_t* ( *interpreter_next_t )( void );

typedef struct bosl_interpreter {
  interpreter_previous_t previous;
  interpreter_current_t current;
  interpreter_next_t next;

  bool error;
  bosl_environment_t* env;

  int64_t loop_break_remaining;
  int64_t loop_continue_remaining;
  int64_t loop_level;

  list_manager_t* ast;
  list_item_t* current_item;
} bosl_interpreter_t;

bool bosl_interpreter_init( list_manager_t* );
void bosl_interpreter_free( void );
bool bosl_interpreter_run( void );
void bosl_interpreter_emit_error( bosl_token_t*, const char* );

#ifdef __cplusplus
}
#endif

#endif
