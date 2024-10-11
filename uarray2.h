/*
 *      uarray2.h
 *      by Peter Morganelli and Shepard Rodgers, 9/17/24
 *      iii assignment
 *
 *      This file is the interface file for our uarray2 abstraction, 
 *      which represents an unboxed two-dimensional array. To use this 
 *      interface, #include "uarray2.h"
 */

#include <stdio.h>
#include "uarray.h"

#ifndef UARRAY2_H
#define UARRAY2_H

typedef struct UArray2_T *UArray2_T;

UArray2_T UArray2_new(int width, int height, int size);
void UArray2_free(UArray2_T *uarray2);

int UArray2_width(UArray2_T uarray2);
int UArray2_height(UArray2_T uarray2);
int UArray2_size(UArray2_T uarray2);

void *UArray2_at(UArray2_T uarray2, int col, int row);

void UArray2_map_row_major
        (UArray2_T uarray2, 
        void apply(int col, int row, UArray2_T uarray2, void *p1, void *p2), 
        void *cl);

void UArray2_map_col_major
        (UArray2_T uarray2, 
        void apply(int col, int row, UArray2_T uarray2, void *p1, void *p2), 
        void *cl);
        
#undef UARRAY2_H
#endif 
