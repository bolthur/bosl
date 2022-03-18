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
  EXPRESSION_GROUPING,
  EXPRESSION_LITERAL,
  EXPRESSION_LOGICAL,
  EXPRESSION_UNARY,
  EXPRESSION_VARIABLE,
} bosl_ast_expression_type_t;

typedef struct {
  bosl_ast_expression_type_t type;
  void* data;
} bosl_ast_expression_t;

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

void* ast_expression_allocate( bosl_ast_expression_type_t );
bool ast_expression_push_literal( bosl_ast_expression_t*, const void*, size_t );

#endif
