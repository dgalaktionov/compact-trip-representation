
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


#include "fileInfo.h"

unsigned long fileSize (char *filename){
	FILE *fpText;
	unsigned long fsize;
	fpText = fopen(filename,"rb");
	fsize=0;
	if (fpText) {
		ssize_t seek_err= fseek(fpText,0,2);
		fsize= ftell(fpText);
		fclose(fpText);
		////fprintf(stderr,"fileSize = %ld",fsize);
	}
	return fsize;
}

/*copies from infile to outfile */
void copyFile (char *infile, char *outfile){
	FILE *in, *out;
	unsigned long fsize;
	
	if ( (in = fopen(infile,"rb")) <0) {
		printf("Cannot open file %s\n", infile); exit(0);
	}	

	unlink(outfile);
	if( (out = fopen(outfile, "w")) < 0) {
		printf("Cannot open file %s\n", outfile);
		exit(0);
	}	

	fsize=fileSize(infile);
	if (fsize) {
		char *buff = (char *) malloc(sizeof(char)*fsize);
		if (fread(buff,sizeof(char),fsize,in)) {
			ssize_t seek_write= fwrite(buff,sizeof(char),fsize,out);			
		}		
		free(buff);
	}			
	fclose(in);
	fclose(out);	
}

