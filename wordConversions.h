/*
 *      wordConversions.h
 *      by Peter Morganelli and Shepard Rodgers, 10/12/24
 *      arith assignment
 *
 *      This file contains the interface for conversions between component 
 *      video pixels and bitpacked codewords. Specifically, it turns 2x2 blocks
 *      of component video pixels into codewords, exporting a function for 
 *      conversion in each direction.
 *  
 */
#ifndef WORD_CONVERSIONS
#define WORD_CONVERSIONS

#include "a2blocked.h"
#include "a2plain.h"

/* Compression */
A2Methods_UArray2 blocksToWords(A2Methods_UArray2 uarray2, 
                                A2Methods_T methods);

/* Decompression */
A2Methods_UArray2 wordsToBlocks(A2Methods_UArray2 uarray2, 
                                A2Methods_T methods);
                                  
#undef WORD_CONVERSIONS
#endif