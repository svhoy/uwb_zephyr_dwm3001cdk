/**********************************************************************************
 * 
 *  Copyright (C) 2023  Sven Hoyer
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.

 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.

 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <https://www.gnu.org/licenses/>.
 * 
***********************************************************************************/

/** 
 * @file sit_diagnostic.h
 * @author Sven Hoyer (svhoy)
 * @date 17.04.2023
 * @brief Defniton of functions for distance measurement.
 *
 * This file defines functions for distance measurement for the 
 * DWM3001cdk in the SIT system. 
 *
 *  
 * @bug No known bugs.
 * @todo everything 
 */

#ifndef __SIT_DIAGNOSTIC_H__
#define __SIT_DIAGNOSTIC_H__

#include "sit_config.h"
#include <stdint.h>


void get_fp_pp_index(void);

void get_diagnostic(diagnostic_info *diagnostic);

#endif // __SIT_DIAGNOSTIC_H__
