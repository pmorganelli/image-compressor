/*
 *      compress40.c
 *      by Peter Morganelli and Shepard Rodgers, 10/22/24
 *      arith assignment
 *
 *      This file contains the implementations for image compression and
 *      decompression using Discrete Cosine Transformation, quantization, and 
 *      bitpacking. Compressed and decompressed images are written to 
 *      standard output. 
 *      
 */

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "assert.h"
#include "a2blocked.h"
#include "a2plain.h"
#include "pnm.h"

#include "compress40.h"
#include "readOrWrite.h"
#include "transformPixels.h"
#include "wordConversions.h"

/* Define our custom denominator as 255. We chose this because of the 
   maximum representation of a character, since we use putchar */
const unsigned CUSTOM_DENOMINATOR = 255;

/********** compress40 ********
 *
 * Compresses a given .PPM image using a compression algorithm and prints
 * the compressed image to stdout
 *
 * Parameters:
 *      FILE *input: a pointer to the file to be compressed
 *
 * Return: 
 *      none
 *
 * Expects
 *      FILE *input to not be null
 *      File to be in correct PPM format
 * 
 * Notes:
 *      Will CRE if the input file pointer is null
 *      Will CRE if Pnm_ppmread returns a null image
 *      
 ************************/
extern void compress40(FILE *input)
{
        /* Ensure the given file pointer is not null */
        assert(input != NULL);

        /* Use the UArray2 methods as default methods*/
        A2Methods_T methods = uarray2_methods_plain;

        /* Read and Trim our input file! Note: if even it is returned back */
        Pnm_ppm image = Pnm_ppmread(input, methods);
        assert(image != NULL);

        /* Trim the image if necessary */
        Pnm_ppm newImage = trim(image, methods);

        /* Transform pixels from RGB to component video (Cv) */
        A2Methods_UArray2 cvUArray2 = rgbToCv(newImage->pixels, 
                                              methods, newImage->denominator);

        /* Convert component video to our a, b, c, d, Pb avg, Pr avg */
        A2Methods_UArray2 bitpackedUArray2 = blocksToWords(cvUArray2, methods);

        unsigned width = methods->width(bitpackedUArray2) * 2;
        unsigned height = methods->height(bitpackedUArray2) * 2;

        /* Write the compressed words to disk */
        writeCompressed(bitpackedUArray2, methods, width, height);
        
        /* Free the image allocated by trim and the UArray2 with the words */
        Pnm_ppmfree(&newImage);
        methods->free(&bitpackedUArray2);
}

/********** decompress40 ********
 *
 * Decompress a given image using a decompression algorithm and prints
 * the decompressed image to stdout
 *
 * Parameters:
 *      FILE *input: a pointer to the file to be decompressed
 *
 * Return: 
 *      none
 *
 * Expects
 *      FILE *input to not be null
 * 
 * Notes:
 *      Will CRE if the input file pointer is null
 *      
 ************************/
extern void decompress40(FILE *input)
{
        /* Check that our input file pointer is not full */
        assert(input != NULL);

        /* Use the UArray2 PLAIN methods as default methods */
        A2Methods_T methods = uarray2_methods_plain;

        /* Read in the compressed words and store it in a UArray2 */
        A2Methods_UArray2 unpackedUArray2 = readCompressed(input, methods);

        /* Convert the compressed words into 2x2 CV blocks */
        unpackedUArray2 = wordsToBlocks(unpackedUArray2, methods);

        /* Convert the 2x2 CV blocks to an RGB representation */
        unpackedUArray2 = cvToRgb(unpackedUArray2, methods, 
                                  CUSTOM_DENOMINATOR);
        
        /* Construct a Pnm_ppm pixmap with the RGB represented data */
        struct Pnm_ppm pixmap  = { .width = methods->width(unpackedUArray2),
                                   .height = methods->height(unpackedUArray2),
                                   .denominator = CUSTOM_DENOMINATOR, 
                                   .pixels = unpackedUArray2, 
                                   .methods = methods
                                };
        /* Write the RGB data stored by pixmap to disk */
        Pnm_ppmwrite(stdout, &pixmap);

        /* Free free unpackedUArray2 from the compressed file */
        methods->free(&unpackedUArray2);
}