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

#include <stdbool.h>

#if defined( _COMPILING_BOSL )
  #include "collection/hashmap.h"
  #include "object.h"
#else
  #include <bosl/collection/hashmap.h>
  #include <bosl/object.h>
#endif

#if ! defined( _BOSL_BINDING_H )
#define _BOSL_BINDING_H

bool bosl_binding_init( void );
void bosl_binding_free( void );
bool bosl_binding_bind_function( const char*, bosl_callback_t );
bool bosl_binding_unbind_function( const char* );
bosl_object_t* bosl_binding_get( const char* );
bosl_object_t* bosl_binding_nget( const char*, size_t );

bosl_object_t* bosl_binding_build_return_uint( bosl_object_type_t, uint64_t );
bosl_object_t* bosl_binding_build_return_int( bosl_object_type_t, int64_t );
bosl_object_t* bosl_binding_build_return_float( long double );
bosl_object_t* bosl_binding_build_return_string( const char* );
bosl_object_t* bosl_binding_build_return_bool( bool );

#endif
