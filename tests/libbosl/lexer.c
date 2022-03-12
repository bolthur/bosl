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
#include "../../library/interpreter/lexer.h"

static void setup( void ) {
}

static void teardown( void ) {
  lexer_free();
}

START_TEST( test_lexer_init ) {
  ck_assert( lexer_init( "let foo: uint32_t = 5" ) );
}
END_TEST

START_TEST( test_lexer_scan_string_single_line ) {
  char str[] = "\"some string\"";
  char* cmp = str;
  ck_assert( lexer_init( str ) );
  list_manager_t* list = lexer_scan();
  ck_assert_ptr_nonnull( list );

  list_item_t* current = list->first;
  bosl_token_t* token = current->data;
  ck_assert_int_eq( token->type, TOKEN_STRING );
  ck_assert( 13 ==  token->length );
  ck_assert_str_eq( token->start, cmp );
  ck_assert_int_eq( token->line, 1 );
}
END_TEST

START_TEST( test_lexer_scan_string_multi_line ) {
  char str[] = "\"some\n\
string\"";
  char* cmp = str;
  ck_assert( lexer_init( str ) );
  list_manager_t* list = lexer_scan();
  ck_assert_ptr_nonnull( list );

  list_item_t* current = list->first;
  bosl_token_t* token = current->data;
  ck_assert_int_eq( token->type, TOKEN_STRING );
  ck_assert( 13 ==  token->length );
  ck_assert_str_eq( token->start, cmp );
  ck_assert_int_eq( token->line, 2 );
}
END_TEST

START_TEST( test_lexer_scan_int ) {
  char str[] = "1337";
  char* cmp = str;
  ck_assert( lexer_init( str ) );
  list_manager_t* list = lexer_scan();
  ck_assert_ptr_nonnull( list );

  list_item_t* current = list->first;
  bosl_token_t* token = current->data;
  ck_assert_int_eq( token->type, TOKEN_NUMBER );
  ck_assert( 4 ==  token->length );
  ck_assert_str_eq( token->start, cmp );
  ck_assert_int_eq( token->line, 1 );
}
END_TEST

START_TEST( test_lexer_scan_float ) {
  char str[] = "13.37";
  char* cmp = str;
  ck_assert( lexer_init( str ) );
  list_manager_t* list = lexer_scan();
  ck_assert_ptr_nonnull( list );

  list_item_t* current = list->first;
  bosl_token_t* token = current->data;
  ck_assert_int_eq( token->type, TOKEN_NUMBER );
  ck_assert( 5 ==  token->length );
  ck_assert_str_eq( token->start, cmp );
  ck_assert_int_eq( token->line, 1 );
}
END_TEST

START_TEST( test_lexer_scan_hex_lower ) {
  char str[] = "0x123abc";
  char* cmp = str;
  ck_assert( lexer_init( str ) );
  list_manager_t* list = lexer_scan();
  ck_assert_ptr_nonnull( list );

  list_item_t* current = list->first;
  bosl_token_t* token = current->data;
  ck_assert_int_eq( token->type, TOKEN_NUMBER );
  ck_assert( 8 ==  token->length );
  ck_assert_str_eq( token->start, cmp );
  ck_assert_int_eq( token->line, 1 );
}
END_TEST

START_TEST( test_lexer_scan_hex_upper ) {
  char str[] = "0X123ABC";
  char* cmp = str;
  ck_assert( lexer_init( str ) );
  list_manager_t* list = lexer_scan();
  ck_assert_ptr_nonnull( list );

  list_item_t* current = list->first;
  bosl_token_t* token = current->data;
  ck_assert_int_eq( token->type, TOKEN_NUMBER );
  ck_assert( 8 ==  token->length );
  ck_assert_str_eq( token->start, cmp );
  ck_assert_int_eq( token->line, 1 );
}
END_TEST

START_TEST( test_lexer_scan_hex_mixed_lower_x ) {
  char str[] = "0x123AbC";
  char* cmp = str;
  ck_assert( lexer_init( str ) );
  list_manager_t* list = lexer_scan();
  ck_assert_ptr_nonnull( list );

  list_item_t* current = list->first;
  bosl_token_t* token = current->data;
  ck_assert_int_eq( token->type, TOKEN_NUMBER );
  ck_assert( 8 ==  token->length );
  ck_assert_str_eq( token->start, cmp );
  ck_assert_int_eq( token->line, 1 );
}
END_TEST

START_TEST( test_lexer_scan_hex_mixed_upper_x ) {
  char str[] = "0X123AbC";
  char* cmp = str;
  ck_assert( lexer_init( str ) );
  list_manager_t* list = lexer_scan();
  ck_assert_ptr_nonnull( list );

  list_item_t* current = list->first;
  bosl_token_t* token = current->data;
  ck_assert_int_eq( token->type, TOKEN_NUMBER );
  ck_assert( 8 ==  token->length );
  ck_assert_str_eq( token->start, cmp );
  ck_assert_int_eq( token->line, 1 );
}
END_TEST

START_TEST( test_lexer_scan_variable_declaration_1 ) {
  char str[] = "let foo: uint32 = 5;";
  char* cmp = str;
  ck_assert( lexer_init( str ) );
  list_manager_t* list = lexer_scan();
  ck_assert_ptr_nonnull( list );

  list_item_t* current = list->first;
  bosl_token_t* token = current->data;
  ck_assert_int_eq( token->type, TOKEN_LET );
  ck_assert( 3 ==  token->length );
  ck_assert_str_eq( token->start, cmp );
  ck_assert_int_eq( token->line, 1 );

  current = current->next;
  token = current->data;
  cmp += 4;
  ck_assert_int_eq( token->type, TOKEN_IDENTIFIER );
  ck_assert( 3 ==  token->length );
  ck_assert_str_eq( token->start, cmp );
  ck_assert_int_eq( token->line, 1 );

  current = current->next;
  token = current->data;
  cmp += 3;
  ck_assert_int_eq( token->type, TOKEN_COLON );
  ck_assert( 1 ==  token->length );
  ck_assert_str_eq( token->start, cmp );
  ck_assert_int_eq( token->line, 1 );

  current = current->next;
  token = current->data;
  cmp += 2;
  ck_assert_int_eq( token->type, TOKEN_TYPE_UINT32 );
  ck_assert( 6 ==  token->length );
  ck_assert_str_eq( token->start, cmp );
  ck_assert_int_eq( token->line, 1 );

  current = current->next;
  token = current->data;
  cmp += 7;
  ck_assert_int_eq( token->type, TOKEN_EQUAL );
  ck_assert( 1 ==  token->length );
  ck_assert_str_eq( token->start, cmp );
  ck_assert_int_eq( token->line, 1 );

  current = current->next;
  token = current->data;
  cmp += 2;
  ck_assert_int_eq( token->type, TOKEN_NUMBER );
  ck_assert( 1 ==  token->length );
  ck_assert_str_eq( token->start, cmp );
  ck_assert_int_eq( token->line, 1 );

  current = current->next;
  token = current->data;
  cmp += 1;
  ck_assert_int_eq( token->type, TOKEN_SEMICOLON );
  ck_assert( 1 ==  token->length );
  ck_assert_str_eq( token->start, cmp );
  ck_assert_int_eq( token->line, 1 );
}
END_TEST

static Suite* lexer_suite( void ) {
  Suite* s;
  TCase* tc_core;

  s = suite_create( "libbosl" );
  // test cases
  tc_core = tcase_create( "lexer" );
  // add tests
  tcase_add_checked_fixture( tc_core, setup, teardown );
  tcase_add_test( tc_core, test_lexer_init );
  tcase_add_test( tc_core, test_lexer_scan_string_single_line );
  tcase_add_test( tc_core, test_lexer_scan_string_multi_line );
  tcase_add_test( tc_core, test_lexer_scan_int );
  tcase_add_test( tc_core, test_lexer_scan_float );
  tcase_add_test( tc_core, test_lexer_scan_hex_lower );
  tcase_add_test( tc_core, test_lexer_scan_hex_upper );
  tcase_add_test( tc_core, test_lexer_scan_hex_mixed_lower_x );
  tcase_add_test( tc_core, test_lexer_scan_hex_mixed_upper_x );
  tcase_add_test( tc_core, test_lexer_scan_variable_declaration_1 );
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
