
/* parameters.h
 * Copyright (C) 2011, Antonio Fariña, all rights reserved.
 *
 * parameters.h: Funtions to parse a string "var1=val1; var2=val2; ... ;varn=valn"
 *   into the different variables and values.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */


#include <string.h>
#include <stdlib.h>
/***********************************************************************************/
/*** FUNCTIONS USED FOR PARSING PARAMETERS FROM COMMAND LINE ***********************/

#ifndef PARAMETERS_INCLUDED
#define PARAMETERS_INCLUDED
 bool is_delimeter(char *delimiters, char c) ;
 void parse_parameters(char *options, int *num_parameters, char ***parameters, char *delimiters);
 void free_parameters(int num_parameters,char ***parameters);
#endif
