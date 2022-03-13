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
#include <check.h>
#include <stdio.h>
#include "../../library/collection/hashmap.h"

hashmap_table_t* table;

static void setup( void ) {
  table = hashmap_construct();
  ck_assert_ptr_nonnull( table );
}

static void teardown( void ) {
  hashmap_destruct( table );
  table = NULL;
}

START_TEST( test_hashmap_create ) {
  hashmap_destruct( table );
  table = hashmap_construct();
  ck_assert_ptr_nonnull( table );
  ck_assert( 0 == table->length );
  ck_assert( HASHMAP_INITIAL_CAPACITY == table->capacity );
}
END_TEST

START_TEST( test_hashmap_add ) {
  // add key
  const char* added_key = hashmap_value_set( table, "foo", ( void* )5 );
  // ensure it was added
  ck_assert_ptr_nonnull( added_key );
  ck_assert( 1 == hashmap_length( table ) );
}
END_TEST

START_TEST( test_hashmap_add_multiple ) {
  ck_assert_ptr_nonnull( hashmap_value_set( table, "foo1", ( void* )5 ) );
  ck_assert( 1 == hashmap_length( table ) );
  ck_assert_ptr_nonnull( hashmap_value_set( table, "foo2", ( void* )5 ) );
  ck_assert( 2 == hashmap_length( table ) );
  ck_assert_ptr_nonnull( hashmap_value_set( table, "foo3", ( void* )5 ) );
  ck_assert( 3 == hashmap_length( table ) );
  ck_assert_ptr_nonnull( hashmap_value_set( table, "foo4", ( void* )5 ) );
  ck_assert( 4 == hashmap_length( table ) );
  ck_assert_ptr_nonnull( hashmap_value_set( table, "foo5", ( void* )5 ) );
  ck_assert( 5 == hashmap_length( table ) );
}
END_TEST

START_TEST( test_hashmap_get_not_added ) {
  // variables for expected and retrieved
  void* expected = NULL;
  void* retrieved = hashmap_value_get( table, "foo" );
  // assert not found
  ck_assert_ptr_eq( expected, retrieved );
}
END_TEST

START_TEST( test_hashmap_get_added ) {
  // variables for expected and retrieved
  void* key = ( void* )5;
  void* expected = NULL;
  void* retrieved = hashmap_value_get( table, "foo" );
  // assert not found
  ck_assert_ptr_eq( expected, retrieved );

  // add key
  const char* added_key = hashmap_value_set( table, "foo", key );
  // ensure it was added
  ck_assert_ptr_nonnull( added_key );
  ck_assert( 1 == hashmap_length( table ) );

  // update expected
  expected = key;
  // try to get via key
  retrieved = hashmap_value_get( table, added_key );
  // assert not found
  ck_assert_ptr_eq( expected, retrieved );
}
END_TEST

START_TEST( test_hashmap_update ) {
  // variables for expected and retrieved
  void* key = ( void* )5;
  void* expected = NULL;
  void* retrieved = hashmap_value_get( table, "foo" );
  // assert not found
  ck_assert_ptr_eq( expected, retrieved );

  // add key
  const char* added_key = hashmap_value_set( table, "foo", key );
  // ensure it was added
  ck_assert_ptr_nonnull( added_key );
  ck_assert( 1 == hashmap_length( table ) );

  // update expected
  expected = key;
  // try to get via key
  retrieved = hashmap_value_get( table, added_key );
  // assert found
  ck_assert_ptr_eq( expected, retrieved );

  // update key
  key = ( void* )10;
  const char* updated_key = hashmap_value_set( table, added_key, key );
  // ensure no error occurred
  ck_assert_ptr_nonnull( added_key );
  ck_assert( 1 == hashmap_length( table ) );
  ck_assert_ptr_eq( updated_key, added_key );
  ck_assert_str_eq( updated_key, added_key );

  // update expected
  expected = key;
  // try to get via key
  retrieved = hashmap_value_get( table, added_key );
  // assert not found
  ck_assert_ptr_eq( expected, retrieved );
}
END_TEST

START_TEST( test_hashmap_iterator ) {
  // populate list
  ck_assert_ptr_nonnull( hashmap_value_set( table, "foo1", ( void* )1 ) );
  ck_assert_ptr_nonnull( hashmap_value_set( table, "foo2", ( void* )2 ) );
  ck_assert_ptr_nonnull( hashmap_value_set( table, "foo3", ( void* )3 ) );
  ck_assert_ptr_nonnull( hashmap_value_set( table, "foo4", ( void* )4 ) );
  ck_assert_ptr_nonnull( hashmap_value_set( table, "foo5", ( void* )5 ) );
  ck_assert( 5 == hashmap_length( table ) );
  // get iterator
  hashmap_iterator_t it = hashmap_iterator( table );
  // some assertions
  ck_assert( 0 == it._index );
  ck_assert_ptr_eq( table, it._table );
  // loop
  while( hashmap_next( &it ) ) {
    size_t num;
    char buf[ 10 ];
    // get number
    ck_assert( 1 == sscanf( it.key, "%*[^0123456789]%zu", &num ) );
    // assert correct value
    ck_assert_ptr_eq( it.value, ( void* )num );
    ck_assert( 4 == sprintf( buf, "foo%zu", num ) );
    // assert correct key
    ck_assert_str_eq( buf, it.key );
  }
}
END_TEST

static Suite* hashmap_suite( void ) {
  Suite* s;
  TCase* tc_core;

  s = suite_create( "libbosl" );
  // test cases
  tc_core = tcase_create( "hashmap" );
  // add tests
  tcase_add_checked_fixture( tc_core, setup, teardown );
  tcase_add_test( tc_core, test_hashmap_create );
  tcase_add_test( tc_core, test_hashmap_add );
  tcase_add_test( tc_core, test_hashmap_add_multiple );
  tcase_add_test( tc_core, test_hashmap_get_not_added );
  tcase_add_test( tc_core, test_hashmap_get_added );
  tcase_add_test( tc_core, test_hashmap_update );
  tcase_add_test( tc_core, test_hashmap_iterator );
  suite_add_tcase( s, tc_core );
  // return suite
  return s;
}

int main( void ) {
  int number_failed;
  Suite *s;
  SRunner *sr;

  s = hashmap_suite();
  sr = srunner_create( s );

  srunner_run_all( sr, CK_NORMAL );
  number_failed = srunner_ntests_failed( sr );
  srunner_free( sr );
  return ( 0 == number_failed ) ? EXIT_SUCCESS : EXIT_FAILURE;
}
