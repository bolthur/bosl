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

#include "type-check.h"

/**
 * @brief Initialize type checking
 *
 * @param ast
 * @return
 */
bool bosl_check_init( __unused list_manager_t* ast ) {
  return true;
}

/**
 * @brief Free type checking
 */
void bosl_check_free( void ) {
}

/**
 * @brief Perform type checking
 *
 * @return
 */
bool bosl_check_types( void ) {
  return true;
}
