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
#include "../collection/list.h"

#if ! defined( _PARSER_H )
#define _PARSER_H

typedef struct bosl_parser {
  list_manager_t* token;
  list_item_t* current;
  uint8_t* byte_code;
} bosl_parser_t;

bool parser_init( list_manager_t* );
void parser_free( void );
uint8_t* parser_compile( void );

#endif
