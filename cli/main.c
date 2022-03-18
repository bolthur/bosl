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

#include <stdio.h>
#include <stdlib.h>
#include <argtable2.h>
#include "../library/lib/error.h"
#include "../library/lib/scanner.h"
#include "../library/lib/parser.h"

int main( int argc, char* argv[] ) {
  struct arg_lit* verbose = arg_lit0( "v", "verbose", "verbose output" );
  struct arg_lit* help = arg_lit0( "h", "help", "print help" );
  struct arg_lit* version = arg_lit0( NULL, "version", "print version" );
  struct arg_lit* ast = arg_lit0( "a", "ast", "print ast" );
  struct arg_file* infile = arg_filen( NULL, NULL, NULL, 1, 1, "input file" );
  struct arg_end* end = arg_end( 20 );
  void* argtable[] = { verbose, help, version, ast, infile, end, };
  int nerrors;

  // verify argtable entries have been allocated
  if ( arg_nullcheck( argtable ) ) {
    fprintf( stderr, "%s: insufficient memory\r\n", PACKAGE_NAME );
    arg_freetable( argtable,sizeof( argtable ) / sizeof( argtable[ 0 ] ) );
    return EXIT_FAILURE;
  }
  // try to parse arguments
  nerrors = arg_parse( argc, argv, argtable );

  // handle --help
  if ( help->count ) {
    fprintf( stdout, "Usage: %s", PACKAGE_NAME );
    arg_print_syntax( stdout, argtable, "\r\n" );
    arg_print_glossary( stdout, argtable, "  %-25s %s\n" );
    arg_freetable( argtable,sizeof( argtable ) / sizeof( argtable[ 0 ] ) );
    return EXIT_SUCCESS;
  }
  // handle --version
  if ( version->count ) {
    fprintf(
      stdout,
      "%s\r\nCopyright (C) 2022 bolthur project.\r\nThis is free software; see the "
      "source for copying conditions.  There is NO\r\nwarranty; not even for MERCHANTABILITY "
      "or FITNESS FOR A PARTICULAR PURPOSE.\r\n",
      PACKAGE_STRING
    );
    arg_freetable( argtable,sizeof( argtable ) / sizeof( argtable[ 0 ] ) );
    return EXIT_SUCCESS;
  }

  // check for errors
  if ( nerrors ) {
    arg_print_errors( stderr, end, PACKAGE_NAME );
    fprintf( stderr, "Try '%s --help' for more information.\r\n",PACKAGE_NAME );
    arg_freetable( argtable,sizeof( argtable ) / sizeof( argtable[ 0 ] ) );
    return EXIT_FAILURE;
  }
  // without arguments print help
  if ( 1 == argc ) {
    fprintf( stdout, "Try '%s --help' for more information.\r\n", PACKAGE_NAME );
    arg_freetable( argtable,sizeof( argtable ) / sizeof( argtable[ 0 ] ) );
    return EXIT_SUCCESS;
  }

  arg_freetable( argtable,sizeof( argtable ) / sizeof( argtable[ 0 ] ) );
  return EXIT_SUCCESS;



  ( void )argv;
  if ( 2 != argc ) {
    printf( "Usage: bosl [script]\r\n" );
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}
