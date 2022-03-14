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

#define HASHMAP_INITIAL_CAPACITY 64

#if ! defined( _HASHMAP_H )
#define _HASHMAP_H

typedef struct {
  const char* key;
  void* value;
} hashmap_entry_t;

typedef struct {
  hashmap_entry_t* entries;
  size_t capacity;
  size_t length;
} hashmap_table_t;

typedef struct {
  const char* key;
  void* value;

  // some sort of private stuff
  hashmap_table_t* _table;
  size_t _index;
} hashmap_iterator_t;

hashmap_table_t* hashmap_construct( void );
void hashmap_destruct( hashmap_table_t* );
void* hashmap_value_get( hashmap_table_t*, const char* );
const char* hashmap_value_set( hashmap_table_t*, const char*, void* );
size_t hashmap_length( hashmap_table_t* );
hashmap_iterator_t hashmap_iterator( hashmap_table_t* );
bool hashmap_next( hashmap_iterator_t* );

#endif