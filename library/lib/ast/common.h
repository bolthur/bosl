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
  #include "statement.h"
  #include "expression.h"
#else
  #include <bosl/ast/statement.h>
  #include <bosl/ast/expression.h>
#endif

#if !defined( BOSL_AST_COMMON_H )
#define BOSL_AST_COMMON_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
  bosl_ast_statement_t* statement;
} bosl_ast_node_t;

bosl_ast_node_t* bosl_ast_node_allocate( void );
void bosl_ast_node_destroy( bosl_ast_node_t* );

#ifdef __cplusplus
}
#endif

#endif
