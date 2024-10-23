/*
 *      transformPixels.c
 *      by Peter Morganelli and Shepard Rodgers, 10/16/24
 *      arith assignment
 *
 *      This file contains the implementation for the transformation of
 *      RGB pixels to Component Video (compression) or vice versa!
 *      (decompression)
 */

#include <stdlib.h>
#include <stdio.h>
#include "assert.h"

#include "componentVideo.h"
#include "uarray2.h"
#include "transformPixels.h"
#include "pnm.h"

/* Compression Functions */
void populateCv(int col, int row, A2Methods_UArray2 uarray2, void *elem, 
                void *cl);
void calculateCv(void *elem, void *pixel, unsigned denominator);

/* Decompression Functions */
void populateRgb(int col, int row, A2Methods_UArray2 uarray2, void *elem, 
                 void *cl);
void calculateRgb(void *beforePix, void *afterPix, unsigned denominator);
float capOrNoCapRGB(float coefficient);

/* Compression and Decompression Functions */
void applyTransform(int i, int j, A2Methods_UArray2 array2b, void *elem, 
                    void *cl);

/* the transformClosure struct can be passed to the applyTransform function
   when mapping to provide useful information for RGB<-->CV conversions */
struct transformClosure {
        A2Methods_UArray2 newUArray2; /* a 2d array to put values in */
        A2Methods_T methods; /* methods for getting values from the 2d array */
        unsigned denominator; /* the denominator used for scaling pixels */
        void (*calculate)(void *beforePix, void *afterPix, 
                          unsigned denominator);
                /* ^ pointer to the function being used for transformation math
                (depends on whether we are compressing or decompressing) */
};

/****************************************************************
*                                                               *
*                    Compression Functions                      *
*                                                               *
*****************************************************************/

/********** rgbToCv ********
 *
 *  To transform all the RGB pixels into a component-video representation
 *
 * Parameters:
 *      A2Methods_UArray2 uarray2: the uarray2 of RGB values
 *      A2Methods_T methods:       the methods that uarray2 uses
 *      unsigned denominator:      the denominator of the original to scale 
 *
 * Return: 
 *      An A2Methods_UArray2 containing componentVideo stucts
 *
 * Expects
 *      uarray to not be null
 *      methods to not be null
 * 
 * Notes:
 *      Will CRE if uarray2 is null
 *      Will CRE is methods are null
 *      Allocates and frees a transformClosure struct
 *      
 ************************/
A2Methods_UArray2 rgbToCv(A2Methods_UArray2 uarray2, A2Methods_T methods, 
                          unsigned denominator)
{
        assert(uarray2 != NULL);
        assert(methods != NULL);

        /* create a new array to store component video values */
        A2Methods_UArray2 newUArray2 = methods->new(methods->width(uarray2), 
                                                    methods->height(uarray2), 
                                                sizeof(struct componentVideo));
        
        /* create a transformation struct to store info needed to transform 
           and pass it as closure to mapping func */
        struct transformClosure *transformation = malloc(sizeof
                                                         (*transformation));
        assert(transformation != NULL);
        transformation->newUArray2 = newUArray2;
        transformation->methods = methods;
        transformation->denominator = denominator;
        transformation->calculate = calculateCv; /* choose func to calculate cv
                                                    values from rgb */

        /* populate new uarray2 with component video structs, then fill it with
        cv values from rgb conversion */
        methods->map_default(newUArray2, populateCv, methods);
        methods->map_default(uarray2, applyTransform, transformation);

        free(transformation);
        return newUArray2;
}

/********** populateAverages ********
 *
 * Description: apply function to populate the given uarray2 with 
 *              componentVideo structs
 *
 * Input Parameters:
 *      int col:                   (UNUSED) The current col in uarray2 
 *      int row:                   (UNUSED) The current row in uarray2
 *      A2Methods_UArray2 uarray2: (UNUSED) The uarray2 on which the function 
 *                                          is applied
 *      void *elem:                The elem at (col, row) in uarray2
 *      void *cl:                  (UNUSED) The closure variable for this apply
 *
 * Ouput:
 *      None
 *
 * Notes:
 *      will CRE if elem is NULL
 *      will CRE if the allocated componentVideo struct is NULL (freed here)
 *      
 ************************/
void populateCv(int col, int row, A2Methods_UArray2 uarray2, void *elem, 
                void *cl)
{
        (void) col;
        (void) row;
        (void) uarray2;
        (void) cl;
        assert(elem != NULL);

        /* create and store component video struct */
        struct componentVideo *pixel = calloc(1, sizeof(*pixel));
        assert(pixel != NULL);

        *(struct componentVideo *)elem = *pixel;
        free(pixel);
}

/********** calculateCv ********
 *
 * Description: apply function to calculate the component video values
 *              using the scaled RGB values
 *
 * Input Parameters:
 *      void *beforePix: the pixel before transformation
 *      void *afterPix: a CV struct for the transformed pixels to be stored
 *
 * Ouput:
 *      None
 *
 * Notes:
 *      Will CRE if beforePix is null
 *      Will CRE if afterPix is null
 *      
 ************************/
void calculateCv(void *beforePix, void *afterPix, unsigned denominator)
{
        assert(beforePix != NULL);
        assert(afterPix != NULL);

        Pnm_rgb RGBpixel = beforePix;
        struct componentVideo *CVpixel = afterPix;

        /* scale rgb values by denominator to get float from 0-1 */
        float red = (float)RGBpixel->red / (float)denominator;
        float green = (float)RGBpixel->green / (float)denominator;
        float blue = (float)RGBpixel->blue / (float)denominator;
        
        /* convert to component video y, pb, and pr values */
        float y = (0.299 * red) + (0.587 * green) + (0.114 * blue);
        float pb = (-0.168736 * red) - (0.331264 * green) + (0.5 * blue);
        float pr = (0.5 * red) - (0.418688 * green) - (0.081312 * blue);

        CVpixel->Y = y;
        CVpixel->Pb = pb;
        CVpixel->Pr = pr;
}

/****************************************************************
*                                                               *
*                  Decompression Functions                      *
*                                                               *
*****************************************************************/


/********** cvToRgb ********
*
*  To transform all the cv struct into an RGB representation
*
* Parameters:
*      A2Methods_UArray2 uarray2: the uarray2 of CV structs values
*      A2Methods_T methods:       the methods that uarray2 uses
*      unsigned denominator:      the denominator of the original to scale 
*
* Return: 
*      An A2Methods_UArray2 containing RGB values
*
* Expects
*      uarray to not be null
*      methods to not be null
* 
* Notes:
*      Will CRE if uarray2 is null
*      Will CRE is methods are null
*      Allocates and frees a transformClosure struct
*      May CRE if memory allocation for transformClosure struct fails
*      
************************/
A2Methods_UArray2 cvToRgb(A2Methods_UArray2 uarray2, A2Methods_T methods, 
                          unsigned denominator)
{
        assert(uarray2 != NULL);
        assert(methods != NULL);
        
        /* create a new array to store rgb values */
        A2Methods_UArray2 newUArray2 = methods->new(methods->width(uarray2), 
                                                    methods->height(uarray2), 
                                                    sizeof(struct Pnm_rgb));

        /* create a transformation struct to store info needed to transform 
           and pass it as closure to mapping func */
        struct transformClosure *transformation = malloc(sizeof
                                                        (*transformation));
        assert(transformation != NULL);

        transformation->newUArray2 = newUArray2;
        transformation->methods = methods;
        transformation->denominator = denominator;
        transformation->calculate = calculateRgb;/*choose func to calculate rgb
                                                    values from cv */

        /* populate new uarray2 with rgb structs, then fill it with 
           rgb values from cv conversion */
        methods->map_default(newUArray2, populateRgb, methods);
        methods->map_default(uarray2, applyTransform, transformation);

        free(transformation);
        methods->free(&uarray2);

        return newUArray2;
}

/********** populateRgb ********
 *
 *  To transform all the RGB pixels into a component-video representation
 *
 * Parameters:
 *      A2Methods_UArray2 uarray2: the uarray2 of RGB values
 *      A2Methods_T methods:       the methods that uarray2 uses
 *      unsigned denominator:      the denominator of the original to scale 
 *
 * Return: 
 *      An A2Methods_UArray2 containing componentVideo stucts
 *
 * Expects
 *      uarray to not be null
 *      methods to not be null
 * 
 * Notes:
 *      Will CRE if uarray2 is null
 *      Will CRE is methods are null
 *      Allocates and frees a transformClosure struct
 *      
 ************************/
void populateRgb(int col, int row, A2Methods_UArray2 uarray2, void *elem, 
                 void *cl)
{
        (void) col;
        (void) row;
        (void) uarray2;
        (void) cl;
        assert(elem != NULL);

        /* create and store rgb struct  */
        Pnm_rgb pixel = calloc(1, sizeof(*pixel));
        assert(pixel != NULL);
        
        *(Pnm_rgb)elem = *pixel;
        free(pixel);
}

/********** calculateRgb ********
 *
 *  To calculate and transform the RGB equivalent given a componentVideo struct
 *
 * Parameters:
 *      A2Methods_UArray2 uarray2: the uarray2 of RGB values
 *      A2Methods_T methods:       the methods that uarray2 uses
 *      unsigned denominator:      the denominator of the original to scale 
 *
 * Return: 
 *      None
 *
 * Expects
 *      beforePix to not be null
 *      afterPix to not be null
 * 
 * Notes:
 *      Will CRE if beforePix is null
 *      Will CRE is afterPix is null
 *      
 ************************/
void calculateRgb(void *beforePix, void *afterPix, unsigned denominator)
{
        assert(beforePix != NULL);
        assert(afterPix != NULL);

        struct componentVideo *CVpixel = beforePix;
        Pnm_rgb RGBpixel = afterPix;
        
        /* pull component video values from struct */
        float y = (float)CVpixel->Y;
        float pb = (float)CVpixel->Pb;
        float pr = (float)CVpixel->Pr;

        /* Convert component video values to rgb */
        float r = 1.0 * y + 0.0 * pb + 1.402 * pr;
        float g = 1.0 * y - 0.344136 * pb - 0.714136 * pr;
        float b = 1.0 * y + 1.772 * pb + 0.0 * pr;

        /* Ensure that rgb values are between 0 and 1 before scaling them */
        r = capOrNoCapRGB(r);
        g = capOrNoCapRGB(g);
        b = capOrNoCapRGB(b);

        /* scale rgb values by denominator to get original values */
        RGBpixel->red = r * (float)denominator;
        RGBpixel->green = g * (float)denominator;
        RGBpixel->blue = b * (float)denominator;
}

/********** capOrNoCapRGB ********
 *
 *  To cap or raise RGB values to be within the range 0-1.
 *
 * Parameters:
 *      float coefficient: the coefficient of RGB to be checked/capped
 *
 * Return: 
 *      A capped or raised version of the float that was passed in
 *
 * Expects
 *      None
 * 
 * Notes:
 *      None
 *      
 ************************/
float capOrNoCapRGB(float coefficient) 
{
        /* ensure that RGB values are in the range of 0 to 1 */
        if (coefficient > 1) {
                return 1;
        } else if (coefficient < 0) {
                return 0;
        }
        return coefficient;
}

/****************************************************************
*                                                               *
*           Compression and Decompression Functions             *
*                                                               *
*****************************************************************/


/********** applyTransform ********
 *
 *  An apply function to be called to transform CV elements to RGB
 *
 * Parameters:
 *      int col:                   the current col apply is being called on
 *      int row:                   the current row apply is being called on
 *      A2Methods_UArray2 uarray2: the uarray2 apply is being called on
 *      void *elem:                a pointer to the current element in uarray2
 *      voic *cl:                  a closure variable holding transformClosure
 *
 * Return: 
 *      none
 *
 * Expects
 *      none
 * 
 * Notes:
 *      none
 *      
 ************************/
void applyTransform(int col, int row, A2Methods_UArray2 uarray2, void *elem, 
                    void *cl)
{       
        (void) uarray2;
        assert(elem != NULL);
        assert(cl != NULL);

        /* Use closure struct to get the array in which to store 
           transformed values in */
        struct transformClosure *closureStruct = cl;
        void *pixel = closureStruct->methods->at(closureStruct->newUArray2, 
                                                 col, row);

        /* Call the correct calculate function, given in the closure, to 
           do the actual transformation */
        closureStruct->calculate(elem, pixel, closureStruct->denominator);
}
