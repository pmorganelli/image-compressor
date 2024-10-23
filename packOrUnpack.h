/*
*      packValues.h
*      by Peter Morganelli and Shepard Rodgers, 10/20/24
*      arith assignment
*
*      This file contains the interface for the packValues 
*      abstraction containing the methods to pack signed and unsigned values
*      into a 32-bit codeword that will be placed into a UArray2 of 
*      uint32_t's
*  
*/

#ifndef PACKORUNPACK_H
#define PACKORUNPACK_H

#include <stdint.h>

/* Compression */
uint32_t bitpack(uint64_t a, int64_t b, int64_t c, int64_t d, 
                      uint64_t pb, uint64_t pr);

/* Decompression */
uint32_t unpackUnsigned(uint32_t word, char *value);
int32_t unpackSigned(uint32_t word, char *value);

#undef PACKORUNPACK_H
#endif