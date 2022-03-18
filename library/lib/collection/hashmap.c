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
 * @brief Construct new hashmap
 *
 * @return Allocated new hashmap or NULL on error
 */
hashmap_table_t* hashmap_construct( void ) {
  // allocate hashmap table
  hashmap_table_t* table = malloc( sizeof( *table  ) );
  if ( ! table ) {
    return NULL;
  }
  // populate properties
  table->capacity = HASHMAP_INITIAL_CAPACITY;
  table->length = 0;
  // allocate hashmap entries
  table->entries = calloc( table->capacity, sizeof( *table->entries ) );
  if ( ! table->entries ) {
    free( table );
    return NULL;
  }
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
  }
  // free entries and finally table
  free( table->entries );
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
  size_t hash = hashmap_generate_hash( key );
  size_t index = hash & ( table->capacity - 1 );

  // loop until non empty
  while ( table->entries[ index ].key ) {
    // check if found and return
    if ( ! strcmp( key, table->entries[ index ].key ) )  {
      return table->entries[ index ].value;
    }
    // increment if key wasn't in slot and check for end reached
    if ( ++index >= table->capacity ) {
      index = 0;
    }
  }
  // return NULL as nothing was found
  return NULL;
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
  if ( table->length >= table->capacity ) {
    // FIXME: EXPAND
    return NULL;
  }

  // generate hash and index
  size_t hash = hashmap_generate_hash( key );
  size_t index = hash & ( table->capacity - 1 );
  // loop until empty entry was found
  while ( table->entries[ index ].key ) {
    // update if already exists
    if ( ! strcmp( key, table->entries[ index ].key ) ) {
      table->entries[ index ].value = value;
      return table->entries[ index ].key;
    }
    // increment if key wasn't in slot and check for end reached
    if ( ++index >= table->capacity ) {
      index = 0;
    }
  }

  // get key length
  size_t key_len = strlen( key ) + 1;
  // allocate entry
  char* new_key = malloc( sizeof( char ) * key_len );
  if ( ! new_key ) {
    return NULL;
  }
  // clear out stuff and copy
  memset( new_key, 0, sizeof( char ) * key_len );
  strcpy( new_key, key );
  // push back data
  table->entries[ index ].key = new_key;
  table->entries[ index ].value = value;
  // increase length
  table->length++;
  // return new key
  return table->entries[ index ].key;
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
