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
#else
  #include <bosl/collection/list.h>
  #include <bosl/ast/statement.h>
#endif

#if ! defined( _BOSL_OBJECT_H )
#define _BOSL_OBJECT_H

// interpreter structure forward declaration
typedef struct bosl_interpreter bosl_interpreter_t;

typedef enum bosl_object_type {
  OBJECT_TYPE_UNDEFINED,
  OBJECT_TYPE_UINT8,
  OBJECT_TYPE_UINT16,
  OBJECT_TYPE_UINT32,
  OBJECT_TYPE_UINT64,
  OBJECT_TYPE_INT8,
  OBJECT_TYPE_INT16,
  OBJECT_TYPE_INT32,
  OBJECT_TYPE_INT64,
  OBJECT_TYPE_STRING,
  OBJECT_TYPE_FLOAT,
} bosl_object_type_t;

typedef enum bosl_object_value_type {
  OBJECT_VALUE_FLOAT,
  OBJECT_VALUE_INT_SIGNED,
  OBJECT_VALUE_INT_UNSIGNED,
  OBJECT_VALUE_HEX_SIGNED,
  OBJECT_VALUE_HEX_UNSIGNED,
  OBJECT_VALUE_BOOL,
  OBJECT_VALUE_STRING,
  OBJECT_VALUE_NULL,
  OBJECT_VALUE_CALLABLE,
} bosl_object_value_type_t;

typedef struct bosl_object {
  bosl_object_value_type_t value_type;
  bosl_object_type_t type;
  void* data;
  size_t size;
  bool environment;
  bool constant;
  bool is_return;
} bosl_object_t;

// type definition for function callback
typedef bosl_object_t* ( *bosl_callback_t )( bosl_object_t*, list_manager_t* );

typedef struct bosl_object_callable {
  bosl_callback_t callback;
  bosl_ast_statement_function_t* statement;
} bosl_object_callable_t;

bool bosl_object_init( void );
void bosl_object_free( void );
void bosl_object_destroy( bosl_object_t* );
bosl_object_t* bosl_object_allocate( bosl_object_value_type_t, void*, size_t );
bosl_object_t* bosl_object_allocate_callable(
  bosl_ast_statement_function_t*,
  bosl_callback_t
);
bosl_object_t* bosl_object_duplicate_environment( bosl_object_t* );

bosl_object_type_t bosl_object_str_to_type( const char*, size_t );
int64_t bosl_object_type_min_int_value( bosl_object_type_t );
uint64_t bosl_object_type_max_int_value( bosl_object_type_t );
long double bosl_object_type_min_float_value( bosl_object_type_t );
long double bosl_object_type_max_float_value( bosl_object_type_t );

#endif
