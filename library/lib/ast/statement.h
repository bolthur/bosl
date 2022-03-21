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
  #include "expression.h"
#else
  typedef struct list_manager list_manager_t;
  #include <bosl/scanner.h>
  #include <bosl/ast/expression.h>
#endif
#if ! defined( _BOSL_AST_STATEMENT_H )
#define _BOSL_AST_STATEMENT_H

typedef enum {
  STATEMENT_BLOCK,
  STATEMENT_EXPRESSION,
  STATEMENT_PARAMETER,
  STATEMENT_FUNCTION,
  STATEMENT_IF,
  STATEMENT_PRINT,
  STATEMENT_RETURN,
  STATEMENT_VARIABLE,
  STATEMENT_CONST,
  STATEMENT_WHILE,
} bosl_ast_statement_type_t;

typedef struct bosl_ast_statement bosl_ast_statement_t;

typedef struct {
  list_manager_t* statements;
} bosl_ast_statement_block_t;

typedef struct {
  bosl_ast_expression_t* expression;
} bosl_ast_statement_expression_t;

typedef struct {
  bosl_token_t* name;
  bosl_token_t* type;
} bosl_ast_statement_parameter_t;

typedef struct {
  bosl_token_t* token;
  list_manager_t* parameter; // list of ast statement parameter
  bosl_token_t* return_type;
  bosl_ast_statement_t* body; // list of statements
  bosl_token_t* load_identifier;
} bosl_ast_statement_function_t;

typedef struct {
  bosl_ast_expression_t* if_condition;
  bosl_ast_statement_t* if_statement;
  bosl_ast_statement_t* else_statement;
} bosl_ast_statement_if_t;

typedef struct {
  bosl_ast_expression_t* expression;
} bosl_ast_statement_print_t;

typedef struct {
  bosl_token_t* keyword;
  bosl_ast_expression_t* value;
} bosl_ast_statement_return_t;

typedef struct {
  bosl_token_t* name;
  bosl_token_t* type;
  bosl_ast_expression_t* initializer;
} bosl_ast_statement_variable_t;

typedef struct {
  bosl_token_t* name;
  bosl_token_t* type;
  bosl_ast_expression_t* initializer;
} bosl_ast_statement_const_t;

typedef struct {
  bosl_ast_expression_t* condition;
  bosl_ast_statement_t* body;
} bosl_ast_statement_while_t;

typedef struct bosl_ast_statement {
  bosl_ast_statement_type_t type;
  union {
    bosl_ast_statement_block_t* block;
    bosl_ast_statement_expression_t* expression;
    bosl_ast_statement_parameter_t* parameter;
    bosl_ast_statement_function_t* function;
    bosl_ast_statement_if_t* if_;
    bosl_ast_statement_print_t* print;
    bosl_ast_statement_return_t* return_;
    bosl_ast_statement_variable_t* variable;
    bosl_ast_statement_const_t* const_;
    bosl_ast_statement_while_t* while_;
    void* data;
  };
  size_t size;
} bosl_ast_statement_t;

bosl_ast_statement_t* bosl_ast_statement_allocate( bosl_ast_statement_type_t );
void bosl_ast_statement_destroy( bosl_ast_statement_t* );

#endif
