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

#define HASHMAP_ENLARGE_CAPACITY(c) ( ( c ) < 8 ? 8 : ( c ) * 2 )

#if ! defined( _HASHMAP_H )
#define _HASHMAP_H

// forward declaration
typedef struct hashmap_entry hashmap_entry_t;
typedef struct hashmap_table hashmap_table_t;
typedef struct hashmap_iterator hashmap_iterator_t;

typedef void ( *hashmap_entry_cleanup_t )( void* a );

struct hashmap_entry {
  const char* key;
  void* value;
};

struct hashmap_table {
  hashmap_entry_t* entries;
  size_t capacity;
  size_t length;
  hashmap_entry_cleanup_t cleanup;
};

struct hashmap_iterator {
  const char* key;
  void* value;

  // some sort of private stuff
  hashmap_table_t* _table;
  size_t _index;
};

hashmap_table_t* hashmap_construct( hashmap_entry_cleanup_t );
void hashmap_destruct( hashmap_table_t* );
void* hashmap_value_get( hashmap_table_t*, const char* );
void* hashmap_value_nget( hashmap_table_t*, const char*, size_t );
const char* hashmap_value_set( hashmap_table_t*, const char*, void* );
const char* hashmap_value_nset( hashmap_table_t*, const char*, size_t, void* );
bool hashmap_value_del( hashmap_table_t*, const char* );
bool hashmap_value_ndel( hashmap_table_t*, const char*, size_t );
size_t hashmap_length( hashmap_table_t* );
hashmap_iterator_t hashmap_iterator( hashmap_table_t* );
bool hashmap_next( hashmap_iterator_t* );

#endif
