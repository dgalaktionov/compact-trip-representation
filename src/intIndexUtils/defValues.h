/* defValues.h
 * Copyright (C) 2011, Antonio Fariña, all rights reserved.
 *
 * defValues.h: Some default constants used in the i-CSA self-index.
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

#ifndef DEFVALUESSAD
#define DEFVALUESSAD

// CONFIGURACIONS DO CSA DE SADAKANE PARA TEXTOS DE ENTEIROS

// Parametros configurables
#define DEFAULT_A_SAMPLING_PERIOD 256		// Periodo de muestreo de A
#define DEFAULT_A_INV_SAMPLING_PERIOD 256		// Periodo de muestreo de inversa A
#define DEFAULT_PSI_SAMPLING_PERIOD 32		// Periodo de muestreo da funcion PSI

/*********************************/
#define PSI_HUFFMANRLE		  			// Uses Lbd's implementation of Psi. Improvement over Gonzalo's (in compression and speed)
#define DEFAULT_nsHUFF  16*1024   		// huffman limit on Psi. it can be optimized for space

//#define PSI_DELTACODES		  		// Uses Delta codes for increments of Psi. Faster but compresses less than the others
//#define PSI_GONZALO
/*********************************/


#define MAX_FILENAME_LENGTH 256						// Lonxitude maxima do nome do ficheiro

// Extensions dos ficheiros creados
#define NUMBER_OF_ELEMENTS_FILE_EXT "CSA.noe"			// Numero de elementos (tama�o de A, Psi e D)

#define D_FILE_EXT "CSA.Dbv"							// O vector D de Sadakane
#define D_RANK_DIRECTORY_FILE_EXT "CSA.Drd"				// O directorio para Rank sobre D

#define SAMPLES_A_FILE_EXT "CSA.sA"						// Mostras do array de sufixos
#define BA_FILE_EXT "CSA.BAbv"							// O vector D de Sadakane
#define BA_RANK_DIRECTORY_FILE_EXT "CSA.BArd"			// O directorio para Rank sobre D

#define SAMPLES_A_INV_FILE_EXT "CSA.sAI"				// Mostras da inversa do array de sufixos
#define SAMPLING_PERIOD_A_FILE_EXT "CSA.sTA"			// Periodo de muestreo do array de sufixos e da inversa

#define PSI_COMPRESSED_FILE_EXT "CSA.psi"				// COdigos delta de PSI
#define SAMPLING_PERIOD_PSI_SEARCHFACTOR_FILE_EXT "CSA.psiK"			// Periodo de muestreo de PSI y search_factor

#define DEFAULT_PSI_BINARY_SEARCH_FACTOR 2				// Periodo de muestreo na busca binaria
//#define BINARY_SEARCH_INTERVAL 128					// Periodo de muestreo na busca binaria

#endif
