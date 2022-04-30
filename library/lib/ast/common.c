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

#include <stdlib.h>
#include <string.h>
#include "common.h"
#include "expression.h"
#include "statement.h"

/**
 * @brief Helper to allocate a ast node
 *
 * @return
 */
bosl_ast_node_t* bosl_ast_node_allocate( void ) {
  // allocate new ast node
  bosl_ast_node_t* node = malloc( sizeof( bosl_ast_node_t ) );
  if ( !node ) {
    return NULL;
  }
  // clear out
  memset( node, 0, sizeof( bosl_ast_node_t ) );
  // return data
  return node;
}

/**
 * @brief Helper to destroy a node
 *
 * @param node
 */
void bosl_ast_node_destroy( bosl_ast_node_t* node ) {
  if ( !node ) {
    return;
  }
  bosl_ast_statement_destroy( node->statement );
  free( node );
}
