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
#include "../lib/scanner.h"

static void setup( void ) {
}

static void teardown( void ) {
  scanner_free();
}

START_TEST( test_scanner_init ) {
  ck_assert( scanner_init( "let foo: uint32_t = 5" ) );
}
END_TEST

START_TEST( test_scanner_scan_string_single_line ) {
  char str[] = "\"some string\"";
  char* cmp = str;
  ck_assert( scanner_init( str ) );
  list_manager_t* list = scanner_scan();
  ck_assert_ptr_nonnull( list );

  list_item_t* current = list->first;
  bosl_token_t* token = current->data;
  ck_assert_int_eq( token->type, TOKEN_STRING );
  ck_assert( 13 ==  token->length );
  ck_assert_str_eq( token->start, cmp );
  ck_assert_int_eq( token->line, 1 );

  cmp += 13;
  current = current->next;
  token = current->data;
  ck_assert_int_eq( token->type, TOKEN_EOF );
  ck_assert( 0 ==  token->length );
  ck_assert_str_eq( token->start, cmp );
  ck_assert_int_eq( token->line, 1 );
}
END_TEST

START_TEST( test_scanner_scan_string_multi_line ) {
  char str[] = "\"some\n\
string\"";
  char* cmp = str;
  ck_assert( scanner_init( str ) );
  list_manager_t* list = scanner_scan();
  ck_assert_ptr_nonnull( list );

  list_item_t* current = list->first;
  bosl_token_t* token = current->data;
  ck_assert_int_eq( token->type, TOKEN_STRING );
  ck_assert( 13 ==  token->length );
  ck_assert_str_eq( token->start, cmp );
  ck_assert_int_eq( token->line, 2 );

  cmp += 13;
  current = current->next;
  token = current->data;
  ck_assert_int_eq( token->type, TOKEN_EOF );
  ck_assert( 0 ==  token->length );
  ck_assert_str_eq( token->start, cmp );
  ck_assert_int_eq( token->line, 2 );
}
END_TEST

START_TEST( test_scanner_scan_string_invalid ) {
  char str[] = "\"some string";
  char* cmp = str;
  char error_cmp[] = "Unterminated string found";
  ck_assert( scanner_init( str ) );
  list_manager_t* list = scanner_scan();
  ck_assert_ptr_nonnull( list );

  list_item_t* current = list->first;
  bosl_token_t* token = current->data;
  ck_assert_int_eq( token->type, TOKEN_ERROR );
  ck_assert( 25 ==  token->length );
  ck_assert_str_eq( token->start, error_cmp );
  ck_assert_int_eq( token->line, 1 );

  cmp += 12;
  current = current->next;
  token = current->data;
  ck_assert_int_eq( token->type, TOKEN_EOF );
  ck_assert( 0 ==  token->length );
  ck_assert_str_eq( token->start, cmp );
  ck_assert_int_eq( token->line, 1 );
}
END_TEST

START_TEST( test_scanner_scan_int ) {
  char str[] = "1337";
  char* cmp = str;
  ck_assert( scanner_init( str ) );
  list_manager_t* list = scanner_scan();
  ck_assert_ptr_nonnull( list );

  list_item_t* current = list->first;
  bosl_token_t* token = current->data;
  ck_assert_int_eq( token->type, TOKEN_NUMBER );
  ck_assert( 4 ==  token->length );
  ck_assert_str_eq( token->start, cmp );
  ck_assert_int_eq( token->line, 1 );

  cmp += 4;
  current = current->next;
  token = current->data;
  ck_assert_int_eq( token->type, TOKEN_EOF );
  ck_assert( 0 ==  token->length );
  ck_assert_str_eq( token->start, cmp );
  ck_assert_int_eq( token->line, 1 );
}
END_TEST

START_TEST( test_scanner_scan_float ) {
  char str[] = "13.37";
  char* cmp = str;
  ck_assert( scanner_init( str ) );
  list_manager_t* list = scanner_scan();
  ck_assert_ptr_nonnull( list );

  list_item_t* current = list->first;
  bosl_token_t* token = current->data;
  ck_assert_int_eq( token->type, TOKEN_NUMBER );
  ck_assert( 5 ==  token->length );
  ck_assert_str_eq( token->start, cmp );
  ck_assert_int_eq( token->line, 1 );

  cmp += 5;
  current = current->next;
  token = current->data;
  ck_assert_int_eq( token->type, TOKEN_EOF );
  ck_assert( 0 ==  token->length );
  ck_assert_str_eq( token->start, cmp );
  ck_assert_int_eq( token->line, 1 );
}
END_TEST

START_TEST( test_scanner_scan_hex_lower ) {
  char str[] = "0x123abc";
  char* cmp = str;
  ck_assert( scanner_init( str ) );
  list_manager_t* list = scanner_scan();
  ck_assert_ptr_nonnull( list );

  list_item_t* current = list->first;
  bosl_token_t* token = current->data;
  ck_assert_int_eq( token->type, TOKEN_NUMBER );
  ck_assert( 8 ==  token->length );
  ck_assert_str_eq( token->start, cmp );
  ck_assert_int_eq( token->line, 1 );

  cmp += 8;
  current = current->next;
  token = current->data;
  ck_assert_int_eq( token->type, TOKEN_EOF );
  ck_assert( 0 ==  token->length );
  ck_assert_str_eq( token->start, cmp );
  ck_assert_int_eq( token->line, 1 );
}
END_TEST

START_TEST( test_scanner_scan_hex_upper ) {
  char str[] = "0X123ABC";
  char* cmp = str;
  ck_assert( scanner_init( str ) );
  list_manager_t* list = scanner_scan();
  ck_assert_ptr_nonnull( list );

  list_item_t* current = list->first;
  bosl_token_t* token = current->data;
  ck_assert_int_eq( token->type, TOKEN_NUMBER );
  ck_assert( 8 ==  token->length );
  ck_assert_str_eq( token->start, cmp );
  ck_assert_int_eq( token->line, 1 );

  cmp += 8;
  current = current->next;
  token = current->data;
  ck_assert_int_eq( token->type, TOKEN_EOF );
  ck_assert( 0 ==  token->length );
  ck_assert_str_eq( token->start, cmp );
  ck_assert_int_eq( token->line, 1 );
}
END_TEST

START_TEST( test_scanner_scan_hex_mixed_lower_x ) {
  char str[] = "0x123AbC";
  char* cmp = str;
  ck_assert( scanner_init( str ) );
  list_manager_t* list = scanner_scan();
  ck_assert_ptr_nonnull( list );

  list_item_t* current = list->first;
  bosl_token_t* token = current->data;
  ck_assert_int_eq( token->type, TOKEN_NUMBER );
  ck_assert( 8 ==  token->length );
  ck_assert_str_eq( token->start, cmp );
  ck_assert_int_eq( token->line, 1 );

  cmp += 8;
  current = current->next;
  token = current->data;
  ck_assert_int_eq( token->type, TOKEN_EOF );
  ck_assert( 0 ==  token->length );
  ck_assert_str_eq( token->start, cmp );
  ck_assert_int_eq( token->line, 1 );
}
END_TEST

START_TEST( test_scanner_scan_hex_mixed_upper_x ) {
  char str[] = "0X123AbC";
  char* cmp = str;
  ck_assert( scanner_init( str ) );
  list_manager_t* list = scanner_scan();
  ck_assert_ptr_nonnull( list );

  list_item_t* current = list->first;
  bosl_token_t* token = current->data;
  ck_assert_int_eq( token->type, TOKEN_NUMBER );
  ck_assert( 8 ==  token->length );
  ck_assert_str_eq( token->start, cmp );
  ck_assert_int_eq( token->line, 1 );

  cmp += 8;
  current = current->next;
  token = current->data;
  ck_assert_int_eq( token->type, TOKEN_EOF );
  ck_assert( 0 ==  token->length );
  ck_assert_str_eq( token->start, cmp );
  ck_assert_int_eq( token->line, 1 );
}
END_TEST

START_TEST( test_scanner_scan_literal_keyword_within ) {
  char str[] = "elseifabc";
  char* cmp = str;
  ck_assert( scanner_init( str ) );
  list_manager_t* list = scanner_scan();
  ck_assert_ptr_nonnull( list );

  list_item_t* current = list->first;
  bosl_token_t* token = current->data;
  ck_assert_int_eq( token->type, TOKEN_IDENTIFIER );
  ck_assert( 9 ==  token->length );
  ck_assert_str_eq( token->start, cmp );
  ck_assert_int_eq( token->line, 1 );

  cmp += 9;
  current = current->next;
  token = current->data;
  ck_assert_int_eq( token->type, TOKEN_EOF );
  ck_assert( 0 ==  token->length );
  ck_assert_str_eq( token->start, cmp );
  ck_assert_int_eq( token->line, 1 );
}
END_TEST

START_TEST( test_scanner_scan_literal_no_keyword_within ) {
  char str[] = "isaac_newton";
  char* cmp = str;
  ck_assert( scanner_init( str ) );
  list_manager_t* list = scanner_scan();
  ck_assert_ptr_nonnull( list );

  list_item_t* current = list->first;
  bosl_token_t* token = current->data;
  ck_assert_int_eq( token->type, TOKEN_IDENTIFIER );
  ck_assert( 12 ==  token->length );
  ck_assert_str_eq( token->start, cmp );
  ck_assert_int_eq( token->line, 1 );

  cmp += 12;
  current = current->next;
  token = current->data;
  ck_assert_int_eq( token->type, TOKEN_EOF );
  ck_assert( 0 ==  token->length );
  ck_assert_str_eq( token->start, cmp );
  ck_assert_int_eq( token->line, 1 );
}
END_TEST

START_TEST( test_scanner_scan_keyword_let ) {
  char str[] = "let";
  char* cmp = str;
  ck_assert( scanner_init( str ) );
  list_manager_t* list = scanner_scan();
  ck_assert_ptr_nonnull( list );

  list_item_t* current = list->first;
  bosl_token_t* token = current->data;
  ck_assert_int_eq( token->type, TOKEN_LET );
  ck_assert( 3 ==  token->length );
  ck_assert_str_eq( token->start, cmp );
  ck_assert_int_eq( token->line, 1 );

  cmp += 3;
  current = current->next;
  token = current->data;
  ck_assert_int_eq( token->type, TOKEN_EOF );
  ck_assert( 0 ==  token->length );
  ck_assert_str_eq( token->start, cmp );
  ck_assert_int_eq( token->line, 1 );
}
END_TEST

START_TEST( test_scanner_scan_keyword_const ) {
  char str[] = "const";
  char* cmp = str;
  ck_assert( scanner_init( str ) );
  list_manager_t* list = scanner_scan();
  ck_assert_ptr_nonnull( list );

  list_item_t* current = list->first;
  bosl_token_t* token = current->data;
  ck_assert_int_eq( token->type, TOKEN_CONST );
  ck_assert( 5 ==  token->length );
  ck_assert_str_eq( token->start, cmp );
  ck_assert_int_eq( token->line, 1 );

  cmp += 5;
  current = current->next;
  token = current->data;
  ck_assert_int_eq( token->type, TOKEN_EOF );
  ck_assert( 0 ==  token->length );
  ck_assert_str_eq( token->start, cmp );
  ck_assert_int_eq( token->line, 1 );
}
END_TEST

START_TEST( test_scanner_scan_keyword_pointer ) {
  char str[] = "pointer";
  char* cmp = str;
  ck_assert( scanner_init( str ) );
  list_manager_t* list = scanner_scan();
  ck_assert_ptr_nonnull( list );

  list_item_t* current = list->first;
  bosl_token_t* token = current->data;
  ck_assert_int_eq( token->type, TOKEN_POINTER );
  ck_assert( 7 ==  token->length );
  ck_assert_str_eq( token->start, cmp );
  ck_assert_int_eq( token->line, 1 );

  cmp += 7;
  current = current->next;
  token = current->data;
  ck_assert_int_eq( token->type, TOKEN_EOF );
  ck_assert( 0 ==  token->length );
  ck_assert_str_eq( token->start, cmp );
  ck_assert_int_eq( token->line, 1 );
}
END_TEST

START_TEST( test_scanner_scan_keyword_true ) {
  char str[] = "true";
  char* cmp = str;
  ck_assert( scanner_init( str ) );
  list_manager_t* list = scanner_scan();
  ck_assert_ptr_nonnull( list );

  list_item_t* current = list->first;
  bosl_token_t* token = current->data;
  ck_assert_int_eq( token->type, TOKEN_TRUE );
  ck_assert( 4 ==  token->length );
  ck_assert_str_eq( token->start, cmp );
  ck_assert_int_eq( token->line, 1 );

  cmp += 4;
  current = current->next;
  token = current->data;
  ck_assert_int_eq( token->type, TOKEN_EOF );
  ck_assert( 0 ==  token->length );
  ck_assert_str_eq( token->start, cmp );
  ck_assert_int_eq( token->line, 1 );
}
END_TEST

START_TEST( test_scanner_scan_keyword_false ) {
  char str[] = "false";
  char* cmp = str;
  ck_assert( scanner_init( str ) );
  list_manager_t* list = scanner_scan();
  ck_assert_ptr_nonnull( list );

  list_item_t* current = list->first;
  bosl_token_t* token = current->data;
  ck_assert_int_eq( token->type, TOKEN_FALSE );
  ck_assert( 5 ==  token->length );
  ck_assert_str_eq( token->start, cmp );
  ck_assert_int_eq( token->line, 1 );

  cmp += 5;
  current = current->next;
  token = current->data;
  ck_assert_int_eq( token->type, TOKEN_EOF );
  ck_assert( 0 ==  token->length );
  ck_assert_str_eq( token->start, cmp );
  ck_assert_int_eq( token->line, 1 );
}
END_TEST

START_TEST( test_scanner_scan_keyword_null ) {
  char str[] = "null";
  char* cmp = str;
  ck_assert( scanner_init( str ) );
  list_manager_t* list = scanner_scan();
  ck_assert_ptr_nonnull( list );

  list_item_t* current = list->first;
  bosl_token_t* token = current->data;
  ck_assert_int_eq( token->type, TOKEN_NULL );
  ck_assert( 4 ==  token->length );
  ck_assert_str_eq( token->start, cmp );
  ck_assert_int_eq( token->line, 1 );

  cmp += 4;
  current = current->next;
  token = current->data;
  ck_assert_int_eq( token->type, TOKEN_EOF );
  ck_assert( 0 ==  token->length );
  ck_assert_str_eq( token->start, cmp );
  ck_assert_int_eq( token->line, 1 );
}
END_TEST

START_TEST( test_scanner_scan_keyword_if ) {
  char str[] = "if";
  char* cmp = str;
  ck_assert( scanner_init( str ) );
  list_manager_t* list = scanner_scan();
  ck_assert_ptr_nonnull( list );

  list_item_t* current = list->first;
  bosl_token_t* token = current->data;
  ck_assert_int_eq( token->type, TOKEN_IF );
  ck_assert( 2 ==  token->length );
  ck_assert_str_eq( token->start, cmp );
  ck_assert_int_eq( token->line, 1 );

  cmp += 2;
  current = current->next;
  token = current->data;
  ck_assert_int_eq( token->type, TOKEN_EOF );
  ck_assert( 0 ==  token->length );
  ck_assert_str_eq( token->start, cmp );
  ck_assert_int_eq( token->line, 1 );
}
END_TEST

START_TEST( test_scanner_scan_keyword_elseif ) {
  char str[] = "elseif";
  char* cmp = str;
  ck_assert( scanner_init( str ) );
  list_manager_t* list = scanner_scan();
  ck_assert_ptr_nonnull( list );

  list_item_t* current = list->first;
  bosl_token_t* token = current->data;
  ck_assert_int_eq( token->type, TOKEN_ELSEIF );
  ck_assert( 6 ==  token->length );
  ck_assert_str_eq( token->start, cmp );
  ck_assert_int_eq( token->line, 1 );

  cmp += 6;
  current = current->next;
  token = current->data;
  ck_assert_int_eq( token->type, TOKEN_EOF );
  ck_assert( 0 ==  token->length );
  ck_assert_str_eq( token->start, cmp );
  ck_assert_int_eq( token->line, 1 );
}
END_TEST

START_TEST( test_scanner_scan_keyword_else ) {
  char str[] = "else";
  char* cmp = str;
  ck_assert( scanner_init( str ) );
  list_manager_t* list = scanner_scan();
  ck_assert_ptr_nonnull( list );

  list_item_t* current = list->first;
  bosl_token_t* token = current->data;
  ck_assert_int_eq( token->type, TOKEN_ELSE );
  ck_assert( 4 ==  token->length );
  ck_assert_str_eq( token->start, cmp );
  ck_assert_int_eq( token->line, 1 );

  cmp += 4;
  current = current->next;
  token = current->data;
  ck_assert_int_eq( token->type, TOKEN_EOF );
  ck_assert( 0 ==  token->length );
  ck_assert_str_eq( token->start, cmp );
  ck_assert_int_eq( token->line, 1 );
}
END_TEST

START_TEST( test_scanner_scan_keyword_while ) {
  char str[] = "while";
  char* cmp = str;
  ck_assert( scanner_init( str ) );
  list_manager_t* list = scanner_scan();
  ck_assert_ptr_nonnull( list );

  list_item_t* current = list->first;
  bosl_token_t* token = current->data;
  ck_assert_int_eq( token->type, TOKEN_WHILE );
  ck_assert( 5 ==  token->length );
  ck_assert_str_eq( token->start, cmp );
  ck_assert_int_eq( token->line, 1 );

  cmp += 5;
  current = current->next;
  token = current->data;
  ck_assert_int_eq( token->type, TOKEN_EOF );
  ck_assert( 0 ==  token->length );
  ck_assert_str_eq( token->start, cmp );
  ck_assert_int_eq( token->line, 1 );
}
END_TEST

START_TEST( test_scanner_scan_keyword_for ) {
  char str[] = "for";
  char* cmp = str;
  ck_assert( scanner_init( str ) );
  list_manager_t* list = scanner_scan();
  ck_assert_ptr_nonnull( list );

  list_item_t* current = list->first;
  bosl_token_t* token = current->data;
  ck_assert_int_eq( token->type, TOKEN_FOR );
  ck_assert( 3 ==  token->length );
  ck_assert_str_eq( token->start, cmp );
  ck_assert_int_eq( token->line, 1 );

  cmp += 3;
  current = current->next;
  token = current->data;
  ck_assert_int_eq( token->type, TOKEN_EOF );
  ck_assert( 0 ==  token->length );
  ck_assert_str_eq( token->start, cmp );
  ck_assert_int_eq( token->line, 1 );
}
END_TEST

START_TEST( test_scanner_scan_keyword_function ) {
  char str[] = "fn";
  char* cmp = str;
  ck_assert( scanner_init( str ) );
  list_manager_t* list = scanner_scan();
  ck_assert_ptr_nonnull( list );

  list_item_t* current = list->first;
  bosl_token_t* token = current->data;
  ck_assert_int_eq( token->type, TOKEN_FUNCTION );
  ck_assert( 2 ==  token->length );
  ck_assert_str_eq( token->start, cmp );
  ck_assert_int_eq( token->line, 1 );

  cmp += 2;
  current = current->next;
  token = current->data;
  ck_assert_int_eq( token->type, TOKEN_EOF );
  ck_assert( 0 ==  token->length );
  ck_assert_str_eq( token->start, cmp );
  ck_assert_int_eq( token->line, 1 );
}
END_TEST

START_TEST( test_scanner_scan_keyword_return ) {
  char str[] = "return";
  char* cmp = str;
  ck_assert( scanner_init( str ) );
  list_manager_t* list = scanner_scan();
  ck_assert_ptr_nonnull( list );

  list_item_t* current = list->first;
  bosl_token_t* token = current->data;
  ck_assert_int_eq( token->type, TOKEN_RETURN );
  ck_assert( 6 ==  token->length );
  ck_assert_str_eq( token->start, cmp );
  ck_assert_int_eq( token->line, 1 );

  cmp += 6;
  current = current->next;
  token = current->data;
  ck_assert_int_eq( token->type, TOKEN_EOF );
  ck_assert( 0 ==  token->length );
  ck_assert_str_eq( token->start, cmp );
  ck_assert_int_eq( token->line, 1 );
}
END_TEST

START_TEST( test_scanner_load ) {
  char str[] = "load";
  char* cmp = str;
  ck_assert( scanner_init( str ) );
  list_manager_t* list = scanner_scan();
  ck_assert_ptr_nonnull( list );

  list_item_t* current = list->first;
  bosl_token_t* token = current->data;
  ck_assert_int_eq( token->type, TOKEN_LOAD );
  ck_assert( 4 ==  token->length );
  ck_assert_str_eq( token->start, cmp );
  ck_assert_int_eq( token->line, 1 );

  cmp += 4;
  current = current->next;
  token = current->data;
  ck_assert_int_eq( token->type, TOKEN_EOF );
  ck_assert( 0 ==  token->length );
  ck_assert_str_eq( token->start, cmp );
  ck_assert_int_eq( token->line, 1 );
}
END_TEST

START_TEST( test_scanner_scan_builtin_print ) {
  char str[] = "print";
  char* cmp = str;
  ck_assert( scanner_init( str ) );
  list_manager_t* list = scanner_scan();
  ck_assert_ptr_nonnull( list );

  list_item_t* current = list->first;
  bosl_token_t* token = current->data;
  ck_assert_int_eq( token->type, TOKEN_PRINT );
  ck_assert( 5 ==  token->length );
  ck_assert_str_eq( token->start, cmp );
  ck_assert_int_eq( token->line, 1 );

  cmp += 5;
  current = current->next;
  token = current->data;
  ck_assert_int_eq( token->type, TOKEN_EOF );
  ck_assert( 0 ==  token->length );
  ck_assert_str_eq( token->start, cmp );
  ck_assert_int_eq( token->line, 1 );
}
END_TEST

START_TEST( test_scanner_scan_comment_skip ) {
  char str[] = "// asdf";
  char* cmp = str;
  ck_assert( scanner_init( str ) );
  list_manager_t* list = scanner_scan();
  ck_assert_ptr_nonnull( list );

  cmp += 7;
  list_item_t* current = list->first;
  bosl_token_t* token = current->data;
  ck_assert_int_eq( token->type, TOKEN_EOF );
  ck_assert( 0 ==  token->length );
  ck_assert_str_eq( token->start, cmp );
  ck_assert_int_eq( token->line, 1 );
}
END_TEST

START_TEST( test_scanner_scan_type_int8 ) {
  char str[] = "int8";
  char* cmp = str;
  ck_assert( scanner_init( str ) );
  list_manager_t* list = scanner_scan();
  ck_assert_ptr_nonnull( list );

  list_item_t* current = list->first;
  bosl_token_t* token = current->data;
  ck_assert_int_eq( token->type, TOKEN_TYPE_IDENTIFIER );
  ck_assert( 4 ==  token->length );
  ck_assert_str_eq( token->start, cmp );
  ck_assert_int_eq( token->line, 1 );

  cmp += 4;
  current = current->next;
  token = current->data;
  ck_assert_int_eq( token->type, TOKEN_EOF );
  ck_assert( 0 ==  token->length );
  ck_assert_str_eq( token->start, cmp );
  ck_assert_int_eq( token->line, 1 );
}
END_TEST

START_TEST( test_scanner_scan_type_int16 ) {
  char str[] = "int16";
  char* cmp = str;
  ck_assert( scanner_init( str ) );
  list_manager_t* list = scanner_scan();
  ck_assert_ptr_nonnull( list );

  list_item_t* current = list->first;
  bosl_token_t* token = current->data;
  ck_assert_int_eq( token->type, TOKEN_TYPE_IDENTIFIER );
  ck_assert( 5 ==  token->length );
  ck_assert_str_eq( token->start, cmp );
  ck_assert_int_eq( token->line, 1 );

  cmp += 5;
  current = current->next;
  token = current->data;
  ck_assert_int_eq( token->type, TOKEN_EOF );
  ck_assert( 0 ==  token->length );
  ck_assert_str_eq( token->start, cmp );
  ck_assert_int_eq( token->line, 1 );
}
END_TEST

START_TEST( test_scanner_scan_type_int32 ) {
  char str[] = "int32";
  char* cmp = str;
  ck_assert( scanner_init( str ) );
  list_manager_t* list = scanner_scan();
  ck_assert_ptr_nonnull( list );

  list_item_t* current = list->first;
  bosl_token_t* token = current->data;
  ck_assert_int_eq( token->type, TOKEN_TYPE_IDENTIFIER );
  ck_assert( 5 ==  token->length );
  ck_assert_str_eq( token->start, cmp );
  ck_assert_int_eq( token->line, 1 );

  cmp += 5;
  current = current->next;
  token = current->data;
  ck_assert_int_eq( token->type, TOKEN_EOF );
  ck_assert( 0 ==  token->length );
  ck_assert_str_eq( token->start, cmp );
  ck_assert_int_eq( token->line, 1 );
}
END_TEST

START_TEST( test_scanner_scan_type_int64 ) {
  char str[] = "int64";
  char* cmp = str;
  ck_assert( scanner_init( str ) );
  list_manager_t* list = scanner_scan();
  ck_assert_ptr_nonnull( list );

  list_item_t* current = list->first;
  bosl_token_t* token = current->data;
  ck_assert_int_eq( token->type, TOKEN_TYPE_IDENTIFIER );
  ck_assert( 5 ==  token->length );
  ck_assert_str_eq( token->start, cmp );
  ck_assert_int_eq( token->line, 1 );

  cmp += 5;
  current = current->next;
  token = current->data;
  ck_assert_int_eq( token->type, TOKEN_EOF );
  ck_assert( 0 ==  token->length );
  ck_assert_str_eq( token->start, cmp );
  ck_assert_int_eq( token->line, 1 );
}
END_TEST

START_TEST( test_scanner_scan_type_uint8 ) {
  char str[] = "uint8";
  char* cmp = str;
  ck_assert( scanner_init( str ) );
  list_manager_t* list = scanner_scan();
  ck_assert_ptr_nonnull( list );

  list_item_t* current = list->first;
  bosl_token_t* token = current->data;
  ck_assert_int_eq( token->type, TOKEN_TYPE_IDENTIFIER );
  ck_assert( 5 ==  token->length );
  ck_assert_str_eq( token->start, cmp );
  ck_assert_int_eq( token->line, 1 );

  cmp += 5;
  current = current->next;
  token = current->data;
  ck_assert_int_eq( token->type, TOKEN_EOF );
  ck_assert( 0 ==  token->length );
  ck_assert_str_eq( token->start, cmp );
  ck_assert_int_eq( token->line, 1 );
}
END_TEST

START_TEST( test_scanner_scan_type_uint16 ) {
  char str[] = "uint16";
  char* cmp = str;
  ck_assert( scanner_init( str ) );
  list_manager_t* list = scanner_scan();
  ck_assert_ptr_nonnull( list );

  list_item_t* current = list->first;
  bosl_token_t* token = current->data;
  ck_assert_int_eq( token->type, TOKEN_TYPE_IDENTIFIER );
  ck_assert( 6 ==  token->length );
  ck_assert_str_eq( token->start, cmp );
  ck_assert_int_eq( token->line, 1 );

  cmp += 6;
  current = current->next;
  token = current->data;
  ck_assert_int_eq( token->type, TOKEN_EOF );
  ck_assert( 0 ==  token->length );
  ck_assert_str_eq( token->start, cmp );
  ck_assert_int_eq( token->line, 1 );
}
END_TEST

START_TEST( test_scanner_scan_type_uint32 ) {
  char str[] = "uint32";
  char* cmp = str;
  ck_assert( scanner_init( str ) );
  list_manager_t* list = scanner_scan();
  ck_assert_ptr_nonnull( list );

  list_item_t* current = list->first;
  bosl_token_t* token = current->data;
  ck_assert_int_eq( token->type, TOKEN_TYPE_IDENTIFIER );
  ck_assert( 6 ==  token->length );
  ck_assert_str_eq( token->start, cmp );
  ck_assert_int_eq( token->line, 1 );

  cmp += 6;
  current = current->next;
  token = current->data;
  ck_assert_int_eq( token->type, TOKEN_EOF );
  ck_assert( 0 ==  token->length );
  ck_assert_str_eq( token->start, cmp );
  ck_assert_int_eq( token->line, 1 );
}
END_TEST

START_TEST( test_scanner_scan_type_uint64 ) {
  char str[] = "uint64";
  char* cmp = str;
  ck_assert( scanner_init( str ) );
  list_manager_t* list = scanner_scan();
  ck_assert_ptr_nonnull( list );

  list_item_t* current = list->first;
  bosl_token_t* token = current->data;
  ck_assert_int_eq( token->type, TOKEN_TYPE_IDENTIFIER );
  ck_assert( 6 ==  token->length );
  ck_assert_str_eq( token->start, cmp );
  ck_assert_int_eq( token->line, 1 );

  cmp += 6;
  current = current->next;
  token = current->data;
  ck_assert_int_eq( token->type, TOKEN_EOF );
  ck_assert( 0 ==  token->length );
  ck_assert_str_eq( token->start, cmp );
  ck_assert_int_eq( token->line, 1 );
}
END_TEST

START_TEST( test_scanner_scan_type_float ) {
  char str[] = "float";
  char* cmp = str;
  ck_assert( scanner_init( str ) );
  list_manager_t* list = scanner_scan();
  ck_assert_ptr_nonnull( list );

  list_item_t* current = list->first;
  bosl_token_t* token = current->data;
  ck_assert_int_eq( token->type, TOKEN_TYPE_IDENTIFIER );
  ck_assert( 5 ==  token->length );
  ck_assert_str_eq( token->start, cmp );
  ck_assert_int_eq( token->line, 1 );

  cmp += 5;
  current = current->next;
  token = current->data;
  ck_assert_int_eq( token->type, TOKEN_EOF );
  ck_assert( 0 ==  token->length );
  ck_assert_str_eq( token->start, cmp );
  ck_assert_int_eq( token->line, 1 );
}
END_TEST

START_TEST( test_scanner_scan_type_string ) {
  char str[] = "string";
  char* cmp = str;
  ck_assert( scanner_init( str ) );
  list_manager_t* list = scanner_scan();
  ck_assert_ptr_nonnull( list );

  list_item_t* current = list->first;
  bosl_token_t* token = current->data;
  ck_assert_int_eq( token->type, TOKEN_TYPE_IDENTIFIER );
  ck_assert( 6 ==  token->length );
  ck_assert_str_eq( token->start, cmp );
  ck_assert_int_eq( token->line, 1 );

  cmp += 6;
  current = current->next;
  token = current->data;
  ck_assert_int_eq( token->type, TOKEN_EOF );
  ck_assert( 0 ==  token->length );
  ck_assert_str_eq( token->start, cmp );
  ck_assert_int_eq( token->line, 1 );
}
END_TEST

START_TEST( test_scanner_scan_type_void ) {
  char str[] = "void";
  char* cmp = str;
  ck_assert( scanner_init( str ) );
  list_manager_t* list = scanner_scan();
  ck_assert_ptr_nonnull( list );

  list_item_t* current = list->first;
  bosl_token_t* token = current->data;
  ck_assert_int_eq( token->type, TOKEN_TYPE_IDENTIFIER );
  ck_assert( 4 ==  token->length );
  ck_assert_str_eq( token->start, cmp );
  ck_assert_int_eq( token->line, 1 );

  cmp += 4;
  current = current->next;
  token = current->data;
  ck_assert_int_eq( token->type, TOKEN_EOF );
  ck_assert( 0 ==  token->length );
  ck_assert_str_eq( token->start, cmp );
  ck_assert_int_eq( token->line, 1 );
}
END_TEST

START_TEST( test_scanner_scan_type_bool ) {
  char str[] = "bool";
  char* cmp = str;
  ck_assert( scanner_init( str ) );
  list_manager_t* list = scanner_scan();
  ck_assert_ptr_nonnull( list );

  list_item_t* current = list->first;
  bosl_token_t* token = current->data;
  ck_assert_int_eq( token->type, TOKEN_TYPE_IDENTIFIER );
  ck_assert( 4 ==  token->length );
  ck_assert_str_eq( token->start, cmp );
  ck_assert_int_eq( token->line, 1 );

  cmp += 4;
  current = current->next;
  token = current->data;
  ck_assert_int_eq( token->type, TOKEN_EOF );
  ck_assert( 0 ==  token->length );
  ck_assert_str_eq( token->start, cmp );
  ck_assert_int_eq( token->line, 1 );
}
END_TEST

START_TEST( test_scanner_scan_left_parenthesis ) {
  char str[] = "(";
  char* cmp = str;
  ck_assert( scanner_init( str ) );
  list_manager_t* list = scanner_scan();
  ck_assert_ptr_nonnull( list );

  list_item_t* current = list->first;
  bosl_token_t* token = current->data;
  ck_assert_int_eq( token->type, TOKEN_LEFT_PARENTHESIS );
  ck_assert( 1 ==  token->length );
  ck_assert_str_eq( token->start, cmp );
  ck_assert_int_eq( token->line, 1 );

  cmp += 1;
  current = current->next;
  token = current->data;
  ck_assert_int_eq( token->type, TOKEN_EOF );
  ck_assert( 0 ==  token->length );
  ck_assert_str_eq( token->start, cmp );
  ck_assert_int_eq( token->line, 1 );
}
END_TEST

START_TEST( test_scanner_scan_right_parenthesis ) {
  char str[] = ")";
  char* cmp = str;
  ck_assert( scanner_init( str ) );
  list_manager_t* list = scanner_scan();
  ck_assert_ptr_nonnull( list );

  list_item_t* current = list->first;
  bosl_token_t* token = current->data;
  ck_assert_int_eq( token->type, TOKEN_RIGHT_PARENTHESIS );
  ck_assert( 1 ==  token->length );
  ck_assert_str_eq( token->start, cmp );
  ck_assert_int_eq( token->line, 1 );

  cmp += 1;
  current = current->next;
  token = current->data;
  ck_assert_int_eq( token->type, TOKEN_EOF );
  ck_assert( 0 ==  token->length );
  ck_assert_str_eq( token->start, cmp );
  ck_assert_int_eq( token->line, 1 );
}
END_TEST

START_TEST( test_scanner_scan_left_brace ) {
  char str[] = "{";
  char* cmp = str;
  ck_assert( scanner_init( str ) );
  list_manager_t* list = scanner_scan();
  ck_assert_ptr_nonnull( list );

  list_item_t* current = list->first;
  bosl_token_t* token = current->data;
  ck_assert_int_eq( token->type, TOKEN_LEFT_BRACE );
  ck_assert( 1 ==  token->length );
  ck_assert_str_eq( token->start, cmp );
  ck_assert_int_eq( token->line, 1 );

  cmp += 1;
  current = current->next;
  token = current->data;
  ck_assert_int_eq( token->type, TOKEN_EOF );
  ck_assert( 0 ==  token->length );
  ck_assert_str_eq( token->start, cmp );
  ck_assert_int_eq( token->line, 1 );
}
END_TEST

START_TEST( test_scanner_scan_right_brace ) {
  char str[] = "}";
  char* cmp = str;
  ck_assert( scanner_init( str ) );
  list_manager_t* list = scanner_scan();
  ck_assert_ptr_nonnull( list );

  list_item_t* current = list->first;
  bosl_token_t* token = current->data;
  ck_assert_int_eq( token->type, TOKEN_RIGHT_BRACE );
  ck_assert( 1 ==  token->length );
  ck_assert_str_eq( token->start, cmp );
  ck_assert_int_eq( token->line, 1 );

  cmp += 1;
  current = current->next;
  token = current->data;
  ck_assert_int_eq( token->type, TOKEN_EOF );
  ck_assert( 0 ==  token->length );
  ck_assert_str_eq( token->start, cmp );
  ck_assert_int_eq( token->line, 1 );
}
END_TEST

START_TEST( test_scanner_scan_comma ) {
  char str[] = ",";
  char* cmp = str;
  ck_assert( scanner_init( str ) );
  list_manager_t* list = scanner_scan();
  ck_assert_ptr_nonnull( list );

  list_item_t* current = list->first;
  bosl_token_t* token = current->data;
  ck_assert_int_eq( token->type, TOKEN_COMMA );
  ck_assert( 1 ==  token->length );
  ck_assert_str_eq( token->start, cmp );
  ck_assert_int_eq( token->line, 1 );

  cmp += 1;
  current = current->next;
  token = current->data;
  ck_assert_int_eq( token->type, TOKEN_EOF );
  ck_assert( 0 ==  token->length );
  ck_assert_str_eq( token->start, cmp );
  ck_assert_int_eq( token->line, 1 );
}
END_TEST

START_TEST( test_scanner_scan_colon ) {
  char str[] = ":";
  char* cmp = str;
  ck_assert( scanner_init( str ) );
  list_manager_t* list = scanner_scan();
  ck_assert_ptr_nonnull( list );

  list_item_t* current = list->first;
  bosl_token_t* token = current->data;
  ck_assert_int_eq( token->type, TOKEN_COLON );
  ck_assert( 1 ==  token->length );
  ck_assert_str_eq( token->start, cmp );
  ck_assert_int_eq( token->line, 1 );

  cmp += 1;
  current = current->next;
  token = current->data;
  ck_assert_int_eq( token->type, TOKEN_EOF );
  ck_assert( 0 ==  token->length );
  ck_assert_str_eq( token->start, cmp );
  ck_assert_int_eq( token->line, 1 );
}
END_TEST

START_TEST( test_scanner_scan_semicolon ) {
  char str[] = ";";
  char* cmp = str;
  ck_assert( scanner_init( str ) );
  list_manager_t* list = scanner_scan();
  ck_assert_ptr_nonnull( list );

  list_item_t* current = list->first;
  bosl_token_t* token = current->data;
  ck_assert_int_eq( token->type, TOKEN_SEMICOLON );
  ck_assert( 1 ==  token->length );
  ck_assert_str_eq( token->start, cmp );
  ck_assert_int_eq( token->line, 1 );

  cmp += 1;
  current = current->next;
  token = current->data;
  ck_assert_int_eq( token->type, TOKEN_EOF );
  ck_assert( 0 ==  token->length );
  ck_assert_str_eq( token->start, cmp );
  ck_assert_int_eq( token->line, 1 );
}
END_TEST

START_TEST( test_scanner_scan_minus ) {
  char str[] = "-";
  char* cmp = str;
  ck_assert( scanner_init( str ) );
  list_manager_t* list = scanner_scan();
  ck_assert_ptr_nonnull( list );

  list_item_t* current = list->first;
  bosl_token_t* token = current->data;
  ck_assert_int_eq( token->type, TOKEN_MINUS );
  ck_assert( 1 ==  token->length );
  ck_assert_str_eq( token->start, cmp );
  ck_assert_int_eq( token->line, 1 );

  cmp += 1;
  current = current->next;
  token = current->data;
  ck_assert_int_eq( token->type, TOKEN_EOF );
  ck_assert( 0 ==  token->length );
  ck_assert_str_eq( token->start, cmp );
  ck_assert_int_eq( token->line, 1 );
}
END_TEST

START_TEST( test_scanner_scan_plus ) {
  char str[] = "+";
  char* cmp = str;
  ck_assert( scanner_init( str ) );
  list_manager_t* list = scanner_scan();
  ck_assert_ptr_nonnull( list );

  list_item_t* current = list->first;
  bosl_token_t* token = current->data;
  ck_assert_int_eq( token->type, TOKEN_PLUS );
  ck_assert( 1 ==  token->length );
  ck_assert_str_eq( token->start, cmp );
  ck_assert_int_eq( token->line, 1 );

  cmp += 1;
  current = current->next;
  token = current->data;
  ck_assert_int_eq( token->type, TOKEN_EOF );
  ck_assert( 0 ==  token->length );
  ck_assert_str_eq( token->start, cmp );
  ck_assert_int_eq( token->line, 1 );
}
END_TEST

START_TEST( test_scanner_scan_star ) {
  char str[] = "*";
  char* cmp = str;
  ck_assert( scanner_init( str ) );
  list_manager_t* list = scanner_scan();
  ck_assert_ptr_nonnull( list );

  list_item_t* current = list->first;
  bosl_token_t* token = current->data;
  ck_assert_int_eq( token->type, TOKEN_STAR );
  ck_assert( 1 ==  token->length );
  ck_assert_str_eq( token->start, cmp );
  ck_assert_int_eq( token->line, 1 );

  cmp += 1;
  current = current->next;
  token = current->data;
  ck_assert_int_eq( token->type, TOKEN_EOF );
  ck_assert( 0 ==  token->length );
  ck_assert_str_eq( token->start, cmp );
  ck_assert_int_eq( token->line, 1 );
}
END_TEST

START_TEST( test_scanner_scan_modulo ) {
  char str[] = "%";
  char* cmp = str;
  ck_assert( scanner_init( str ) );
  list_manager_t* list = scanner_scan();
  ck_assert_ptr_nonnull( list );

  list_item_t* current = list->first;
  bosl_token_t* token = current->data;
  ck_assert_int_eq( token->type, TOKEN_MODULO );
  ck_assert( 1 ==  token->length );
  ck_assert_str_eq( token->start, cmp );
  ck_assert_int_eq( token->line, 1 );

  cmp += 1;
  current = current->next;
  token = current->data;
  ck_assert_int_eq( token->type, TOKEN_EOF );
  ck_assert( 0 ==  token->length );
  ck_assert_str_eq( token->start, cmp );
  ck_assert_int_eq( token->line, 1 );
}
END_TEST

START_TEST( test_scanner_scan_slash ) {
  char str[] = "/";
  char* cmp = str;
  ck_assert( scanner_init( str ) );
  list_manager_t* list = scanner_scan();
  ck_assert_ptr_nonnull( list );

  list_item_t* current = list->first;
  bosl_token_t* token = current->data;
  ck_assert_int_eq( token->type, TOKEN_SLASH );
  ck_assert( 1 ==  token->length );
  ck_assert_str_eq( token->start, cmp );
  ck_assert_int_eq( token->line, 1 );

  cmp += 1;
  current = current->next;
  token = current->data;
  ck_assert_int_eq( token->type, TOKEN_EOF );
  ck_assert( 0 ==  token->length );
  ck_assert_str_eq( token->start, cmp );
  ck_assert_int_eq( token->line, 1 );
}
END_TEST

START_TEST( test_scanner_scan_xor ) {
  char str[] = "^";
  char* cmp = str;
  ck_assert( scanner_init( str ) );
  list_manager_t* list = scanner_scan();
  ck_assert_ptr_nonnull( list );

  list_item_t* current = list->first;
  bosl_token_t* token = current->data;
  ck_assert_int_eq( token->type, TOKEN_XOR );
  ck_assert( 1 ==  token->length );
  ck_assert_str_eq( token->start, cmp );
  ck_assert_int_eq( token->line, 1 );

  cmp += 1;
  current = current->next;
  token = current->data;
  ck_assert_int_eq( token->type, TOKEN_EOF );
  ck_assert( 0 ==  token->length );
  ck_assert_str_eq( token->start, cmp );
  ck_assert_int_eq( token->line, 1 );
}
END_TEST

START_TEST( test_scanner_scan_binary_one_complement ) {
  char str[] = "~";
  char* cmp = str;
  ck_assert( scanner_init( str ) );
  list_manager_t* list = scanner_scan();
  ck_assert_ptr_nonnull( list );

  list_item_t* current = list->first;
  bosl_token_t* token = current->data;
  ck_assert_int_eq( token->type, TOKEN_BINARY_ONE_COMPLEMENT );
  ck_assert( 1 ==  token->length );
  ck_assert_str_eq( token->start, cmp );
  ck_assert_int_eq( token->line, 1 );

  cmp += 1;
  current = current->next;
  token = current->data;
  ck_assert_int_eq( token->type, TOKEN_EOF );
  ck_assert( 0 ==  token->length );
  ck_assert_str_eq( token->start, cmp );
  ck_assert_int_eq( token->line, 1 );
}
END_TEST

START_TEST( test_scanner_scan_shift_left ) {
  char str[] = "<<";
  char* cmp = str;
  ck_assert( scanner_init( str ) );
  list_manager_t* list = scanner_scan();
  ck_assert_ptr_nonnull( list );

  list_item_t* current = list->first;
  bosl_token_t* token = current->data;
  ck_assert_int_eq( token->type, TOKEN_SHIFT_LEFT );
  ck_assert( 2 ==  token->length );
  ck_assert_str_eq( token->start, cmp );
  ck_assert_int_eq( token->line, 1 );

  cmp += 2;
  current = current->next;
  token = current->data;
  ck_assert_int_eq( token->type, TOKEN_EOF );
  ck_assert( 0 ==  token->length );
  ck_assert_str_eq( token->start, cmp );
  ck_assert_int_eq( token->line, 1 );
}
END_TEST

START_TEST( test_scanner_scan_shift_right ) {
  char str[] = ">>";
  char* cmp = str;
  ck_assert( scanner_init( str ) );
  list_manager_t* list = scanner_scan();
  ck_assert_ptr_nonnull( list );

  list_item_t* current = list->first;
  bosl_token_t* token = current->data;
  ck_assert_int_eq( token->type, TOKEN_SHIFT_RIGHT );
  ck_assert( 2 ==  token->length );
  ck_assert_str_eq( token->start, cmp );
  ck_assert_int_eq( token->line, 1 );

  cmp += 2;
  current = current->next;
  token = current->data;
  ck_assert_int_eq( token->type, TOKEN_EOF );
  ck_assert( 0 ==  token->length );
  ck_assert_str_eq( token->start, cmp );
  ck_assert_int_eq( token->line, 1 );
}
END_TEST

START_TEST( test_scanner_scan_bang ) {
  char str[] = "!";
  char* cmp = str;
  ck_assert( scanner_init( str ) );
  list_manager_t* list = scanner_scan();
  ck_assert_ptr_nonnull( list );

  list_item_t* current = list->first;
  bosl_token_t* token = current->data;
  ck_assert_int_eq( token->type, TOKEN_BANG );
  ck_assert( 1 ==  token->length );
  ck_assert_str_eq( token->start, cmp );
  ck_assert_int_eq( token->line, 1 );

  cmp += 1;
  current = current->next;
  token = current->data;
  ck_assert_int_eq( token->type, TOKEN_EOF );
  ck_assert( 0 ==  token->length );
  ck_assert_str_eq( token->start, cmp );
  ck_assert_int_eq( token->line, 1 );
}
END_TEST

START_TEST( test_scanner_scan_bang_equal ) {
  char str[] = "!=";
  char* cmp = str;
  ck_assert( scanner_init( str ) );
  list_manager_t* list = scanner_scan();
  ck_assert_ptr_nonnull( list );

  list_item_t* current = list->first;
  bosl_token_t* token = current->data;
  ck_assert_int_eq( token->type, TOKEN_BANG_EQUAL );
  ck_assert( 2 ==  token->length );
  ck_assert_str_eq( token->start, cmp );
  ck_assert_int_eq( token->line, 1 );

  cmp += 2;
  current = current->next;
  token = current->data;
  ck_assert_int_eq( token->type, TOKEN_EOF );
  ck_assert( 0 ==  token->length );
  ck_assert_str_eq( token->start, cmp );
  ck_assert_int_eq( token->line, 1 );
}
END_TEST

START_TEST( test_scanner_scan_equal ) {
  char str[] = "=";
  char* cmp = str;
  ck_assert( scanner_init( str ) );
  list_manager_t* list = scanner_scan();
  ck_assert_ptr_nonnull( list );

  list_item_t* current = list->first;
  bosl_token_t* token = current->data;
  ck_assert_int_eq( token->type, TOKEN_EQUAL );
  ck_assert( 1 ==  token->length );
  ck_assert_str_eq( token->start, cmp );
  ck_assert_int_eq( token->line, 1 );

  cmp += 1;
  current = current->next;
  token = current->data;
  ck_assert_int_eq( token->type, TOKEN_EOF );
  ck_assert( 0 ==  token->length );
  ck_assert_str_eq( token->start, cmp );
  ck_assert_int_eq( token->line, 1 );
}
END_TEST

START_TEST( test_scanner_scan_equal_equal ) {
  char str[] = "==";
  char* cmp = str;
  ck_assert( scanner_init( str ) );
  list_manager_t* list = scanner_scan();
  ck_assert_ptr_nonnull( list );

  list_item_t* current = list->first;
  bosl_token_t* token = current->data;
  ck_assert_int_eq( token->type, TOKEN_EQUAL_EQUAL );
  ck_assert( 2 ==  token->length );
  ck_assert_str_eq( token->start, cmp );
  ck_assert_int_eq( token->line, 1 );

  cmp += 2;
  current = current->next;
  token = current->data;
  ck_assert_int_eq( token->type, TOKEN_EOF );
  ck_assert( 0 ==  token->length );
  ck_assert_str_eq( token->start, cmp );
  ck_assert_int_eq( token->line, 1 );
}
END_TEST

START_TEST( test_scanner_scan_greater ) {
  char str[] = ">";
  char* cmp = str;
  ck_assert( scanner_init( str ) );
  list_manager_t* list = scanner_scan();
  ck_assert_ptr_nonnull( list );

  list_item_t* current = list->first;
  bosl_token_t* token = current->data;
  ck_assert_int_eq( token->type, TOKEN_GREATER );
  ck_assert( 1 ==  token->length );
  ck_assert_str_eq( token->start, cmp );
  ck_assert_int_eq( token->line, 1 );

  cmp += 1;
  current = current->next;
  token = current->data;
  ck_assert_int_eq( token->type, TOKEN_EOF );
  ck_assert( 0 ==  token->length );
  ck_assert_str_eq( token->start, cmp );
  ck_assert_int_eq( token->line, 1 );
}
END_TEST

START_TEST( test_scanner_scan_greater_equal ) {
  char str[] = ">=";
  char* cmp = str;
  ck_assert( scanner_init( str ) );
  list_manager_t* list = scanner_scan();
  ck_assert_ptr_nonnull( list );

  list_item_t* current = list->first;
  bosl_token_t* token = current->data;
  ck_assert_int_eq( token->type, TOKEN_GREATER_EQUAL );
  ck_assert( 2 ==  token->length );
  ck_assert_str_eq( token->start, cmp );
  ck_assert_int_eq( token->line, 1 );

  cmp += 2;
  current = current->next;
  token = current->data;
  ck_assert_int_eq( token->type, TOKEN_EOF );
  ck_assert( 0 ==  token->length );
  ck_assert_str_eq( token->start, cmp );
  ck_assert_int_eq( token->line, 1 );
}
END_TEST

START_TEST( test_scanner_scan_less ) {
  char str[] = "<";
  char* cmp = str;
  ck_assert( scanner_init( str ) );
  list_manager_t* list = scanner_scan();
  ck_assert_ptr_nonnull( list );

  list_item_t* current = list->first;
  bosl_token_t* token = current->data;
  ck_assert_int_eq( token->type, TOKEN_LESS );
  ck_assert( 1 ==  token->length );
  ck_assert_str_eq( token->start, cmp );
  ck_assert_int_eq( token->line, 1 );

  cmp += 1;
  current = current->next;
  token = current->data;
  ck_assert_int_eq( token->type, TOKEN_EOF );
  ck_assert( 0 ==  token->length );
  ck_assert_str_eq( token->start, cmp );
  ck_assert_int_eq( token->line, 1 );
}
END_TEST

START_TEST( test_scanner_scan_less_equal ) {
  char str[] = "<=";
  char* cmp = str;
  ck_assert( scanner_init( str ) );
  list_manager_t* list = scanner_scan();
  ck_assert_ptr_nonnull( list );

  list_item_t* current = list->first;
  bosl_token_t* token = current->data;
  ck_assert_int_eq( token->type, TOKEN_LESS_EQUAL );
  ck_assert( 2 ==  token->length );
  ck_assert_str_eq( token->start, cmp );
  ck_assert_int_eq( token->line, 1 );

  cmp += 2;
  current = current->next;
  token = current->data;
  ck_assert_int_eq( token->type, TOKEN_EOF );
  ck_assert( 0 ==  token->length );
  ck_assert_str_eq( token->start, cmp );
  ck_assert_int_eq( token->line, 1 );
}
END_TEST

START_TEST( test_scanner_scan_and ) {
  char str[] = "&";
  char* cmp = str;
  ck_assert( scanner_init( str ) );
  list_manager_t* list = scanner_scan();
  ck_assert_ptr_nonnull( list );

  list_item_t* current = list->first;
  bosl_token_t* token = current->data;
  ck_assert_int_eq( token->type, TOKEN_AND );
  ck_assert( 1 ==  token->length );
  ck_assert_str_eq( token->start, cmp );
  ck_assert_int_eq( token->line, 1 );

  cmp += 1;
  current = current->next;
  token = current->data;
  ck_assert_int_eq( token->type, TOKEN_EOF );
  ck_assert( 0 ==  token->length );
  ck_assert_str_eq( token->start, cmp );
  ck_assert_int_eq( token->line, 1 );
}
END_TEST

START_TEST( test_scanner_scan_and_and ) {
  char str[] = "&&";
  char* cmp = str;
  ck_assert( scanner_init( str ) );
  list_manager_t* list = scanner_scan();
  ck_assert_ptr_nonnull( list );

  list_item_t* current = list->first;
  bosl_token_t* token = current->data;
  ck_assert_int_eq( token->type, TOKEN_AND_AND );
  ck_assert( 2 ==  token->length );
  ck_assert_str_eq( token->start, cmp );
  ck_assert_int_eq( token->line, 1 );

  cmp += 2;
  current = current->next;
  token = current->data;
  ck_assert_int_eq( token->type, TOKEN_EOF );
  ck_assert( 0 ==  token->length );
  ck_assert_str_eq( token->start, cmp );
  ck_assert_int_eq( token->line, 1 );
}
END_TEST

START_TEST( test_scanner_scan_or ) {
  char str[] = "|";
  char* cmp = str;
  ck_assert( scanner_init( str ) );
  list_manager_t* list = scanner_scan();
  ck_assert_ptr_nonnull( list );

  list_item_t* current = list->first;
  bosl_token_t* token = current->data;
  ck_assert_int_eq( token->type, TOKEN_OR );
  ck_assert( 1 ==  token->length );
  ck_assert_str_eq( token->start, cmp );
  ck_assert_int_eq( token->line, 1 );

  cmp += 1;
  current = current->next;
  token = current->data;
  ck_assert_int_eq( token->type, TOKEN_EOF );
  ck_assert( 0 ==  token->length );
  ck_assert_str_eq( token->start, cmp );
  ck_assert_int_eq( token->line, 1 );
}
END_TEST

START_TEST( test_scanner_scan_or_or ) {
  char str[] = "||";
  char* cmp = str;
  ck_assert( scanner_init( str ) );
  list_manager_t* list = scanner_scan();
  ck_assert_ptr_nonnull( list );

  list_item_t* current = list->first;
  bosl_token_t* token = current->data;
  ck_assert_int_eq( token->type, TOKEN_OR_OR );
  ck_assert( 2 ==  token->length );
  ck_assert_str_eq( token->start, cmp );
  ck_assert_int_eq( token->line, 1 );

  cmp += 2;
  current = current->next;
  token = current->data;
  ck_assert_int_eq( token->type, TOKEN_EOF );
  ck_assert( 0 ==  token->length );
  ck_assert_str_eq( token->start, cmp );
  ck_assert_int_eq( token->line, 1 );
}
END_TEST

START_TEST( test_scanner_scan_invalid_character ) {
  char str[] = "$";
  char* cmp = str;
  char error_cmp[] = "Unknown token";
  ck_assert( scanner_init( str ) );
  list_manager_t* list = scanner_scan();
  ck_assert_ptr_nonnull( list );

  list_item_t* current = list->first;
  bosl_token_t* token = current->data;
  ck_assert_int_eq( token->type, TOKEN_ERROR );
  ck_assert( 13 ==  token->length );
  ck_assert_str_eq( token->start, error_cmp );
  ck_assert_int_eq( token->line, 1 );

  cmp += 1;
  current = current->next;
  token = current->data;
  ck_assert_int_eq( token->type, TOKEN_EOF );
  ck_assert( 0 ==  token->length );
  ck_assert_str_eq( token->start, cmp );
  ck_assert_int_eq( token->line, 1 );
}
END_TEST

START_TEST( test_scanner_scan_with_newline ) {
  char str[] = "\n\
let";
  char* cmp = str;
  ck_assert( scanner_init( str ) );
  list_manager_t* list = scanner_scan();
  ck_assert_ptr_nonnull( list );

  cmp += 1;
  list_item_t* current = list->first;
  bosl_token_t* token = current->data;
  ck_assert_int_eq( token->type, TOKEN_LET );
  ck_assert( 3 ==  token->length );
  ck_assert_str_eq( token->start, cmp );
  ck_assert_int_eq( token->line, 2 );

  cmp += 3;
  current = current->next;
  token = current->data;
  ck_assert_int_eq( token->type, TOKEN_EOF );
  ck_assert( 0 ==  token->length );
  ck_assert_str_eq( token->start, cmp );
  ck_assert_int_eq( token->line, 2 );
}
END_TEST

START_TEST( test_scanner_scan_white_space_skip_space ) {
  char str[] = " let";
  char* cmp = str;
  ck_assert( scanner_init( str ) );
  list_manager_t* list = scanner_scan();
  ck_assert_ptr_nonnull( list );

  cmp += 1;
  list_item_t* current = list->first;
  bosl_token_t* token = current->data;
  ck_assert_int_eq( token->type, TOKEN_LET );
  ck_assert( 3 ==  token->length );
  ck_assert_str_eq( token->start, cmp );
  ck_assert_int_eq( token->line, 1 );

  cmp += 3;
  current = current->next;
  token = current->data;
  ck_assert_int_eq( token->type, TOKEN_EOF );
  ck_assert( 0 ==  token->length );
  ck_assert_str_eq( token->start, cmp );
  ck_assert_int_eq( token->line, 1 );
}
END_TEST

START_TEST( test_scanner_scan_white_space_skip_tab ) {
  char str[] = "\tlet";
  char* cmp = str;
  ck_assert( scanner_init( str ) );
  list_manager_t* list = scanner_scan();
  ck_assert_ptr_nonnull( list );

  cmp += 1;
  list_item_t* current = list->first;
  bosl_token_t* token = current->data;
  ck_assert_int_eq( token->type, TOKEN_LET );
  ck_assert( 3 ==  token->length );
  ck_assert_str_eq( token->start, cmp );
  ck_assert_int_eq( token->line, 1 );

  cmp += 3;
  current = current->next;
  token = current->data;
  ck_assert_int_eq( token->type, TOKEN_EOF );
  ck_assert( 0 ==  token->length );
  ck_assert_str_eq( token->start, cmp );
  ck_assert_int_eq( token->line, 1 );
}
END_TEST

START_TEST( test_scanner_scan_white_space_skip_carriage_return ) {
  char str[] = "\rlet";
  char* cmp = str;
  ck_assert( scanner_init( str ) );
  list_manager_t* list = scanner_scan();
  ck_assert_ptr_nonnull( list );

  cmp += 1;
  list_item_t* current = list->first;
  bosl_token_t* token = current->data;
  ck_assert_int_eq( token->type, TOKEN_LET );
  ck_assert( 3 ==  token->length );
  ck_assert_str_eq( token->start, cmp );
  ck_assert_int_eq( token->line, 1 );

  cmp += 3;
  current = current->next;
  token = current->data;
  ck_assert_int_eq( token->type, TOKEN_EOF );
  ck_assert( 0 ==  token->length );
  ck_assert_str_eq( token->start, cmp );
  ck_assert_int_eq( token->line, 1 );
}
END_TEST

static Suite* scanner_suite( void ) {
  Suite* s;
  TCase* tc_core;

  s = suite_create( "libbosl" );
  // test cases
  tc_core = tcase_create( "scanner" );
  // add tests
  tcase_add_checked_fixture( tc_core, setup, teardown );
  tcase_add_test( tc_core, test_scanner_init );
  // literals
  tcase_add_test( tc_core, test_scanner_scan_string_single_line );
  tcase_add_test( tc_core, test_scanner_scan_string_multi_line );
  tcase_add_test( tc_core, test_scanner_scan_string_invalid );
  tcase_add_test( tc_core, test_scanner_scan_int );
  tcase_add_test( tc_core, test_scanner_scan_float );
  tcase_add_test( tc_core, test_scanner_scan_hex_lower );
  tcase_add_test( tc_core, test_scanner_scan_hex_upper );
  tcase_add_test( tc_core, test_scanner_scan_hex_mixed_lower_x );
  tcase_add_test( tc_core, test_scanner_scan_hex_mixed_upper_x );
  tcase_add_test( tc_core, test_scanner_scan_literal_keyword_within );
  tcase_add_test( tc_core, test_scanner_scan_literal_no_keyword_within );
  // keywords
  tcase_add_test( tc_core, test_scanner_scan_keyword_let );
  tcase_add_test( tc_core, test_scanner_scan_keyword_const );
  tcase_add_test( tc_core, test_scanner_scan_keyword_pointer );
  tcase_add_test( tc_core, test_scanner_scan_keyword_true );
  tcase_add_test( tc_core, test_scanner_scan_keyword_false );
  tcase_add_test( tc_core, test_scanner_scan_keyword_null );
  tcase_add_test( tc_core, test_scanner_scan_keyword_if );
  tcase_add_test( tc_core, test_scanner_scan_keyword_elseif );
  tcase_add_test( tc_core, test_scanner_scan_keyword_else );
  tcase_add_test( tc_core, test_scanner_scan_keyword_while );
  tcase_add_test( tc_core, test_scanner_scan_keyword_for );
  tcase_add_test( tc_core, test_scanner_scan_keyword_function );
  tcase_add_test( tc_core, test_scanner_scan_keyword_return );
  tcase_add_test( tc_core, test_scanner_load );
  // data types
  tcase_add_test( tc_core, test_scanner_scan_type_int8 );
  tcase_add_test( tc_core, test_scanner_scan_type_int16 );
  tcase_add_test( tc_core, test_scanner_scan_type_int32 );
  tcase_add_test( tc_core, test_scanner_scan_type_int64 );
  tcase_add_test( tc_core, test_scanner_scan_type_uint8 );
  tcase_add_test( tc_core, test_scanner_scan_type_uint16 );
  tcase_add_test( tc_core, test_scanner_scan_type_uint32 );
  tcase_add_test( tc_core, test_scanner_scan_type_uint64 );
  tcase_add_test( tc_core, test_scanner_scan_type_float );
  tcase_add_test( tc_core, test_scanner_scan_type_string );
  tcase_add_test( tc_core, test_scanner_scan_type_void );
  tcase_add_test( tc_core, test_scanner_scan_type_bool );
  // single character tokens
  tcase_add_test( tc_core, test_scanner_scan_left_parenthesis );
  tcase_add_test( tc_core, test_scanner_scan_right_parenthesis );
  tcase_add_test( tc_core, test_scanner_scan_left_brace );
  tcase_add_test( tc_core, test_scanner_scan_right_brace );
  tcase_add_test( tc_core, test_scanner_scan_comma );
  tcase_add_test( tc_core, test_scanner_scan_colon );
  tcase_add_test( tc_core, test_scanner_scan_semicolon );
  tcase_add_test( tc_core, test_scanner_scan_minus );
  tcase_add_test( tc_core, test_scanner_scan_plus );
  tcase_add_test( tc_core, test_scanner_scan_star );
  tcase_add_test( tc_core, test_scanner_scan_modulo );
  tcase_add_test( tc_core, test_scanner_scan_slash );
  tcase_add_test( tc_core, test_scanner_scan_xor );
  tcase_add_test( tc_core, test_scanner_scan_binary_one_complement );
  // one or two character tokens
  tcase_add_test( tc_core, test_scanner_scan_bang );
  tcase_add_test( tc_core, test_scanner_scan_bang_equal );
  tcase_add_test( tc_core, test_scanner_scan_equal );
  tcase_add_test( tc_core, test_scanner_scan_equal_equal );
  tcase_add_test( tc_core, test_scanner_scan_greater );
  tcase_add_test( tc_core, test_scanner_scan_greater_equal );
  tcase_add_test( tc_core, test_scanner_scan_less );
  tcase_add_test( tc_core, test_scanner_scan_less_equal );
  tcase_add_test( tc_core, test_scanner_scan_and );
  tcase_add_test( tc_core, test_scanner_scan_and_and );
  tcase_add_test( tc_core, test_scanner_scan_or );
  tcase_add_test( tc_core, test_scanner_scan_or_or );
  // two character tokens
  tcase_add_test( tc_core, test_scanner_scan_shift_left );
  tcase_add_test( tc_core, test_scanner_scan_shift_right );
  // built-in functions
  tcase_add_test( tc_core, test_scanner_scan_builtin_print );
  // whitespace ignore
  tcase_add_test( tc_core, test_scanner_scan_white_space_skip_space );
  tcase_add_test( tc_core, test_scanner_scan_white_space_skip_tab );
  tcase_add_test( tc_core, test_scanner_scan_white_space_skip_carriage_return );
  // custom
  tcase_add_test( tc_core, test_scanner_scan_comment_skip );
  tcase_add_test( tc_core, test_scanner_scan_invalid_character );
  tcase_add_test( tc_core, test_scanner_scan_with_newline );
  suite_add_tcase( s, tc_core );
  // return suite
  return s;
}

int main( void ) {
  int number_failed;
  Suite *s;
  SRunner *sr;

  s = scanner_suite();
  sr = srunner_create( s );

  srunner_run_all( sr, CK_NORMAL );
  number_failed = srunner_ntests_failed( sr );
  srunner_free( sr );
  return ( 0 == number_failed ) ? EXIT_SUCCESS : EXIT_FAILURE;
}
