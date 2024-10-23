/*
*      packOrUnpack.c
*      by Peter Morganelli and Shepard Rodgers, 10/22/24
*      arith assignment
*
*      This file contains the implementations for image compression and
*      decompression using Discrete Cosine Transformation, quantization, and 
*      bitpacking. Compressed and decompressed images are written to 
*      standard output. 
*/

#include "packOrUnpack.h"
#include "bitpack.h"
#include <string.h>
#include "assert.h"

/* Define global variables for the LSB and Width for A, B, C, D, Pb and Pr */

/* LSB values for each variables */
const unsigned A_LSB = 23;
const unsigned B_LSB = 18;
const unsigned C_LSB = 13;
const unsigned D_LSB = 8;
const unsigned PB_LSB = 4;
const unsigned PR_LSB = 0;

/* WIDTH values for each variable */
const unsigned A_WIDTH = 9;
const unsigned B_WIDTH = 5;
const unsigned C_WIDTH = 5;
const unsigned D_WIDTH = 5;
const unsigned PB_WIDTH = 4;
const unsigned PR_WIDTH = 4;

/****************************************************************
*                                                               *
*                  Compression Functions                        *
*                                                               *
*****************************************************************/

/********** bitpack ********
 *
 *  To pack the values of a, b, c, d, pb, and pr into a 32-bit codeword 
 *
 * Parameters:
 *      uint64_t a:  the value of a
 *      int64_t b:   the value of b
 *      int64_t c:   the value of c
 *      int64_t d:   the value of d
 *      uint64_t pb: the value of pb
 *      uint64_t pr: the value of pr
 *
 * Return: 
 *      A 32-bit unsigned packed codeword containing the data of a, b, c, d,
 *      pb, and pr
 *
 * Expects
 *      All values passed in to be accurate 
 * 
 * Notes:
 *      none
 *      
 ************************/
uint32_t bitpack(uint64_t a, int64_t b, int64_t c, int64_t d, 
                 uint64_t pb, uint64_t pr)
{
        uint32_t word = (uint32_t)0;
        
        /* Pack each section into the codeword in the order: 
           A, B, C, D, Pb, Pr */
        word = Bitpack_newu(word, A_WIDTH, A_LSB, a);
        word = Bitpack_news(word, B_WIDTH, B_LSB, b);
        word = Bitpack_news(word, C_WIDTH, C_LSB, c);
        word = Bitpack_news(word, D_WIDTH, D_LSB, d);
        word = Bitpack_newu(word, PB_WIDTH, PB_LSB, pb);
        word = Bitpack_newu(word, PR_WIDTH, PR_LSB, pr);

        return word;
}

/****************************************************************
*                                                               *
*                 Decompression Functions                       *
*                                                               *
*****************************************************************/

/********** unpackUnsigned ********
 *
 *  To extract an unsigned integer from the codeword
 *
 * Parameters:
 *      uint32_t word: the codeword to extract from
 *      char *value:   a cstring containing which unsigned section to unpack
 *
 * Return: 
 *      An unsigned 32-bit integer representing either an a, pb, or pr value
 *
 * Expects
 *      none
 * 
 * Notes:
 *      none
 *      
 ************************/
uint32_t unpackUnsigned(uint32_t word, char *value)
{
        uint32_t extractedValue = 0;
        
        /* See which value was passed in, and return the requested section 
           of the codeword */
        if (strcmp(value, "a") == 0) {
                extractedValue = Bitpack_getu(word, A_WIDTH, A_LSB);
        } else if (strcmp(value, "pb") == 0) {
                extractedValue = Bitpack_getu(word, PB_WIDTH, PB_LSB);
        } else if (strcmp(value, "pr") == 0) {
                extractedValue = Bitpack_getu(word, PR_WIDTH, PR_LSB);
        }

        return extractedValue;
}

/********** unpackSigned ********
 *
 *  To extract a signed integer from the codeword
 *
 * Parameters:
 *      uint32_t word: the codeword to extract from
 *      char *value:   a cstring containing which unsigned section to unpack
 *
 * Return: 
 *      A signed 32-bit integer representing either a b, c, or d value
 *
 * Expects
 *      none
 * 
 * Notes:
 *      none
 *      
 ************************/
int32_t unpackSigned(uint32_t word, char *value)
{
        int32_t extractedValue = 0;

        /* See which value was passed in, and return the requested section 
           of the codeword */
        if (strcmp(value, "b") == 0) {
                extractedValue = Bitpack_gets(word, B_WIDTH, B_LSB);
        } else if (strcmp(value, "c") == 0) {
                extractedValue = Bitpack_gets(word, C_WIDTH, C_LSB);
        } else if (strcmp(value, "d") == 0) {
                extractedValue = Bitpack_gets(word, D_WIDTH, D_LSB);
        }

        return extractedValue;
}