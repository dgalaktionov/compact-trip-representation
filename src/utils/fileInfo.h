
/* fileInfo.h
 * Copyright (C) 2011, Antonio Fariña, all rights reserved.
 *
 * fileInfo.h: Some basic file functions: fileSize and copyfile.
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

#ifndef FILE_INFO_INCLUDED
#define FILE_INFO_INCLUDED

#include <stdio.h>
#include <stdlib.h>
 #include <unistd.h>



	/*------------------------------------------------------------------
	 Obtains the size of a file.
	------------------------------------------------------------------ */
	unsigned long fileSize (char *filename);
	
	/* copies from infile to outfile */
	void copyFile (char *infile, char *outfile);

#endif

