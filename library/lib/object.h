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

#if !defined( BOSL_OBJECT_H )
#define BOSL_OBJECT_H

#ifdef __cplusplus
extern "C" {
#endif

// interpreter structure forward declaration
typedef struct bosl_interpreter bosl_interpreter_t;

typedef enum bosl_object_type {
  BOSL_OBJECT_TYPE_UNDEFINED,
  BOSL_OBJECT_TYPE_BOOL,
  BOSL_OBJECT_TYPE_UINT_8,
  BOSL_OBJECT_TYPE_UINT_16,
  BOSL_OBJECT_TYPE_UINT_32,
  BOSL_OBJECT_TYPE_UINT_64,
  BOSL_OBJECT_TYPE_INT_8,
  BOSL_OBJECT_TYPE_INT_16,
  BOSL_OBJECT_TYPE_INT_32,
  BOSL_OBJECT_TYPE_INT_64,
  BOSL_OBJECT_TYPE_STRING,
  BOSL_OBJECT_TYPE_FLOAT,
} bosl_object_type_t;

typedef enum bosl_object_value_type {
  BOSL_OBJECT_VALUE_FLOAT,
  BOSL_OBJECT_VALUE_INT_SIGNED,
  BOSL_OBJECT_VALUE_INT_UNSIGNED,
  BOSL_OBJECT_VALUE_BOOL,
  BOSL_OBJECT_VALUE_STRING,
  BOSL_OBJECT_VALUE_NULL,
  BOSL_OBJECT_VALUE_CALLABLE,
} bosl_object_value_type_t;

typedef struct bosl_object {
  bosl_object_value_type_t value_type;
  bosl_object_type_t type;
  void* data;
  size_t size;
  bool environment;
  bool constant;
  bool is_return;
  bool is_break;
  bool is_continue;
} bosl_object_t;

// type definition for function callback
typedef bosl_object_t* ( *bosl_callback_t )( bosl_object_t*, list_manager_t* );

typedef struct bosl_object_callable {
  bosl_callback_t callback;
  bosl_ast_statement_function_t* statement;
  bosl_environment_t* closure;
} bosl_object_callable_t;

bool bosl_object_init( void );
void bosl_object_free( void );
void bosl_object_destroy( bosl_object_t* );
bosl_object_t* bosl_object_allocate(
  bosl_object_value_type_t, bosl_object_type_t, void*, size_t );
bosl_object_t* bosl_object_allocate_callable(
  bosl_ast_statement_function_t*,
  bosl_callback_t,
  bosl_environment_t*
);
bosl_object_t* bosl_object_duplicate_environment( bosl_object_t* );
bool bosl_object_assign_push_value(
  bosl_environment_t*, bosl_token_t*, bosl_token_t*, bosl_object_t*, bool );

bool bosl_object_extract_number(
  bosl_object_t*, uint64_t*, int64_t*, long double* );
bosl_object_type_t bosl_object_str_to_type( const char*, size_t );
const char* bosl_object_type_to_str( bosl_object_type_t );
char* bosl_object_stringify( bosl_object_t* );
void* bosl_object_extract_parameter( list_manager_t*, size_t );
bool bosl_object_validate( bosl_token_t*, bosl_object_type_t, bosl_object_t* );

#ifdef __cplusplus
}
#endif

#endif
