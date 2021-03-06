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

#include <stdint.h>
#include <stdio.h>

#if defined( _COMPILING_BOSL )
#include "scanner.h"
#else
#include <bosl/scanner.h>
#endif

#if !defined( BOSL_ERROR_H )
#define BOSL_ERROR_H

#ifdef __cplusplus
extern "C" {
#endif

void bosl_error_raise( bosl_token_t*, const char*, ... ) __attribute__( ( format( printf, 2, 3 ) ) );

#ifdef __cplusplus
}
#endif

#endif
