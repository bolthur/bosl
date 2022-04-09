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
#include <stdint.h>
#include <string.h>
#include "hashmap.h"

static void default_cleanup( __unused void* a ) {}

/**
 * @brief Helper to generate a hash of key using Jenkin's one_at_a_time
 *
 * @param key
 * @return
 */
static size_t hashmap_generate_hash( const char* key ) {
  // determine length of null terminated key
  size_t len = strlen( key );
  // transform to const uint8 poitner
  const uint8_t* byte = ( const uint8_t* )key;
  // hash variable
  size_t hash = 0;

  // build hash
  for ( size_t idx = 0; idx < len; idx++ ) {
    hash += *byte++;
    hash += ( hash << 10 );
    hash ^= ( hash >> 6 );
  }
  hash += ( hash << 3 );
  hash ^= ( hash >> 11 );
  hash += ( hash << 15 );

  // return built hash
  return hash;
}

/**
 * @brief Helper to find an entry
 *
 * @param entries
 * @param key
 * @param capacity
 * @return
 */
static hashmap_entry_t* hashmap_find_entry(
  hashmap_entry_t* entries,
  const char* key,
  size_t capacity
) {
  // generate hash
  size_t hash = hashmap_generate_hash( key );
  size_t index = hash & ( capacity - 1 );
  // loop until non empty
  while ( entries[ index ].key ) {
    // check if found and return
    if ( ! strcmp( key, entries[ index ].key ) )  {
      return &entries[ index ];
    }
    // increment if key wasn't in slot and check for end reached
    if ( ++index >= capacity ) {
      index = 0;
    }
  }
  // return by index
  return &entries[ index ];
}

/**
 * @brief Helper to duplicate key by length
 *
 * @param key
 * @param len
 * @return
 */
static char* duplicate_key( const char* key, size_t len ) {
  // get key length
  size_t key_len = len + 1;
  // allocate temporary
  char* tmp = malloc( sizeof( char ) * key_len );
  if ( ! tmp ) {
    return NULL;
  }
  // clear out
  memset( tmp, 0, sizeof( char ) * key_len );
  // copy content
  strncpy( tmp, key, len );
  // return key
  return tmp;
}

/**
 * @brief Helper to adjust hashmap size
 *
 * @param table
 * @param capacity
 * @return
 */
static bool adjust_capacity( hashmap_table_t* table, size_t capacity ) {
  // allocate new hashmap entries
  hashmap_entry_t* new_list = calloc( capacity, sizeof( hashmap_entry_t ) );
  // handle error
  if ( ! new_list ) {
    return false;
  }
  // clear out
  memset( new_list, 0, sizeof( hashmap_entry_t ) * capacity );
  // resize table length
  table->length = 0;
  for ( size_t i = 0; i < table->capacity; i++ ) {
    // skip unused entries
    if ( ! table->entries[ i ].key ) {
      continue;
    }
    // find entry
    hashmap_entry_t* new_entry = hashmap_find_entry(
      new_list, table->entries[ i ].key, capacity );
    // copy over
    new_entry->key = table->entries[ i ].key;
    new_entry->value = table->entries[ i ].value;
    // increment length
    table->length++;
  }
  // free up old table
  free( table->entries );
  // set entries and new capacity
  table->entries = new_list;
  table->capacity = capacity;
  // return success
  return true;
}

/**
 * @brief Construct new hashmap
 *
 * @return Allocated new hashmap or NULL on error
 */
hashmap_table_t* hashmap_construct( hashmap_entry_cleanup_t cleanup ) {
  // allocate hashmap table
  hashmap_table_t* table = malloc( sizeof( hashmap_table_t ) );
  if ( ! table ) {
    return NULL;
  }
  // clear out
  memset( table, 0, sizeof( hashmap_table_t ) );
  // populate properties
  table->capacity = 0;
  table->length = 0;
  table->entries = NULL;
  table->cleanup = cleanup ? cleanup : default_cleanup;
  // return built table
  return table;
}

/**
 * @brief Destruct hashmap
 *
 * @param table table to destruct
 */
void hashmap_destruct( hashmap_table_t* table ) {
  // loop through table items and free up keys
  for ( size_t idx = 0; idx < table->capacity; idx++ ) {
    // free key
    if ( table->entries[ idx ].key ) {
      free( ( void* )table->entries[ idx ].key );
    }
    // free value
    if ( table->entries[ idx ].value ) {
      table->cleanup( table->entries[ idx ].value );
    }
  }
  // free table if set
  if ( table->entries ) {
    free( table->entries );
  }
  // free structure
  free( table );
}

/**
 * @brief Method to get hashmap entry by key
 *
 * @param table
 * @param key
 * @return Set value or NULL if not found
 */
void* hashmap_value_get( hashmap_table_t* table, const char* key ) {
  // handle no entries yet
  if ( ! table->entries ) {
    return NULL;
  }
  // get matching entry from map
  hashmap_entry_t* e = hashmap_find_entry( table->entries, key, table->capacity );
  // return stored value ( NULL if not set previously )
  return e->value;
}

/**
 * @brief Method to get hashmap entry by key with given size
 *
 * @param table
 * @param key
 * @param len
 * @return Set value or NULL if not found
 */
void* hashmap_value_nget( hashmap_table_t* table, const char* key, size_t len ) {
  // handle no entries yet
  if ( ! table->entries ) {
    return NULL;
  }
  // duplicate key
  char* tmp = duplicate_key( key, len );
  if ( ! tmp ) {
    return NULL;
  }
  // get value using temporary key
  void* data = hashmap_value_get( table, tmp );
  // free up temporary
  free( tmp );
  // return found data
  return data;
}

/**
 * @brief Set a value in hashmap
 *
 * @param table
 * @param key
 * @param value
 * @return
 */
const char* hashmap_value_set(
  hashmap_table_t* table,
  const char* key,
  void* value
) {
  // expand table if limit reached
  if (
    table->length + 1 >= table->capacity
    && ! adjust_capacity( table, HASHMAP_ENLARGE_CAPACITY( table->capacity ) )
  ) {
    return NULL;
  }
  // get matching entry from map
  hashmap_entry_t* e = hashmap_find_entry( table->entries, key, table->capacity );
  // handle no new key
  if ( e->key ) {
    table->cleanup( e->value );
    e->value = value;
  // handle new key
  } else {
    // duplicate key
    char* new_key = duplicate_key( key, strlen( key ) );
    if ( ! new_key ) {
      return NULL;
    }
    // push back data
    e->key = new_key;
    e->value = value;
    // increase length
    table->length++;
  }
  // return stored key ( NULL if not set previously )
  return e->key;
}

/**
 * @brief Set a value in hashmap
 *
 * @param table
 * @param key
 * @param len
 * @param value
 * @return
 */
const char* hashmap_value_nset(
  hashmap_table_t* table,
  const char* key,
  size_t len,
  void* value
) {
  // duplicate key
  char* tmp = duplicate_key( key, len );
  if ( ! tmp ) {
    return NULL;
  }
  // add via normal set
  const char* return_key = hashmap_value_set( table, tmp, value );
  // free temporary ( duplicated in hashmap value set )
  free( tmp );
  // return set key
  return return_key;
}

/**
 * @brief Delete a hashmap entry by key
 *
 * @param table
 * @param key
 * @return
 */
bool hashmap_value_del( hashmap_table_t* table, const char* key ) {
  // find entry
  hashmap_entry_t* e = hashmap_find_entry( table->entries, key, table->capacity );
  // treat no key ( no entry ) as failure
  if ( ! e->key ) {
    return false;
  }
  // cleanup value
  table->cleanup( e->value );
  // free key
  free( e->key );
  // unset key and value to NULL again
  e->key = NULL;
  e->value = NULL;
  // return success
  return true;
}

/**
 * @brief Delete a value by key
 *
 * @param table
 * @param key
 * @param len
 */
bool hashmap_value_ndel( hashmap_table_t* table, const char* key, size_t len ) {
  // duplicate key
  char* tmp = duplicate_key( key, len );
  if ( ! tmp ) {
    return false;
  }
  // delete value
  bool r = hashmap_value_del( table, tmp );
  // free tmp again
  free( tmp );
  // return result
  return r;
}

/**
 * @brief Method to get hashmap length
 *
 * @param table
 * @return length of the hashmap
 */
size_t hashmap_length( hashmap_table_t* table ) {
  return table->length;
}

/**
 * @brief Create iterator for hashmap
 *
 * @param table
 * @return hashmap iterator
 */
hashmap_iterator_t hashmap_iterator( hashmap_table_t* table ) {
  // local variable
  hashmap_iterator_t iterator = {
    ._table = table,
    ._index = 0,
  };
  // return it
  return iterator;
}

/**
 * @brief Get next entry via iterator
 *
 * @param iterator
 * @return
 */
bool hashmap_next( hashmap_iterator_t* iterator ) {
  // loop while there is still something left
  while ( iterator->_index < iterator->_table->capacity ) {
    // save current index
    size_t index = iterator->_index;
    // increment to next one
    iterator->_index++;
    // handle something in current index
    if ( iterator->_table->entries[ index ].key ) {
      // save key and value in iterator
      iterator->key = iterator->_table->entries[ index ].key;
      iterator->value = iterator->_table->entries[ index ].value;
      // return success
      return true;
    }
  }
  // return end
  return false;
}
