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

#include "error.h"
#include "scanner.h"
#include "stdarg.h"
#include <stdio.h>

/**
 * @brief Method to raise error
 *
 * @param token
 * @param message
 */
#if defined( __linux__ ) || defined( __bolthur__ )
__weak // weak reference only for linux and bolthur
#endif
void bosl_error_raise( bosl_token_t* token, const char* message, ... ) {
  // variables
  va_list parameter;
  // variable arguments
  va_start( parameter, message );
  // set font color
  fprintf( stderr, "\033[0;91m" );
  // token information
  if ( token ) {
    // start error output
    fprintf( stderr, "[line %u] Error", token->line );
    // position / token information
    if ( TOKEN_EOF == token->type ) {
      fprintf( stderr, " at end: " );
    } else if ( TOKEN_ERROR != token->type ) {
      fprintf( stderr, " at '%.*s': ", ( int )token->length, token->start );
    } else {
      fprintf( stderr, ": " );
    }
  }
  // error message itself
  vfprintf( stderr, message, parameter );
  // reset font color
  fprintf( stderr, "\r\n\033[0m" );
  // cleanup parameter
  va_end( parameter );
}
