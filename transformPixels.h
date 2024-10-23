/*
 *      transformPixels.h
 *      by Peter Morganelli and Shepard Rodgers, 10/16/24
 *      arith assignment
 *
 *      This file contains the interface for transformPixels, which provides
 *      functions to convert rgb pixel values to component video pixel values 
 *      and vice versa. It also gives functions to populate a 2d array with
 *      both rgb and component video structs, but to use these structs one
 *      must include pnm.h or componentVideo.h, respectively.
 */

#ifndef TRANSFORM_PIXELS
#define TRANSFORM_PIXELS

#include "a2blocked.h"
#include "a2plain.h"

/* Compression */
A2Methods_UArray2 rgbToCv(A2Methods_UArray2 uarray2, A2Methods_T methods, 
                          unsigned denominator);
void populateCv(int col, int row, A2Methods_UArray2 uarray2, void *elem, 
                void *cl);

/* Decompression */
A2Methods_UArray2 cvToRgb(A2Methods_UArray2 uarray2, A2Methods_T methods, 
                          unsigned denominator);
void populateRgb(int col, int row, A2Methods_UArray2 uarray2, void *elem, 
                 void *cl);

#undef TRANSFORM_PIXELS
#endif