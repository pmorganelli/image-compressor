/*
 *      readOrWrite.h
 *      by Peter Morganelli and Shepard Rodgers, 10/22/24
 *      arith assignment
 *
 *      This file contains the interface for the reading and printing steps
 *      in compression/decompression. It is called readOrPrint because
 *      this function will correspond to either the reading portion of 
 *      compression OR the printing part of decompression
 *  
 */

#ifndef READ_OR_WRITE
#define READ_OR_WRITE

#include "pnm.h"

/* Compression */
Pnm_ppm trim(Pnm_ppm image, A2Methods_T methods);
void writeCompressed(A2Methods_UArray2 uarray2, A2Methods_T methods, 
                     unsigned width, unsigned height);

/* Decompression */
A2Methods_UArray2 readCompressed(FILE *fp, A2Methods_T methods);

#undef READ_OR_WRITE
#endif