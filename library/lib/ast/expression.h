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

#if defined( _COMPILING_BOSL )
  #include "../scanner.h"
  #include "../collection/list.h"
#else
  typedef struct list_manager list_manager_t;
  #include <bosl/scanner.h>
#endif
#if ! defined( _BOSL_AST_EXPRESSION_H )
#define _BOSL_AST_EXPRESSION_H

typedef enum {
  EXPRESSION_ASSIGN,
  EXPRESSION_BINARY,
  EXPRESSION_CALL,
  EXPRESSION_LOAD,
  EXPRESSION_POINTER,
  EXPRESSION_GROUPING,
  EXPRESSION_LITERAL,
  EXPRESSION_LOGICAL,
  EXPRESSION_UNARY,
  EXPRESSION_VARIABLE,
} bosl_ast_expression_type_t;

typedef enum {
  EXPRESSION_LITERAL_TYPE_NULL,
  EXPRESSION_LITERAL_TYPE_NUMBER_INT,
  EXPRESSION_LITERAL_TYPE_NUMBER_HEX,
  EXPRESSION_LITERAL_TYPE_NUMBER_FLOAT,
  EXPRESSION_LITERAL_TYPE_STRING,
  EXPRESSION_LITERAL_TYPE_BOOL,
} bosl_ast_expression_literal_type_t;

typedef struct bosl_ast_expression bosl_ast_expression_t;

typedef struct {
  bosl_token_t* token;
  bosl_ast_expression_t* value;
} bosl_ast_expression_assign_t;

typedef struct {
  bosl_ast_expression_t* left;
  bosl_token_t* operator;
  bosl_ast_expression_t* right;
} bosl_ast_expression_binary_t;

typedef struct {
  bosl_ast_expression_t* callee;
  bosl_token_t* paren;
  list_manager_t* arguments; // list of bosl_ast_expression_t
} bosl_ast_expression_call_t;

typedef struct {
  bosl_ast_expression_t* expression;
} bosl_ast_expression_grouping_t;

typedef struct {
  void* value;
  size_t size;
  bosl_ast_expression_literal_type_t type;
} bosl_ast_expression_literal_t;

typedef struct {
  bosl_ast_expression_t* left;
  bosl_token_t* operator;
  bosl_ast_expression_t* right;
} bosl_ast_expression_logical_t;

typedef struct {
  bosl_token_t* operator;
  bosl_ast_expression_t* right;
} bosl_ast_expression_unary_t;

typedef struct {
  bosl_token_t* name;
} bosl_ast_expression_variable_t;

typedef struct {
  bosl_token_t* name;
} bosl_ast_expression_load_t;

typedef struct {
  bosl_token_t* name;
} bosl_ast_expression_pointer_t;

typedef struct bosl_ast_expression {
  bosl_ast_expression_type_t type;
  union {
    void* data;
    bosl_ast_expression_assign_t* assign;
    bosl_ast_expression_binary_t* binary;
    bosl_ast_expression_call_t* call;
    bosl_ast_expression_grouping_t* grouping;
    bosl_ast_expression_literal_t* literal;
    bosl_ast_expression_logical_t* logical;
    bosl_ast_expression_unary_t* unary;
    bosl_ast_expression_variable_t* variable;
    bosl_ast_expression_load_t* load;
    bosl_ast_expression_pointer_t* pointer;
  };
  size_t size;
} bosl_ast_expression_t;

bosl_ast_expression_t* bosl_ast_expression_allocate( bosl_ast_expression_type_t );
void bosl_ast_expression_destroy( bosl_ast_expression_t* );
bosl_ast_expression_t* bosl_ast_expression_allocate_binary(
  bosl_ast_expression_t*, bosl_token_t*, bosl_ast_expression_t* );
bosl_ast_expression_t* bosl_ast_expression_allocate_logical(
  bosl_ast_expression_t*, bosl_token_t*, bosl_ast_expression_t* );
bosl_ast_expression_t* bosl_ast_expression_allocate_literal(
  const void*, size_t, bosl_ast_expression_literal_type_t );

#endif
