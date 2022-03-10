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
#include <stdio.h>
#include <check.h>
#include <unistd.h>
#include <time.h>
#include "../../library/collection/list.h"

typedef struct {
  size_t data;
  int* foo;
} foobar_t;

static foobar_t* create_foobar( size_t data ) {
  // create entry
  foobar_t* c = malloc( sizeof( foobar_t ) );
  if ( ! c ) {
    return NULL;
  }
  // clear out
  memset( c, 0, sizeof( foobar_t ) );
  // allocate dynamic
  c->foo = malloc( sizeof( int ) );
  if ( ! c->foo ) {
    free( c );
    return NULL;
  }
  // set random and return
  *c->foo = rand();
  c->data = data;
  return c;
}

static int32_t list_custom_lookup( const list_item_t* item, const void* data ) {
  const foobar_t* item_data = item->data;
  return item_data->data == ( size_t )data ? 0 : 1;
}

static void list_custom_cleanup( list_item_t* item ) {
  foobar_t* item_data = item->data;
  if ( item_data->foo ) {
    free( item_data->foo );
  }
  free( item_data );
  list_default_cleanup( item );
}

static bool list_custom_insert( list_manager_t* list, void* data ) {
  return list_push_front_data( list, data );
}

list_manager_t* list;

static void setup( void ) {
  list = list_construct( NULL, NULL, NULL );
  ck_assert_ptr_nonnull( list );
}

static void teardown( void ) {
  list_destruct( list );
  list = NULL;
}

START_TEST( test_construct ) {
  list_manager_t* l = list_construct( NULL, NULL, NULL );
  ck_assert_ptr_nonnull( l );
  // ensure that list is empty
  ck_assert_ptr_null( l->first );
  ck_assert_ptr_null( l->last );
  // destroy list again
  list_destruct( l );
}
END_TEST

START_TEST( test_empty ) {
  ck_assert( list_empty( list ) );
}
END_TEST

START_TEST( test_item_create ) {
  void* data = ( void* )5;
  list_item_t* item = list_item_create( data );
  ck_assert_ptr_nonnull( item );
  ck_assert_ptr_null( item->next );
  ck_assert_ptr_null( item->previous );
  ck_assert_ptr_eq( item->data, data );
  free( item );
}
END_TEST

START_TEST( test_push_front_data ) {
  // ensure list is empty
  ck_assert( list_empty( list ) );
  // test data
  void* data[] = { ( void* )5, ( void* )10, ( void* )15, };
  // item count
  int item_count = 0;
  // pushback data three times
  ck_assert( list_push_front_data( list, data[ 2 ] ) );
  ck_assert( list_push_front_data( list, data[ 1 ] ) );
  ck_assert( list_push_front_data( list, data[ 0 ] ) );
  // ensure list is not empty and different
  ck_assert( ! list_empty( list ) );
  ck_assert_ptr_ne( list->first, list->last );
  // loop through data
  list_item_t* current = list->first;
  while ( current ) {
    // test data
    ck_assert_ptr_eq( current->data, data[ item_count ] );
    // increase item_count and set current to next
    item_count++;
    current = current->next;
  }
  // assert item count to be 3
  ck_assert_int_eq( item_count, 3 );
}
END_TEST

START_TEST( test_push_back_data ) {
  // ensure list is empty
  ck_assert( list_empty( list ) );
  // test data
  void* data[] = { ( void* )5, ( void* )10, ( void* )15, };
  // item count
  int item_count = 0;
  // pushback data three times
  ck_assert( list_push_back_data( list, data[ 0 ] ) );
  ck_assert( list_push_back_data( list, data[ 1 ] ) );
  ck_assert( list_push_back_data( list, data[ 2 ] ) );
  // ensure list is not empty and different
  ck_assert( ! list_empty( list ) );
  ck_assert_ptr_ne( list->first, list->last );
  // loop through data
  list_item_t* current = list->first;
  while ( current ) {
    // test data
    ck_assert_ptr_eq( current->data, data[ item_count ] );
    // increase item_count and set current to next
    item_count++;
    current = current->next;
  }
  // assert item count to be 3
  ck_assert_int_eq( item_count, 3 );
}
END_TEST

START_TEST( test_remove_item_first ) {
  ck_assert( list_empty( list ) );
  void* data1 = ( void* )5;
  void* data2 = ( void* )10;
  void* data3 = ( void* )15;

  // insert items
  ck_assert( list_insert_data( list, data1 ) );
  ck_assert( list_insert_data( list, data2 ) );
  ck_assert( list_insert_data( list, data3 ) );
  // assert first and last pointer
  ck_assert_ptr_eq( list->first->data, data1 );
  ck_assert_ptr_eq( list->last->data, data3 );

  // remove middle item
  ck_assert( list_remove_item( list, list->first ) );
  // assert first and last pointer
  ck_assert_ptr_eq( list->first->data, data2 );
  ck_assert_ptr_eq( list->last->data, data3 );
  // assert connection between item 1 and 3
  ck_assert_ptr_null( list->first->previous );
  ck_assert_ptr_eq( list->first->next, list->last );
  ck_assert_ptr_null( list->last->next );
  ck_assert_ptr_eq( list->last->previous, list->first );
}
END_TEST

START_TEST( test_remove_item_middle ) {
  ck_assert( list_empty( list ) );
  void* data1 = ( void* )5;
  void* data2 = ( void* )10;
  void* data3 = ( void* )15;

  // insert items
  ck_assert( list_insert_data( list, data1 ) );
  ck_assert( list_insert_data( list, data2 ) );
  ck_assert( list_insert_data( list, data3 ) );
  // assert first and last pointer
  ck_assert_ptr_eq( list->first->data, data1 );
  ck_assert_ptr_eq( list->last->data, data3 );

  // remove middle item
  ck_assert( list_remove_item( list, list->first->next ) );
  // assert first and last pointer
  ck_assert_ptr_eq( list->first->data, data1 );
  ck_assert_ptr_eq( list->last->data, data3 );
  // assert connection between item 1 and 3
  ck_assert_ptr_null( list->first->previous );
  ck_assert_ptr_eq( list->first->next, list->last );
  ck_assert_ptr_null( list->last->next );
  ck_assert_ptr_eq( list->last->previous, list->first );
}
END_TEST

START_TEST( test_remove_item_last ) {
  ck_assert( list_empty( list ) );
  void* data1 = ( void* )5;
  void* data2 = ( void* )10;
  void* data3 = ( void* )15;

  // insert items
  ck_assert( list_insert_data( list, data1 ) );
  ck_assert( list_insert_data( list, data2 ) );
  ck_assert( list_insert_data( list, data3 ) );
  // assert first and last pointer
  ck_assert_ptr_eq( list->first->data, data1 );
  ck_assert_ptr_eq( list->last->data, data3 );

  // remove middle item
  ck_assert( list_remove_item( list, list->last ) );
  // assert first and last pointer
  ck_assert_ptr_eq( list->first->data, data1 );
  ck_assert_ptr_eq( list->last->data, data2 );
  // assert connection between item 1 and 3
  ck_assert_ptr_null( list->first->previous );
  ck_assert_ptr_eq( list->first->next, list->last );
  ck_assert_ptr_null( list->last->next );
  ck_assert_ptr_eq( list->last->previous, list->first );
}
END_TEST

START_TEST( test_remove_data_first ) {
  ck_assert( list_empty( list ) );
  void* data1 = ( void* )5;
  void* data2 = ( void* )10;
  void* data3 = ( void* )15;

  // insert items
  ck_assert( list_push_back_data( list, data1 ) );
  ck_assert( list_push_back_data( list, data2 ) );
  ck_assert( list_push_back_data( list, data3 ) );
  // assert first and last pointer
  ck_assert_ptr_eq( list->first->data, data1 );
  ck_assert_ptr_eq( list->last->data, data3 );

  // remove last item
  ck_assert( list_remove_data( list, data1 ) );
  // assert first and last pointer
  ck_assert_ptr_eq( list->first->data, data2 );
  ck_assert_ptr_eq( list->last->data, data3 );
  // assert connection between remaining items
  ck_assert_ptr_null( list->first->previous );
  ck_assert_ptr_eq( list->first->next, list->last );
  ck_assert_ptr_null( list->last->next );
  ck_assert_ptr_eq( list->last->previous, list->first );
}
END_TEST

START_TEST( test_remove_data_middle ) {
  ck_assert( list_empty( list ) );
  void* data1 = ( void* )5;
  void* data2 = ( void* )10;
  void* data3 = ( void* )15;

  // insert items
  ck_assert( list_push_back_data( list, data1 ) );
  ck_assert( list_push_back_data( list, data2 ) );
  ck_assert( list_push_back_data( list, data3 ) );
  // assert first and last pointer
  ck_assert_ptr_eq( list->first->data, data1 );
  ck_assert_ptr_eq( list->last->data, data3 );

  // remove last item
  ck_assert( list_remove_data( list, data2 ) );
  // assert first and last pointer
  ck_assert_ptr_eq( list->first->data, data1 );
  ck_assert_ptr_eq( list->last->data, data3 );
  // assert connection between remaining items
  ck_assert_ptr_null( list->first->previous );
  ck_assert_ptr_eq( list->first->next, list->last );
  ck_assert_ptr_null( list->last->next );
  ck_assert_ptr_eq( list->last->previous, list->first );
}
END_TEST

START_TEST( test_remove_data_last ) {
  ck_assert( list_empty( list ) );
  void* data1 = ( void* )5;
  void* data2 = ( void* )10;
  void* data3 = ( void* )15;

  // insert items
  ck_assert( list_push_back_data( list, data1 ) );
  ck_assert( list_push_back_data( list, data2 ) );
  ck_assert( list_push_back_data( list, data3 ) );
  // assert first and last pointer
  ck_assert_ptr_eq( list->first->data, data1 );
  ck_assert_ptr_eq( list->last->data, data3 );

  // remove last item
  ck_assert( list_remove_data( list, data3 ) );
  // assert first and last pointer
  ck_assert_ptr_eq( list->first->data, data1 );
  ck_assert_ptr_eq( list->last->data, data2 );
  // assert connection between remaining items
  ck_assert_ptr_null( list->first->previous );
  ck_assert_ptr_eq( list->first->next, list->last );
  ck_assert_ptr_null( list->last->next );
  ck_assert_ptr_eq( list->last->previous, list->first );
}
END_TEST

START_TEST( test_pop_front ) {
  ck_assert( list_empty( list ) );
  void* data1 = ( void* )5;
  void* data2 = ( void* )10;
  void* data3 = ( void* )15;

  // insert items
  ck_assert( list_push_back_data( list, data1 ) );
  ck_assert( list_push_back_data( list, data2 ) );
  ck_assert( list_push_back_data( list, data3 ) );
  // assert first and last pointer
  ck_assert_ptr_eq( list->first->data, data1 );
  ck_assert_ptr_eq( list->last->data, data3 );

  // pop front
  void* front = list_pop_front_data( list );
  ck_assert_ptr_eq( front, data1 );
  // check for changed head and tail
  ck_assert_ptr_eq( list->first->data, data2 );
  ck_assert_ptr_eq( list->last->data, data3 );

  // pop front
  front = list_pop_front_data( list );
  ck_assert_ptr_eq( front, data2 );
  // check for changed head and tail
  ck_assert_ptr_eq( list->first->data, data3 );
  ck_assert_ptr_eq( list->last->data, data3 );

  // pop front
  front = list_pop_front_data( list );
  ck_assert_ptr_eq( front, data3 );
  // check for empty
  ck_assert( list_empty( list ) );

  // pop front
  front = list_pop_front_data( list );
  ck_assert_ptr_null( front );
  // check for empty
  ck_assert( list_empty( list ) );
}
END_TEST

START_TEST( test_pop_back ) {
  ck_assert( list_empty( list ) );
  void* data1 = ( void* )5;
  void* data2 = ( void* )10;
  void* data3 = ( void* )15;

  // insert items
  ck_assert( list_push_back_data( list, data1 ) );
  ck_assert( list_push_back_data( list, data2 ) );
  ck_assert( list_push_back_data( list, data3 ) );
  // assert first and last pointer
  ck_assert_ptr_eq( list->first->data, data1 );
  ck_assert_ptr_eq( list->last->data, data3 );

  // pop back
  void* back = list_pop_back_data( list );
  ck_assert_ptr_eq( back, data3 );
  // check for changed head and tail
  ck_assert_ptr_eq( list->first->data, data1 );
  ck_assert_ptr_eq( list->last->data, data2 );

  // pop back
  back = list_pop_back_data( list );
  ck_assert_ptr_eq( back, data2 );
  // check for changed head and tail
  ck_assert_ptr_eq( list->first->data, data1 );
  ck_assert_ptr_eq( list->last->data, data1 );

  // pop back
  back = list_pop_back_data( list );
  ck_assert_ptr_eq( back, data1 );
  // check for empty
  ck_assert( list_empty( list ) );

  // pop back
  back = list_pop_back_data( list );
  ck_assert_ptr_null( back );
  // check for empty
  ck_assert( list_empty( list ) );
}
END_TEST

START_TEST( test_peek_front ) {
  ck_assert( list_empty( list ) );
  void* data1 = ( void* )5;
  void* data2 = ( void* )10;
  void* data3 = ( void* )15;

  // insert items
  ck_assert( list_push_back_data( list, data1 ) );
  ck_assert( list_push_back_data( list, data2 ) );
  ck_assert( list_push_back_data( list, data3 ) );
  // assert first and last pointer
  ck_assert_ptr_eq( list->first->data, data1 );
  ck_assert_ptr_eq( list->last->data, data3 );

  // pop front
  void* front = list_peek_front_data( list );
  ck_assert_ptr_eq( front, data1 );
  // check for changed head
  ck_assert_ptr_eq( list->first->data, data1 );
}
END_TEST

START_TEST( test_peek_back ) {
  ck_assert( list_empty( list ) );
  void* data1 = ( void* )5;
  void* data2 = ( void* )10;
  void* data3 = ( void* )15;

  // insert items
  ck_assert( list_push_back_data( list, data1 ) );
  ck_assert( list_push_back_data( list, data2 ) );
  ck_assert( list_push_back_data( list, data3 ) );
  // assert first and last pointer
  ck_assert_ptr_eq( list->first->data, data1 );
  ck_assert_ptr_eq( list->last->data, data3 );

  // pop front
  void* back = list_peek_back_data( list );
  ck_assert_ptr_eq( back, data3 );
  // check for changed head
  ck_assert_ptr_eq( list->last->data, data3 );
}
END_TEST

START_TEST( test_lookup_data_default ) {
  ck_assert( list_empty( list ) );
  void* data1 = ( void* )5;
  void* data2 = ( void* )10;
  void* data3 = ( void* )15;
  void* data4 = ( void* )20;

  // insert items
  ck_assert( list_insert_data( list, data1 ) );
  ck_assert( list_insert_data( list, data2 ) );
  ck_assert( list_insert_data( list, data3 ) );
  // assert first and last pointer
  ck_assert_ptr_eq( list->first->data, data1 );
  ck_assert_ptr_eq( list->last->data, data3 );

  // test lookup of data
  ck_assert_ptr_eq( list_lookup_data( list, data3 ), list->last );
  ck_assert_ptr_eq( list_lookup_data( list, data2 ), list->first->next );
  ck_assert_ptr_eq( list_lookup_data( list, data2 ), list->last->previous );
  ck_assert_ptr_eq( list_lookup_data( list, data1 ), list->first );
  ck_assert_ptr_null( list_lookup_data( list, data4 ) );
}
END_TEST

START_TEST( test_lookup_item_default ) {
  ck_assert( list_empty( list ) );
  void* data1 = ( void* )5;
  void* data2 = ( void* )10;
  void* data3 = ( void* )15;
  void* data4 = ( void* )20;

  // insert items
  ck_assert( list_insert_data( list, data1 ) );
  ck_assert( list_insert_data( list, data2 ) );
  ck_assert( list_insert_data( list, data3 ) );
  // assert first and last pointer
  ck_assert_ptr_eq( list->first->data, data1 );
  ck_assert_ptr_eq( list->last->data, data3 );

  // test lookup of item
  ck_assert_ptr_eq( list_lookup_item( list, list->last ), list->last );
  ck_assert_ptr_eq( list_lookup_item( list, list->first->next ), list->first->next );
  ck_assert_ptr_eq( list_lookup_item( list, list->first->next ), list->last->previous );
  ck_assert_ptr_eq( list_lookup_item( list, list->first ), list->first );
  ck_assert_ptr_null( list_lookup_data( list, data4 ) );
}
END_TEST

START_TEST( test_lookup_data_custom ) {
  // destroy default list
  list_destruct( list );
  list = list_construct( list_custom_lookup, list_custom_cleanup, NULL );
  ck_assert_ptr_nonnull( list );
  ck_assert( list_empty( list ) );

  void* lookup_data1 = ( void* )5;
  void* lookup_data2 = ( void* )10;
  void* lookup_data3 = ( void* )15;
  void* lookup_data4 = ( void* )20;

  foobar_t* data1 = create_foobar( 5 );
  foobar_t* data2 = create_foobar( 10 );
  foobar_t* data3 = create_foobar( 15 );
  ck_assert_ptr_nonnull( data1 );
  ck_assert_ptr_nonnull( data2 );
  ck_assert_ptr_nonnull( data3 );

  // insert items
  ck_assert( list_insert_data( list, data1 ) );
  ck_assert( list_insert_data( list, data2 ) );
  ck_assert( list_insert_data( list, data3 ) );
  // assert first and last pointer
  ck_assert_ptr_eq( list->first->data, data1 );
  ck_assert_ptr_eq( list->last->data, data3 );

  // test lookup of data
  ck_assert_ptr_eq( list_lookup_data( list, lookup_data3 ), list->last );
  ck_assert_ptr_eq( list_lookup_data( list, lookup_data2 ), list->first->next );
  ck_assert_ptr_eq( list_lookup_data( list, lookup_data2 ), list->last->previous );
  ck_assert_ptr_eq( list_lookup_data( list, lookup_data1 ), list->first );
  ck_assert_ptr_null( list_lookup_data( list, lookup_data4 ) );
}
END_TEST

START_TEST( test_lookup_item_custom ) {
  // destroy default list
  list_destruct( list );
  list = list_construct( list_custom_lookup, list_custom_cleanup, NULL );
  ck_assert_ptr_nonnull( list );
  ck_assert( list_empty( list ) );

  foobar_t* data1 = create_foobar( 5 );
  foobar_t* data2 = create_foobar( 10 );
  foobar_t* data3 = create_foobar( 15 );
  foobar_t* data4 = create_foobar( 20 );
  ck_assert_ptr_nonnull( data1 );
  ck_assert_ptr_nonnull( data2 );
  ck_assert_ptr_nonnull( data3 );
  ck_assert_ptr_nonnull( data4 );

  list_item_t* item4 = list_item_create( data4 );
  ck_assert_ptr_nonnull( item4 );

  // insert items
  ck_assert( list_insert_data( list, data1 ) );
  ck_assert( list_insert_data( list, data2 ) );
  ck_assert( list_insert_data( list, data3 ) );
  // assert first and last pointer
  ck_assert_ptr_eq( list->first->data, data1 );
  ck_assert_ptr_eq( list->last->data, data3 );

  // test lookup of data
  ck_assert_ptr_eq( list_lookup_item( list, list->last ), list->last );
  ck_assert_ptr_eq( list_lookup_item( list, list->first->next ), list->first->next );
  ck_assert_ptr_eq( list_lookup_item( list, list->first->next ), list->last->previous );
  ck_assert_ptr_eq( list_lookup_item( list, list->first ), list->first );
  ck_assert_ptr_null( list_lookup_item( list, item4 ) );

  list_custom_cleanup( item4 );
}
END_TEST

START_TEST( test_print ) {
  ck_assert_ptr_nonnull( list );
  ck_assert( list_empty( list ) );

  void* data1 = ( void* )5;
  void* data2 = ( void* )10;
  void* data3 = ( void* )15;

  // insert items
  ck_assert( list_insert_data( list, data1 ) );
  ck_assert( list_insert_data( list, data2 ) );
  ck_assert( list_insert_data( list, data3 ) );
  // assert first and last pointer
  ck_assert_ptr_eq( list->first->data, data1 );
  ck_assert_ptr_eq( list->last->data, data3 );

  // allocate buffer and expected for print
  char* buffer = malloc( sizeof( char ) * 1024 );
  char* expected = malloc( sizeof( char ) * 1024 );
  ck_assert_ptr_nonnull( expected );
  ck_assert_ptr_nonnull( buffer );
  memset( expected, 0, 1024 * sizeof( char ) );
  memset( buffer, 0, 1024 * sizeof( char ) );

  // prepare expected
  sprintf( expected, "list->data = %p\r\n", data1 );
  sprintf( expected + strlen( expected ), "list->data = %p\r\n", data2 );
  sprintf( expected + strlen( expected ), "list->data = %p\r\n", data3 );

  // redirect stdout
  fflush( stdout );
  int stdout_backup = dup( STDOUT_FILENO );
  FILE* f = freopen( "/dev/null", "a", stdout );
  ck_assert_ptr_nonnull( f );
  ck_assert_int_eq( 0, setvbuf( stdout, buffer, _IOFBF, 1024 ) );

  // call print
  list_print( list );
  // flush stdout
  fflush( stdout );
  // assert buffer and expected
  ck_assert_str_eq( expected, buffer );

  // restore stdout
  f = freopen( "/dev/null", "a", stdout );
  ck_assert_ptr_nonnull( f );
  ck_assert_int_ne( -1, dup2( stdout_backup, STDOUT_FILENO ) );
  ck_assert_int_eq( 0, setvbuf( stdout, NULL, _IOFBF, BUFSIZ ) );

  // free buffer and expected
  free( buffer );
  free( expected );
}
END_TEST

START_TEST( test_list_insert_default ) {
  ck_assert( list_empty( list ) );
  void* data1 = ( void* )5;
  void* data2 = ( void* )10;
  void* data3 = ( void* )15;

  // insert items
  ck_assert( list_insert_data( list, data1 ) );
  ck_assert( list_insert_data( list, data2 ) );
  ck_assert( list_insert_data( list, data3 ) );
  // assert first and last pointer
  ck_assert_ptr_eq( list->first->data, data1 );
  ck_assert_ptr_eq( list->first->next->data, data2 );
  ck_assert_ptr_eq( list->first->next->previous->data, data1 );
  ck_assert_ptr_eq( list->first->next->next->data, data3 );
  ck_assert_ptr_eq( list->first->next->next->previous->data, data2 );
  ck_assert_ptr_eq( list->last->data, data3 );
}
END_TEST

START_TEST( test_list_insert_before_default ) {
  ck_assert( list_empty( list ) );
  void* data1 = ( void* )5;
  void* data2 = ( void* )10;
  void* data3 = ( void* )15;

  // insert items
  ck_assert( list_insert_data( list, data3 ) );
  ck_assert_ptr_eq( list->first->data, data3 );
  ck_assert_ptr_eq( list->last->data, data3 );
  ck_assert( list_insert_data_before( list, list->first, data2 ) );
  ck_assert_ptr_eq( list->first->data, data2 );
  ck_assert_ptr_eq( list->last->data, data3 );
  ck_assert( list_insert_data_before( list, list->last, data1 ) );
  ck_assert_ptr_eq( list->first->data, data2 );
  ck_assert_ptr_eq( list->first->next->data, data1 );
  ck_assert_ptr_eq( list->last->data, data3 );
  // assert first and last pointer
  ck_assert_ptr_eq( list->first->data, data2 );
  ck_assert_ptr_eq( list->first->next->data, data1 );
  ck_assert_ptr_eq( list->first->next->previous->data, data2 );
  ck_assert_ptr_eq( list->first->next->next->data, data3 );
  ck_assert_ptr_eq( list->first->next->next->previous->data, data1 );
  ck_assert_ptr_eq( list->last->data, data3 );
}
END_TEST

START_TEST( test_list_custom_insert ) {
  // destroy default list
  list_destruct( list );
  list = list_construct( NULL, NULL, list_custom_insert );
  ck_assert_ptr_nonnull( list );
  ck_assert( list_empty( list ) );

  void* data1 = ( void* )5;
  void* data2 = ( void* )10;
  void* data3 = ( void* )15;

  // insert items
  ck_assert( list_insert_data( list, data1 ) );
  ck_assert( list_insert_data( list, data2 ) );
  ck_assert( list_insert_data( list, data3 ) );
  // assert first and last pointer
  ck_assert_ptr_eq( list->first->data, data3 );
  ck_assert_ptr_eq( list->last->data, data1 );

  // test lookup of data
  ck_assert_ptr_eq( list->first->data, data3 );
  ck_assert_ptr_eq( list->first->next->data, data2 );
  ck_assert_ptr_eq( list->first->next->next->data, data1 );
  ck_assert_ptr_eq( list->last->data, data1 );
}
END_TEST

static Suite* lexer_suite( void ) {
  Suite* s;
  TCase* tc_core;

  s = suite_create( "libbosl" );
  // test cases
  tc_core = tcase_create( "collections" );
  // add tests
  tcase_add_checked_fixture( tc_core, setup, teardown );
  tcase_add_test( tc_core, test_construct );
  tcase_add_test( tc_core, test_empty );
  tcase_add_test( tc_core, test_item_create );
  tcase_add_test( tc_core, test_list_custom_insert );
  tcase_add_test( tc_core, test_list_insert_default );
  tcase_add_test( tc_core, test_list_insert_before_default );
  tcase_add_test( tc_core, test_push_front_data );
  tcase_add_test( tc_core, test_push_back_data );
  tcase_add_test( tc_core, test_remove_data_first );
  tcase_add_test( tc_core, test_remove_data_middle );
  tcase_add_test( tc_core, test_remove_data_last );
  tcase_add_test( tc_core, test_remove_item_first );
  tcase_add_test( tc_core, test_remove_item_middle );
  tcase_add_test( tc_core, test_remove_item_last );
  tcase_add_test( tc_core, test_pop_front );
  tcase_add_test( tc_core, test_pop_back );
  tcase_add_test( tc_core, test_peek_front );
  tcase_add_test( tc_core, test_peek_back );
  tcase_add_test( tc_core, test_lookup_data_default );
  tcase_add_test( tc_core, test_lookup_item_default );
  tcase_add_test( tc_core, test_lookup_data_custom );
  tcase_add_test( tc_core, test_lookup_item_custom );
  tcase_add_test( tc_core, test_print );
  suite_add_tcase( s, tc_core );
  // return suite
  return s;
}

int main( void ) {
  int number_failed;
  Suite *s;
  SRunner *sr;

  // init random
  srand( ( unsigned int )time( NULL ) );
  // create suite and runner
  s = lexer_suite();
  sr = srunner_create( s );

  srunner_run_all( sr, CK_VERBOSE );
  number_failed = srunner_ntests_failed( sr );
  srunner_free( sr );
  return ( 0 == number_failed )
    ? EXIT_SUCCESS
    : EXIT_FAILURE;
}
