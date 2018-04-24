
/* defValues.h
 * Copyright (C) 2011, Antonio Fariña, all rights reserved.
 *
 * defValues.h: Some default constants used in the WCSA.
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
 
#ifndef DEFVALUES_INCLUDED
#define DEFVALUES_INCLUDED



#define DEFAULT_SUFFIX_ARRAY_SIZE 50000000





//for queries
#define MAX_INTEGER_PATTERN_SIZE 20

#ifndef DEBUG_ON
	// #define DEBUG_ON
#endif

/*
#ifndef byte
#define byte unsigned char
#endif
*/

#define CSA_ON				//generates the CSA or only "presentation layer"

// Extensions of created  files

#define VOCABULARY_WORDS_FILE_EXT "words"
#define SE_FILE_EXT "se"
#define CONSTANTS_FILE_EXT "cte"
#define TIMES_FILE_EXT "times"
#define WEEKS_FILE_EXT "weeks"

#endif
