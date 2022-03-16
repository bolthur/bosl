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
#include <unistd.h>
#include "../../library/error.h"

static char* buffer;
static char* expected;
static FILE* f;
static int stderr_backup;

static void setup( void ) {
  // allocate buffer and expected for print
  buffer = malloc( sizeof( char ) * 1024 );
  expected = malloc( sizeof( char ) * 1024 );
  ck_assert_ptr_nonnull( expected );
  ck_assert_ptr_nonnull( buffer );
  memset( expected, 0, 1024 * sizeof( char ) );
  memset( buffer, 0, 1024 * sizeof( char ) );

  // redirect stderr
  fflush( stderr );
  stderr_backup = dup( STDERR_FILENO );
  f = freopen( "/dev/null", "a", stderr );
  ck_assert_ptr_nonnull( f );
  ck_assert_int_eq( 0, setvbuf( stderr, buffer, _IOFBF, 1024 ) );
}

static void teardown( void ) {
  // restore stderr
  f = freopen( "/dev/null", "a", stderr );
  ck_assert_ptr_nonnull( f );
  ck_assert_int_ne( -1, dup2( stderr_backup, STDOUT_FILENO ) );
  ck_assert_int_eq( 0, setvbuf( stderr, NULL, _IOFBF, BUFSIZ ) );
  // free buffers
  free( expected );
  free( buffer );
}

START_TEST( test_error_at_eof ) {
  bosl_token_t token = {
    .type = TOKEN_EOF,
    .line = 5,
    .start = NULL,
  };
  // prepare expected
  sprintf(
    expected,
    "[line %u] Error at end: %s\r\n",
    token.line,
    "foo bar foo"
  );
  // call raise
  error_raise( &token, "foo bar foo" );
  // assert string equal
  ck_assert_str_eq( expected, buffer );
}
END_TEST

START_TEST( test_error_token ) {
  bosl_token_t token = {
    .type = TOKEN_ERROR,
    .line = 5,
    .start = NULL,
  };
  // prepare expected
  sprintf(
    expected,
    "[line %u] Error: %s\r\n",
    token.line,
    "token error"
  );
  // call raise
  error_raise( &token, "token error" );
  // assert string equal
  ck_assert_str_eq( expected, buffer );
}
END_TEST

START_TEST( test_error_normal ) {
  bosl_token_t token = {
    .type = TOKEN_COMMA,
    .line = 5,
    .start = "foo( a, b, )",
    .length = 12,
  };
  // prepare expected
  sprintf(
    expected,
    "[line %u] Error at '%.*s': %s\r\n",
    token.line,
    ( int )token.length,
    token.start,
    "trailing comma"
  );
  // call raise
  error_raise( &token, "trailing comma" );
  // assert string equal
  ck_assert_str_eq( expected, buffer );
}
END_TEST

static Suite* lexer_suite( void ) {
  Suite* s;
  TCase* tc_core;

  s = suite_create( "libbosl" );
  // test cases
  tc_core = tcase_create( "error" );
  // add tests
  tcase_add_checked_fixture( tc_core, setup, teardown );
  tcase_add_test( tc_core, test_error_at_eof );
  tcase_add_test( tc_core, test_error_token );
  tcase_add_test( tc_core, test_error_normal );
  suite_add_tcase( s, tc_core );
  // return suite
  return s;
}

int main( void ) {
  int number_failed;
  Suite *s;
  SRunner *sr;

  s = lexer_suite();
  sr = srunner_create( s );

  srunner_run_all( sr, CK_NORMAL );
  number_failed = srunner_ntests_failed( sr );
  srunner_free( sr );
  return ( 0 == number_failed ) ? EXIT_SUCCESS : EXIT_FAILURE;
}
