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
#include <string.h>
#include <argtable2.h>
#include "../library/lib/error.h"
#include "../library/lib/scanner.h"
#include "../library/lib/parser.h"
#include "../library/lib/interpreter.h"

/**
 * @brief Helper to read file content
 *
 * @param path
 * @return
 */
static char* read_file( const char* path ) {
  // open script
  FILE* f = fopen( path, "r" );
  if ( ! f ) {
    perror( path );
    return NULL;
  }
  // switch to end, get size and rewind to beginning
  fseek( f, 0, SEEK_END );
  // get position
  long l_size = ftell( f );
  if ( -1L == l_size ) {
    fclose( f );
    return NULL;
  }
  size_t size = ( size_t )l_size;
  // rewind to begin
  rewind( f );
  // allocate memory
  char* buffer = malloc( sizeof( char ) * size );
  if ( ! buffer ) {
    fclose( f );
    return NULL;
  }
  // clearout
  memset( buffer, 0, sizeof( char ) * size );
  // read whole file into buffer
  if ( 1 != fread( buffer, size, 1, f ) ) {
    fclose( f );
    return NULL;
  }
  // close and return buffer
  fclose( f );
  return buffer;
}

/**
 * @brief Interprete buffer
 *
 * @param print_ast print ast instead of interpreting
 * @param buffer code to interprete
 * @return
 */
static bool interprete( bool print_ast, char* buffer ) {
  // initialize scanner
  if ( ! bosl_scanner_init( buffer ) ) {
    fprintf( stderr, "Unable to init scanner!\r\n" );
    return false;
  }
  // scan token
  list_manager_t* token_list = bosl_scanner_scan();
  if ( ! token_list ) {
    bosl_scanner_free();
    return false;
  }
  // init parser
  if ( ! bosl_parser_init( token_list ) ) {
    bosl_scanner_free();
    return false;
  }
  // parse ast
  list_manager_t* ast_list = bosl_parser_scan();
  if ( ! ast_list ) {
    bosl_parser_free();
    bosl_scanner_free();
    return false;
  }
  // setup interpreter
  if ( ! bosl_interpreter_init( ast_list ) ) {
    bosl_parser_free();
    bosl_scanner_free();
    return false;
  }

  if ( print_ast ) {
    // print ast
    bosl_parser_print();
  } else {
    // run code
    if ( ! bosl_interpreter_run() ) {
      bosl_interpreter_free();
      bosl_parser_free();
      bosl_scanner_free();
      return false;
    }
  }

  // destroy parser, scanner and interpreter
  bosl_parser_free();
  bosl_scanner_free();
  bosl_interpreter_free();
  // return success
  return true;
}

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
  // read file into buffer
  char* buffer = read_file( infile->filename[ 0 ] );
  if ( ! buffer ) {
    arg_freetable( argtable,sizeof( argtable ) / sizeof( argtable[ 0 ] ) );
    return EXIT_FAILURE;
  }
  // interprete it
  if ( ! interprete( ast->count, buffer ) ) {
    arg_freetable( argtable,sizeof( argtable ) / sizeof( argtable[ 0 ] ) );
    return EXIT_FAILURE;
  }
  // free buffer and free argtable
  free( buffer );
  arg_freetable( argtable,sizeof( argtable ) / sizeof( argtable[ 0 ] ) );
  return EXIT_SUCCESS;
}
