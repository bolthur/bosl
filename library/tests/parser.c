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
#include "../lib/ast/common.h"
#include "../lib/ast/statement.h"
#include "../lib/ast/expression.h"
#include "../lib/scanner.h"
#include "../lib/parser.h"

static void setup( void ) {
}

static void teardown( void ) {
  // destroy scanner and parser
  bosl_scanner_free();
  bosl_parser_free();
}

START_TEST( test_simple_expression ) {
  const char expression[] = "3 + 2 * 7;";
  // init scanner
  ck_assert( bosl_scanner_init( expression ) );
  // parse token
  list_manager_t* token = bosl_scanner_scan();
  ck_assert_ptr_nonnull( token );
  // init parser
  ck_assert( bosl_parser_init( token ) );
  // parse ast
  list_manager_t* ast = bosl_parser_scan();
  ck_assert_ptr_nonnull( ast );
  // check tokens
  list_item_t* current = ast->first;
  bosl_ast_node_t* n;
  // token should be expression statement ( + 3 ( * 2 7 ) )
  // get node
  n = current->data;
  ck_assert( n->type == NODE_STATEMENT );
  // check statement and expression
  ck_assert( n->statement->type == STATEMENT_EXPRESSION );
  ck_assert( n->statement->expression->expression->type == EXPRESSION_BINARY );
  // get expression
  bosl_ast_expression_t* e = n->statement->expression->expression;
  // left has to be a literal and operator '+'
  ck_assert( e->binary->left->type == EXPRESSION_LITERAL );
  ck_assert( e->binary->operator->type == TOKEN_PLUS );
  // right is another expression ( * 2 7 )
  ck_assert( e->binary->right->type == EXPRESSION_BINARY );
  ck_assert( e->binary->right->binary->left->type == EXPRESSION_LITERAL );
  ck_assert( e->binary->right->binary->operator->type == TOKEN_STAR );
  ck_assert( e->binary->right->binary->right->type == EXPRESSION_LITERAL );
}
END_TEST

static Suite* parser_suite( void ) {
  Suite* s;
  TCase* tc_core;

  s = suite_create( "libbosl" );
  // test cases
  tc_core = tcase_create( "parser" );
  // add tests
  tcase_add_checked_fixture( tc_core, setup, teardown );
  tcase_add_test( tc_core, test_simple_expression );
  suite_add_tcase( s, tc_core );
  // return suite
  return s;
}

int main( void ) {
  int number_failed;
  Suite *s;
  SRunner *sr;

  s = parser_suite();
  sr = srunner_create( s );

  srunner_run_all( sr, CK_NORMAL );
  number_failed = srunner_ntests_failed( sr );
  srunner_free( sr );
  return ( 0 == number_failed ) ? EXIT_SUCCESS : EXIT_FAILURE;
}
