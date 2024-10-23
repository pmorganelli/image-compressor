/*
*       wordConversions.c
*       by Peter Morganelli and Shepard Rodgers, 10/12/24
*       arith assignment
*
*       This file contains the implementation for the wordConversions interface
*       providing functions to do most major steps of the image compression,
*       including the discrete cosine transform, quantizing average values, and
*       bitpacking into codewords. The file contains both compression and 
*       decompression steps.
*/

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "assert.h"

#include "wordConversions.h"
#include "a2blocked.h"
#include "a2plain.h"
#include "componentVideo.h"
#include "arith40.h"
#include "transformPixels.h"
#include "math.h"
#include "packOrUnpack.h"

/* A constant integer representing the blocksize for a 2x2 block */
const int BLOCKSIZE = 2;

/* A struct containing all the appropriate floats for a cv 2x2 block */
struct cvBlock {
        float y1, y2, y3, y4, pb, pr; /* y values are luma of the 4 pixels. */
                                    /* pb and pr are averages for the block */
};

/* A struct containing the scaled-integer representations of values obtained
   through discrete cosine transformation and quantization of cv blocks */
struct blockAverages {
        uint64_t a;  /*average brightness of the image*/
        int64_t  b;  /*degree to which image gets brighter from top to bottom*/
        int64_t  c;  /*degree to which image gets brighter from left to right*/
        int64_t  d;  /*degree to which pixels are brighter on one diagonal */
        uint64_t pb; /*average color-difference proportional to B - Y*/
        uint64_t pr; /*average color-difference proportional to R - Y*/
};

/* struct to be passed as closure to a conversion mapping function, containing
   the 2d array in which the end product of conversion (either words or
   component video pixels) will be stored and corresponding methods) */
struct averagesClosure {
        A2Methods_UArray2 closureUArray2;/* uarray2 to store converted values*/
        A2Methods_T methods; /* methods for the uarray2 */
};

/* Compression Functions */
void calculateAndPackWords(int col, int row, A2Methods_UArray2 uarray2, 
                           void *elem, void *cl);
void findAverageChroma(struct componentVideo *cell1, 
                       struct componentVideo *cell2, 
                       struct componentVideo *cell3, 
                       struct componentVideo *cell4, 
                       struct blockAverages *averagesStruct);
void discreteCosineTransform(float Y1, float Y2, float Y3, float Y4, 
                             struct blockAverages *averagesStruct);
float capOrNoCapDCT(float coefficient);

/* Decompression Functions */
void unpackAndCalculateCv(int col, int row, A2Methods_UArray2 uarray2, 
                          void *elem, void *cl);
void unpackAverages(struct blockAverages *averagesStruct, uint32_t arrayWord);
void convertAverages(struct blockAverages *averagesStruct, 
                     struct cvBlock *cvBlockStruct); 
void setCv(struct cvBlock *cvBlockStruct, 
           struct componentVideo *cvStruct1, struct componentVideo *cvStruct2, 
           struct componentVideo *cvStruct3, struct componentVideo *cvStruct4);

/****************************************************************
*                                                               *
*                  Compression Functions                        *
*                                                               *
*****************************************************************/

/********** blocksToWords ********
 *
 * Description: Transforms a uarray2 of component video pixels to 32-bit words
 *
 * Input Parameters:
 *      A2Methods_UArray2 uarray2: the uarray2 of pixels
 *      A2Methods_T methods: the type of methods that uarray2 uses
 *
 * Ouput:
 *      A new uarray2 which contains 32-bit words
 * 
 * Expects:
 *      uarray2 to not be null
 *      methods to not be null
 * Notes:
 *      Expects uarray2 and methods to not be NULL, otherwise will CRE
 *      May CRE if memory allocation fails
 *      Frees memory allocated for uarray2
 *      
 ************************/
A2Methods_UArray2 blocksToWords(A2Methods_UArray2 uarray2, A2Methods_T methods)
{
        assert(uarray2 != NULL);
        assert(methods != NULL);
        
        /* create 2d array to store bitpacked codewords */
        int width = methods->width(uarray2) / BLOCKSIZE;
        int height = methods->height(uarray2) / BLOCKSIZE;
        A2Methods_UArray2 wordsUArray2 = methods->new(width, height, 
                                            sizeof(uint32_t));
        
        /* put the 2d array in closure struct to be updated in apply func */
        struct averagesClosure *averagesCl = malloc(sizeof(*averagesCl));
        assert(averagesCl != NULL);
        averagesCl->closureUArray2 = wordsUArray2;
        averagesCl->methods = methods;

        /* map through component video uarray2 and convert blocks to codewords 
           and store them in the 2d array created above */
        methods->map_default(uarray2, calculateAndPackWords, averagesCl);

        /* free memory allocated for closure struct and passed-in uarray2 */
        free(averagesCl);
        methods->free(&uarray2);
        
        return wordsUArray2;
}

/********** calculateAndPackWords ********
 *
 * Description: An apply function to turn an index in uarray2 from CV to a 
 *              packed 32-bit codeword
 *
 * Input Parameters:
 *      int col:                        the current col in the uarray2
 *      int row:                        the current row in the uarray2
 *      A2Methods_UArray2 uarray2:      the uarray we are applying on
 *      void *elem:                     the element at this index (unused)
 *      void *cl:                       the closure variable for apply
 *
 * Ouput:
 *      None
 *
 * Expects
 *      uarray2 to not be null
 *      closure to not be null
 * Notes:
 *      Will CRE if uarray2 is null
 *      Will CRE if closure is null
 *      
 ************************/
void calculateAndPackWords(int col, int row, A2Methods_UArray2 uarray2, 
                           void *elem, void *cl)
{
        (void) elem;
        assert(uarray2 != NULL);
        assert(cl != NULL);
        
        /* check that we are on the top left cell of a 2x2 block */
        if (!(col % BLOCKSIZE == 0) || !(row % BLOCKSIZE == 0)) {
                return;
        }

        struct averagesClosure *avgClosure = cl;
        struct componentVideo *cell1, *cell2, *cell3, *cell4;

        /* Get all four indices in the block */
        cell1 = avgClosure->methods->at(uarray2, col, row);
        cell2 = avgClosure->methods->at(uarray2, col + 1, row);
        cell3 = avgClosure->methods->at(uarray2, col, row + 1);
        cell4 = avgClosure->methods->at(uarray2, col + 1, row + 1);
        
        struct blockAverages *averagesStruct = calloc(1, 
                                                      sizeof(*averagesStruct));
        
        /* average pb and pr values of the block and store them in struct */
        findAverageChroma(cell1, cell2, cell3, cell4, averagesStruct);
        
        /* Use DCT to get a, b, c, and d from the block's Y values and 
           store them in struct*/
        discreteCosineTransform(cell1->Y, cell2->Y, cell3->Y, cell4->Y, 
                                averagesStruct);

        /* get the spot in the words uarray2 to store the bitpacked codeword */
        int arrayWordCol = col / 2;
        int arrayWordRow = row / 2;
        uint32_t *arrayWord = avgClosure->methods->at
                              (avgClosure->closureUArray2, 
                               arrayWordCol, arrayWordRow);

        /* bitpack all "average" values from the block into a codeword */   
        uint32_t word = bitpack(averagesStruct->a, averagesStruct->b, 
                                averagesStruct->c, averagesStruct->d, 
                                averagesStruct->pb, averagesStruct->pr);
        
        *arrayWord = word;

        free(averagesStruct);
}

/********** findAverageChroma ********
 *
 * Description: To average the chroma values of a 2x2 block and store
 *              them in the given averagesStruct
 *
 * Input Parameters:
 *      struct componentVideo *cell1, *cell2, *cell3, *cell4: pointers to 
 *             the four cells in a 2x2 block
 *      struct blockAverages *averagesStruct: the struct to store the averages
 *                                            of the four cells in
 *
 * Ouput:
 *      None
 *
 * Expects
 *      NONE of the cells to be null (cell1, cell2, cell3, cell4)
 *      averagesStruct to not be null
 * Notes:
 *      Will CRE if ANY of the cells are null (cell1, cell2, cell3, cell4)
 *      Will CRE if averagesStruct is null
 *      
 ************************/
void findAverageChroma(struct componentVideo *cell1, 
                       struct componentVideo *cell2, 
                       struct componentVideo *cell3, 
                       struct componentVideo *cell4, 
                       struct blockAverages *averagesStruct) 
{
        assert(averagesStruct != NULL);
        assert(cell1 != NULL);
        assert(cell2 != NULL);
        assert(cell3 != NULL);
        assert(cell4 != NULL);

        /* average Pb values in block */
        float pbSum = 0;
        pbSum += cell1->Pb;
        pbSum += cell2->Pb;
        pbSum += cell3->Pb;
        pbSum += cell4->Pb;
        float pbAverage = pbSum / 4;
        
        /* average Pr values in block */
        float prSum = 0;
        prSum += cell1->Pr;
        prSum += cell2->Pr;
        prSum += cell3->Pr;
        prSum += cell4->Pr;
        float prAverage = prSum / 4;

        /* use provided function to convert the averages to unsigned ints */
        averagesStruct->pb = Arith40_index_of_chroma(pbAverage);
        averagesStruct->pr = Arith40_index_of_chroma(prAverage);
}

/********** discreteCosineTransform ********
 *
 * Description: To transform the given Y values into coefficients a, b, c,
 *              and d using the discrete cosine
 *
 * Input Parameters:
 *      float Y1, Y2, Y3, Y2: the four Y value floats from a block
 *      struct blockAverages *averagesStruct: a struct to store the new
 *                                            coefficients in
 *
 * Ouput:
 *      None
 *
 * Expects
 *      averagesStruct to not be null
 * Notes:
 *      Will CRE if averagesStruct is null
 *      
 ************************/
void discreteCosineTransform(float Y1, float Y2, float Y3, float Y4, 
                             struct blockAverages *averagesStruct) 
{
        assert(averagesStruct != NULL);

        /* Use Discrete Cosine Transform to convert Y values to a, b, c, d */
        float a = (Y4 + Y3 + Y2 + Y1) / 4.0;
        float b = (Y4 + Y3 - Y2 - Y1) / 4.0;
        float c = (Y4 - Y3 + Y2 - Y1) / 4.0;
        float d = (Y4 - Y3 - Y2 + Y1) / 4.0;
        
        /* Cap our b, c, and d values before quantizing*/
        b = capOrNoCapDCT(b);
        c = capOrNoCapDCT(c);
        d = capOrNoCapDCT(d);

        /* quantize a, b, c, and d by mapping onto a signed integer set */
        averagesStruct->a = (uint64_t)round(a * 511);
        averagesStruct->b = (int64_t)(round(b * 50.0));
        averagesStruct->c = (int64_t)(round(c * 50.0));
        averagesStruct->d = (int64_t)(round(d * 50.0));
}

/********** capOrNoCapDCT ********
 *
 * Description: To cap the given coefficient to stay in discrete cosine 
 *              transform range (-0.3, 0.3)
 *
 * Input Parameters:
 *      float coefficient: the coefficient to be capped or not capped
 *
 * Ouput:
 *      The updated coefficient to be within the specified range
 *
 * Expects
 *      none
 * Notes:
 *      none
 *      
 ************************/
float capOrNoCapDCT(float coefficient)
{
        /* ensure that DCT values are in the range of -0.3 to +0.3 */
        if (coefficient > 0.3) {
                return 0.3;
        } else if (coefficient < -0.3) {
                return -0.3;
        }

        return coefficient;
}

/****************************************************************
*                                                               *
*                  Decompression Functions                      *
*                                                               *
*****************************************************************/

/********** wordsToBlocks ********
 *
 * Description: Transforms a uarray2 of 32-bit words into component video 
 *              blocks
 *
 * Input Parameters:
 *      A2Methods_UArray2 uarray2: the uarray2 of pixels
 *      A2Methods_T methods: the type of methods that uarray2 uses
 *
 * Ouput:
 *      A new uarray2 which contains component video blocks
 * 
 * Expects:
 *      uarray2 to not be null
 *      methods to not be null
 * Notes:
 *      Expects uarray2 and methods to not be NULL, otherwise will CRE
 *      May CRE if memory allocation fails
 *      Frees memory allocated for uarray2
 *      
 ************************/
A2Methods_UArray2 wordsToBlocks(A2Methods_UArray2 uarray2, A2Methods_T methods)
{
        assert(uarray2 != NULL);
        assert(methods != NULL);
        
        /* create 2d array to store unpacked component video pixels */
        int width = methods->width(uarray2) * BLOCKSIZE;
        int height = methods->height(uarray2) * BLOCKSIZE;
        A2Methods_UArray2 cvUArray2 = methods->new(width, height, 
                                            sizeof(struct componentVideo));
        
        /* put the 2d array in closure struct to be updated in apply func */
        struct averagesClosure *averagesCl = malloc(sizeof(*averagesCl));
        assert(averagesCl != NULL);
        averagesCl->closureUArray2 = cvUArray2;
        averagesCl->methods = methods;

        /* map through the uarray2 with words and convert them to blocks
           of component video pixels, storing them in 2d array created above */
        methods->map_default(cvUArray2, populateCv, methods);
        methods->map_default(uarray2, unpackAndCalculateCv, averagesCl);
        
        /* free memory allocated for closure struct and passed-in uarray2 */
        free(averagesCl);
        methods->free(&uarray2); 
        
        return cvUArray2;
}

/********** unpackAndCalculateCv ********
 *
 * Description: An apply function to turn an index in uarray2 from a 32-bit 
 *              codeword to a component video struct
 *
 * Input Parameters:
 *      int col:                        the current col in the uarray2
 *      int row:                        the current row in the uarray2
 *      A2Methods_UArray2 uarray2:      the uarray we are applying on
 *      void *elem:                     the element at this index (unused)
 *      void *cl:                       the closure variable for apply
 *
 * Ouput:
 *      None
 *
 * Expects
 *      uarray2 to not be null
 *      closure to not be null
 * Notes:
 *      Will CRE if uarray2 is null
 *      Will CRE if closure is null
 *      
 ************************/
void unpackAndCalculateCv(int col, int row, A2Methods_UArray2 uarray2, 
                          void *elem, void *cl)
{
        (void) uarray2;
        assert(elem != NULL);
        assert(cl != NULL);
        
        uint32_t arrayWord = *(uint32_t *)elem;
        struct averagesClosure *closure = cl;

        /* Create structs to store unpacked scaled ints and block CV data */
        struct blockAverages *averagesStruct = calloc(1, 
                                               sizeof(*averagesStruct));
        struct cvBlock *cvBlockStruct = calloc(1, sizeof(*cvBlockStruct));

        struct componentVideo *cvStruct1, *cvStruct2,
                              *cvStruct3, *cvStruct4;
        
        /* Get the four cells to unpack the word into */
        int uarray2Col = col * BLOCKSIZE;
        int uarray2Row = row * BLOCKSIZE;
        cvStruct1 = closure->methods->at(closure->closureUArray2, 
                                         uarray2Col, uarray2Row);
        cvStruct2 = closure->methods->at(closure->closureUArray2, 
                                         uarray2Col + 1, uarray2Row);
        cvStruct3 = closure->methods->at(closure->closureUArray2, 
                                         uarray2Col, uarray2Row + 1);
        cvStruct4 = closure->methods->at(closure->closureUArray2, 
                                         uarray2Col + 1, uarray2Row + 1);
        
        /* Unpack the codeword into a, b, c, d, pb, and pr */
        unpackAverages(averagesStruct, arrayWord);

        /* Turn the scaled integer values into data about a CV block */
        convertAverages(averagesStruct, cvBlockStruct);

        /* Update the individual CV cells with the block data */
        setCv(cvBlockStruct, cvStruct1, cvStruct2, cvStruct3, cvStruct4);

        free(averagesStruct);
        free(cvBlockStruct);
}

/********** unpackAverages ********
 *
 * Description: To unpack the parts of the codeword into coefficients in
 *              an averagesStruct
 *
 * Input Parameters:
 *      struct blockAverages *averagesStruct: the averagesStruct to store 
 *                                            the packed values into
 *      uint32_t arrayWord: the word we are unpacking
 *
 * Ouput:
 *      None
 *
 * Expects
 *      averagesStruct to not be null
 * Notes:
 *      Will CRE if averagesStruct is null
 *      
 ************************/
void unpackAverages(struct blockAverages *averagesStruct, uint32_t arrayWord)
{
        assert(averagesStruct != NULL);

        /* use the packOrUnpack functions to get individual values from word */
        averagesStruct->a = unpackUnsigned(arrayWord, "a");
        averagesStruct->b = unpackSigned(arrayWord, "b");
        averagesStruct->c = unpackSigned(arrayWord, "c");
        averagesStruct->d = unpackSigned(arrayWord, "d");
        averagesStruct->pb = unpackUnsigned(arrayWord, "pb");
        averagesStruct->pr = unpackUnsigned(arrayWord, "pr");
}

/********** convertAverages ********
 *
 * Description: To convert the coefficients a, b, c, d into the Y values, Pb, 
 *              and Pb using the inverse discrete cosine
 *
 * Input Parameters:
 *      struct blockAverages *averagesStruct: the averagesStruct to store 
 *                                            the new values into
 *      struct cvBlock *cvBlockStruct: a block in the uarray containing all 
 *                                     the CV data for one block
 *
 * Ouput:
 *      None
 *
 * Expects
 *      averagesStruct to not be null
 *      cvBlockStruct to not be null
 * Notes:
 *      Will CRE if averagesStruct is null
 *      Will CRE if cvBlockStruct is null
 *      
 ************************/
void convertAverages(struct blockAverages *averagesStruct, 
                     struct cvBlock *cvBlockStruct) 
{
        assert(averagesStruct != NULL);
        assert(cvBlockStruct != NULL);

        /* Convert scaled ints to their original float values */
        float a = (float)averagesStruct->a / 511.0;
        float b = (float)averagesStruct->b / 50.0;
        float c = (float)averagesStruct->c / 50.0;
        float d = (float)averagesStruct->d / 50.0;

        /* Use Inverse Discrete Cosine Transform to get original luma values */
        float Y1 = a - b - c + d;
        float Y2 = a - b + c - d;
        float Y3 = a + b - c - d;
        float Y4 = a + b + c + d;

        /* Use provided function to get original chroma averages */
        float pb = Arith40_chroma_of_index(averagesStruct->pb);
        float pr = Arith40_chroma_of_index(averagesStruct->pr);

        /* Store the block info in struct */
        cvBlockStruct->y1 = Y1;
        cvBlockStruct->y2 = Y2;
        cvBlockStruct->y3 = Y3;
        cvBlockStruct->y4 = Y4;
        cvBlockStruct->pb = pb;
        cvBlockStruct->pr = pr;
}

/********** setCv ********
 *
 * Description: To set the CV values for all of the blocks a 2x2 block
 *
 * Input Parameters:
 *      struct cvBlock *cvBlockStruct: the struct containing all the values
 *                                     for the blocks
 *      struct componentVideo *cvStruct1, *cvStruct2, *cvStruct3, *cvStruct4:
 *              The component video struct for each 2x2 blcok
 *
 * Ouput:
 *      None
 *
 * Expects
 *      averagesStruct to not be null
 * Notes:
 *      Will CRE if averagesStruct is null
 *      
 ************************/
void setCv(struct cvBlock *cvBlockStruct, 
           struct componentVideo *cvStruct1, struct componentVideo *cvStruct2, 
           struct componentVideo *cvStruct3, struct componentVideo *cvStruct4)
{
        assert(cvBlockStruct != NULL);
        assert(cvStruct1 != NULL);
        assert(cvStruct2 != NULL);
        assert(cvStruct3 != NULL);
        assert(cvStruct4 != NULL);

        /* Update the luma and chroma values for all four CV pixels */
        cvStruct1->Y = cvBlockStruct->y1;
        cvStruct2->Y = cvBlockStruct->y2;
        cvStruct3->Y = cvBlockStruct->y3;
        cvStruct4->Y = cvBlockStruct->y4;

        cvStruct1->Pb = cvBlockStruct->pb;
        cvStruct2->Pb = cvBlockStruct->pb;
        cvStruct3->Pb = cvBlockStruct->pb;
        cvStruct4->Pb = cvBlockStruct->pb;

        cvStruct1->Pr = cvBlockStruct->pr;
        cvStruct2->Pr = cvBlockStruct->pr;
        cvStruct3->Pr = cvBlockStruct->pr;
        cvStruct4->Pr = cvBlockStruct->pr;
}
