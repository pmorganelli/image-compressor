/*
 *      readOrWrite.c
 *      by Peter Morganelli and Shepard Rodgers, 10/22/24
 *      arith assignment
 *
 *      This file contains the implementation for the reading or printing
 *      stages of compression and/or decompression. It also provides a function
 *      to trim edges from a PPM image array with an odd height or width.
 *      
 */

#include <stdlib.h>
#include <stdio.h>
#include "stdint.h"
#include "assert.h"

#include "wordConversions.h"
#include "bitpack.h"
#include "readOrWrite.h"

/* Define some global ints to represente information about the codeword bits
   that assist in big-endian iteration */
const int CODEWORD_SIZE = 32; /* Define codeword size hence 32-bit uint */
const int BYTE_SIZE = 8; /* Define a global variable for the size of a byte */
const int BIGGEST_ENDIAN = 24; /* Starting point for big-endianness */
const int LITTLEST_ENDIAN = 0; /* Ending point for big-endianness */


/* Compression Functions */
Pnm_ppm makeNewImage(Pnm_ppm image, A2Methods_T methods, int height, 
                     int width);
void writeContents(int col, int row, A2Methods_UArray2 uarray2, 
                   void *elem, void *cl);


/* Decompression Functions */ 
void readWord(int col, int row, A2Methods_UArray2 uarray2, 
              void *elem, void *cl);

/****************************************************************
*                                                               *
*                  Compression Functions                        *
*                                                               *
*****************************************************************/


/********** trim ********
 *
 *  Trims a given Pnm_ppm image to have an even width and height 
 *
 * Parameters:
 *      Pnm_ppm image:         a Pnm_ppm struct with the information of the
 *                             original image
 *      A2Methods_T methods:   the methods that the image should use
 *
 * Return: 
 *      The Pnm_ppm image that was passed in except with trimmed height and
 *      width
 *
 * Expects
 *      image to not be null
 *      methods to not be null
 * 
 * Notes:
 *      Will CRE if the Pnm_ppm image is null
 *      Will CRE if methods are null
 *      
 ************************/
Pnm_ppm trim(Pnm_ppm image, A2Methods_T methods)
{
        /* Check our parameters */
        assert(image != NULL);
        assert(methods != NULL);
        int height = image->height;
        int width = image->width;

        /* If height and width are both even, return the original image */
        if (height % 2 == 0 && width % 2 == 0) {
                return image;
        }

        /* Modify the height and width into local variables to even numbers */
        if (height % 2 != 0) {
                height -= 1;
        }         
        if (width % 2 != 0) {
                width -= 1;
        }
        
        /* Make our new image using a helper function to copy over data */
        Pnm_ppm newImage = makeNewImage(image, methods, height, width);

        /* Free the original image and return the new modified Pnm_ppm 
           NOTE: the modified Pnm_ppm newImage is freed in compress40.c */
        Pnm_ppmfree(&image);

        return newImage;
}

/********** makeNewImage ********
 *
 *  Helper function for the trim function to copy over all of the pixels
 *  from the original image to a new image minus the trimmed height/width
 *
 * Parameters:
 *      Pnm_ppm image:         a Pnm_ppm struct with the information of the
 *                             original image
 *      A2Methods_T methods:   the methods that the image should use
 *      int height:            the trimmed height
 *      int width:             the trimmed width
 *
 * Return: 
 *      A new Pnm_ppm struct with the same data as the original image
 *      minues the columns and rows that were trimmed 
 *
 * Expects
 *      image to not be null
 *      methods to not be null
 *      height and width to be the trimmed values if applicable
 * 
 * Notes:
 *      Will CRE if malloc fails for newImage
 *      Will CRE if the Pnm_ppm image is null
 *      Will CRE if methods are null
 *      Will CRE if the creation of the uarray2 fails
 *      
 ************************/
Pnm_ppm makeNewImage(Pnm_ppm image, A2Methods_T methods, int height, int width)
{
        /* Malloc space for our newImage ppm */
        Pnm_ppm newImage = malloc(sizeof(*newImage));
        assert(newImage != NULL);
        
        /* Set all the header variables before populating the pixels */
        newImage->height = height;
        newImage->width = width;
        newImage->denominator = image->denominator;
        newImage->methods = image->methods;
        
        /* Make our new pixels uarray2 */
        A2Methods_UArray2 uarray2 = methods->new(width, 
                                                 height, 
                                                 sizeof(struct Pnm_rgb));
        assert(uarray2 != NULL);
        newImage->pixels = uarray2;
        
        /*  Copy over all the RGBs into the new pixels from the orig image */
        Pnm_rgb newPixel;
        Pnm_rgb originalPixel;
        for (int row = 0; row < height; row++) {
                for(int col = 0; col < width; col++) {
                        newPixel = methods->at(newImage->pixels, col, row);
                        originalPixel = methods->at(image->pixels, col, row);
                        newPixel->red = originalPixel->red;
                        newPixel->green = originalPixel->green;
                        newPixel->blue = originalPixel->blue;
                }
        }

        return newImage;        
}

/********** writeCompressed ********
 *
 *  To write the given compressed file to disk in big-endian order
 *
 * Parameters:
 *      A2Methods_UArray2 uarray2:  a uarray2 of 32-bit codewords
 *                                  the original image
 *      A2Methods_T methods:        the methods that the image should use
 *      unsigned height:            the height of the compressed file
 *      unsigned width:             the width of the compressed file
 *
 * Return: 
 *      none
 *
 * Expects
 *      width and height to be even numbers
 * 
 * Notes:
 *      Will CRE if width and height are not even numbers
 *      
 ************************/
void writeCompressed(A2Methods_UArray2 uarray2, A2Methods_T methods, 
                     unsigned width, unsigned height)
{
        /* Write the header to disk and make sure width/height are even */
        printf("COMP40 Compressed image format 2\n%u %u", width, height);
        printf("\n");
        assert(width % 2 == 0);
        assert(height % 2 == 0);

        /* Map through the uarray2 of codewords and write them to disk */
        methods->map_row_major(uarray2, writeContents, NULL);
}

/********** writeContents ********
 *
 *  An apply function to write the given codeword to disk in big-endian order
 *
 * Parameters:
 *      int col:                    (UNUSED) the current col in uarray2
 *      int row:                    (UNUSED) the current row in uarray2
 *      A2Methods_UArray2 uarray:   (UNUSED) the uarray2 of codewords
 *      void *elem:                 the current codeword in the uarray2
 *      void *cl:                   (UNUSED) the closure variable
 *                         
 *
 * Return: 
 *      none
 *
 * Expects
 *      void *elem to be a pointer to a 32-bit packed codeword
 * 
 * Notes:
 *      None
 *      
 ************************/
void writeContents(int col, int row, A2Methods_UArray2 uarray2, 
                   void *elem, void *cl)
{
        (void) col;
        (void) row;
        (void) uarray2;
        (void) cl;
        
        /* Cast the elem passed in as a 32-bit signed integer */
        int32_t word = *(int32_t *)elem;
        
        /* Write the codeword to disk using putchar in big-endian order */
        for (int i = BIGGEST_ENDIAN; i >= LITTLEST_ENDIAN; i -= BYTE_SIZE) {
                putchar(Bitpack_getu(word, BYTE_SIZE, i));
        }
}


/****************************************************************
*                                                               *
*                  Decompression Functions                      *
*                                                               *
*****************************************************************/


/********** readCompressed ********
 *
 *  To read the compressed file into a UArray2
 *
 * Parameters:
 *      FILE *fp:            a pointer to the file containing compressed 
 *                           32-bit codewords
 *      A2Methods_T methods: the methods that will be used for the construction
 *                           of the UArray2 of codewords
 *
 * Return: 
 *      An A2Methods_UArray2 containing the codewords from the image
 *
 * Expects
 *      fp and methods to not be null
 *      
 * Notes:
 *      Will CRE if fp is null
 *      Will CRE if methods are null
 *      Will CRE if fscanf does not return 2
 *      Will CRE if getc does not return a newline
 *      
 ************************/
A2Methods_UArray2 readCompressed(FILE *fp, A2Methods_T methods)
{
        /* Check that our parameters are valid */
        assert(fp != NULL);
        assert(methods != NULL);

        /* Read in the height and width and store them using spec code */
        unsigned height, width;
        int read = fscanf(fp, "COMP40 Compressed image format 2\n%u %u", 
                          &width, &height);
        assert(read == 2);
        int c = getc(fp);
        assert(c == '\n');
        
        /* Make a new uarray2 that will contain codewords that has half the
           original width and height */
        A2Methods_UArray2 wordsUArray2 = methods->new(width / 2, height / 2, 
                                                      sizeof(uint32_t));
        /* Map through this new uarray2 to insert the words */
        methods->map_default(wordsUArray2, readWord, fp);
       
        return wordsUArray2;
}

/********** readWord ********
 *
 *  An apply function for readCompresssed to read in an individual codeword
 *
 * Parameters:
 *      int col:                    (UNUSED) the current col in uarray2
 *      int row:                    (UNUSED) the current row in uarray2
 *      A2Methods_UArray2 uarray:   (UNUSED) the uarray2 of codewords
 *      void *elem:                 the current codeword in the uarray2
 *      void *cl:                   the closure variable representing the
 *                                  file pointer to read in from
 *
 * Return: 
 *      none
 *
 * Expects
 *      elem to be a pointer to a 32-bit codeword and not null
 *      closure to not be null
 *      
 * Notes:
 *      Will CRE if cl is null
 *      Will CRE if elem is null
 *      
 ************************/
void readWord(int col, int row, A2Methods_UArray2 uarray2, 
              void *elem, void *cl)
{
        (void) col;
        (void) row;
        (void) uarray2;

        /* Assert our parameters */
        assert(elem != NULL);
        assert(cl != NULL);

        /* Cast our closure variable to a file pointer to then read from */
        FILE *fp = (FILE *)cl;
        
        /* Cast our elem variable to a uint32_t to store the codeword */
        uint32_t *finalWord = (uint32_t *)elem;

        /* Define an byte to then be read in and a word of all 0s to help 
           with the process of reading in the codeword*/
        int byte;
        uint32_t word = (uint32_t)0;

        /* Assign the current elemtn in the array to be the codeword 
           which is read in big-endian */
        for (int i = BIGGEST_ENDIAN; i >= LITTLEST_ENDIAN; i -= BYTE_SIZE) {
                byte = getc(fp);
                assert(byte != EOF);
                word = Bitpack_newu(word, BYTE_SIZE, i, byte);
        }
        /* Set our element equal to the word we have read in */
        *finalWord = word;
}