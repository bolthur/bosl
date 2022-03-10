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

/**
 * @fn void bosl_error_raise(uint32_t, const char*)
 * @brief Method to raise an error
 *
 * @param line
 * @param message
 */
void bosl_error_raise(
  __unused uint32_t line,
  __unused const char* message
) {
}

/**
 * @fn void bosl_error_report(uint32_t, const char*, const char*)
 * @brief Function to print error which can be overwritten
 *
 * @param line
 * @param source
 * @param message
 */
__weak void bosl_error_report(
  __unused uint32_t line,
  __unused const char* source,
  __unused const char* message
) {
}
