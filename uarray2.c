/*
 *      uarray2.c
 *      by Peter Morganelli and Amelia Bermack, 10/9/24
 *      locality assignment
 *
 *      This file contains the implementation for our UArray2 abstraction
 *      
 */

#include <stdio.h>
#include <stdlib.h>
#include "assert.h"

#include "uarray.h"
#include "uarray2.h"


/* UArray2_T struct that stores the uarray's data such as it's width,
   height, size of elements, and content of elements */
struct UArray2_T {
        int width;
        int height;
        int size;
        UArray_T elements;
};

/********** UArray2_new ********
 *
 * Description: Make a new UArray2 data structure with column bounds between
 *              0 and width -1 and row bounds between 0 and height - 1
 *
 * Input Parameters:
 *      int width = the width of the UArray2
 *      int height = the height of the UArray2
 *      int size = the size of the data that is being stored
 *
 * Ouput:
 *      A pointer to a UArray2_T struct
 *
 * Notes:
 *      Expects width and height to be nonnegative integers
 *      Expects size to be a positive integer
 *      UArray_new can raise Mem_Failed
 *      This function will malloc space for the uarray2 -- 
 *           user is responsible for freeing this memory
 *      
 ************************/
UArray2_T UArray2_new(int width, int height, int size) 
{
        /*  Ensure input is valid */
        assert(width >= 0);
        assert(height >= 0);
        assert(size > 0);

        /* Allocate space for a struct */
        UArray2_T uarray2 = malloc(sizeof(*uarray2));
        assert(uarray2 != NULL);

        /* Set our member variables for our UArray2 struct */
        uarray2->width = width;
        uarray2->height = height;
        uarray2->size = size;
        uarray2->elements = UArray_new((width*height), size);

        return uarray2;
}


/********** UArray2_free ********
 *
 * Description: Free the memory occupied by the UArray2 array
 *
 * Input Parameters:
 *      T *uarray2 = a pointer to a UArray2_T struct
 *
 * Ouput:
 *      None
 *      
 * Notes:
 *      It is a checked runtime error for uarray2 or *uarray2 to be null
 *       
 ************************/
void UArray2_free(UArray2_T *uarray2) 
{
        assert(uarray2 != NULL); /* First, make sure uarray2 isn't null */
        UArray_free(&((*uarray2)->elements)); /* Free our elements */
        free(*uarray2);
}


/********** UArray2_width ********
 *
 * Description: To return the width of the UArray2
 *
 * Input Parameters:
 *      T uarray2 = the UArray2 data structure
 *
 * Ouput:
 *      An integer representing the width of the UArray2
 *      Expected to return a nonnegative integer
 *
 * Notes:
 *      It is a checked runtime error for uarray2 to be null
 *     
 ************************/
int UArray2_width(UArray2_T uarray2) 
{
        assert(uarray2 != NULL);
        return uarray2->width;
}

/********** UArray2_height ********
 *
 * Description: To return the height of the UArray2
 *
 * Input Parameters:
 *      T uarray2 = the UArray2 data structure
 *
 * Ouput:
 *      An integer representing the height of the UArray2
 *      Expected to return a nonnegative integer
 *
 * Notes:
 *       It is a checked runtime error for uarray2 to be null 
 *      
 ************************/
int UArray2_height(UArray2_T uarray2) 
{
        assert(uarray2 != NULL);
        return uarray2->height;
}

/********** UArray2_size ********
 *
 * Description: To return the size of each element that is stored in the array
 *             
 *
 * Parameters:
 *      T uarray2 = the UArray2 data structure
 *
 * Output:
 *      An integer representing the size of the data being stored in the 
 *      uarray2
 *
 * Notes:
 *      will throw a CRE if incorrect number of arguments provided 
 *      It is a checked runtime error for uarray2 to be null
 *      
 ************************/
int UArray2_size(UArray2_T uarray2) 
{
        assert(uarray2 != NULL);
        return uarray2->size;
}

/********** UArray2_at ********
 *
 * Description: To return a void pointer to the element at the given
 *              index.
 *
 * Input Parameters:
 *      T uarray2 = the UArray2 data structure
 *      int col = the index of the given column
 *      int row = the index of the given row
 *
 * Ouput:
 *      a void pointer to the the element at the given col and row
 *
 * Notes:
 *      Expects col and row to be less than the width and height
 *      respectively (given by UArray2_width and UArray2_height.)
 *      It is a checked runtime error for uarray2 to be null
 *      
 ************************/
void *UArray2_at(UArray2_T uarray2, int col, int row) 
{
        /* ensure input is valid */
        assert(uarray2 != NULL);
        assert(col < uarray2->width);
        assert(row < uarray2->height);
        assert(col >= 0 && row >= 0); 

        /* Use array arithmetic to find index */
        int index = uarray2->width * row + col;
        return UArray_at(uarray2->elements, index);
}

/********** UArray2_map_row_major ********
 *
 * Description: To call the apply function for each element in the array.
 *              Specifically, iterate through the UArray2 by row rather 
 *              than column. Column indices vary more rapidly than row indices.
 *          
 * Input Parameters:
 *      T uarray2 = the UArray2 data structure
 *      void apply(int col, int row, UArray2_T uarray2, void *p1, void *p2)
 *                = The apply function that is called on every element in the
 *                  array -- must be given the column, row, uarray2 data 
 *                  structure, and two void pointers which the client
 *                  can decide what to do with.
 *      void *cl =  Threaded through every call made via function pointer, 
 *                  stores some value to be updated after every function call
 *                  (Example: passing in a bool to indicate that the array 
 *                   is in a proper state)
 * Ouput:
 *      None
 * 
 * Notes: Column indicies here will vary more rapidly than row indicies
 *        It is a checked runtime error for uarray2 to be null
 *      
 ************************/
void UArray2_map_row_major (UArray2_T uarray2, 
                            void apply(int col, int row, UArray2_T uarray2, 
                                       void *p1, void *p2), 
                            void *cl) 
{
        /* Make sure uarray2 isn't null */
        assert(uarray2 != NULL);

        /* Make our row and column variables for readability */
        int numRows = uarray2->height;
        int numCols = uarray2->width;
        
        /* Row major = column vary more rapidly */
        for (int row = 0; row < numRows; row++) {
                for (int col = 0; col < numCols; col++) {
                        /* Our index here follows this mathematical equation */
                        int index = uarray2->width * row + col;

                        /* Define our value as the element at this index*/
                        void *value = UArray_at(uarray2->elements, index);

                        /* Now, call our apply function on every index */
                        apply(col, row, uarray2, value, cl);
                }
        }
}


/********** UArray2_map_col_major ********
 *
 * Description: To call the apply function for each element in the array.
 *              Specifically, iterate through the UArray2 by column rather 
 *              than row. Row indices vary more rapidly than column indices.
 *
 * Input Parameters:
 *      T uarray2 = the UArray2 data structure
 *      void apply(int col, int row, UArray2_T uarray2, void *p1, void*p2)
 *                = The apply function that is called on every element in the
 *                  array -- must be given the column, row, uarray2 data 
 *                  structure, and two void pointers which the client
 *                  can decide what to do with.
 *      void *cl =  Threaded through every call made via function pointer, 
 *                  stores some value to be updated after every function call
 *                  (Example: passing in a bool to indicate that the array 
 *                   is in a proper state)
 *
 *
 * Ouput:
 *      None
 * 
 * Notes: Row indicies here will vary more rapidly than row indicies
 *        It is a checked runtime error for uarray2 to be null
 *      
 ************************/
void UArray2_map_col_major
        (UArray2_T uarray2, 
        void apply(int col, int row, UArray2_T uarray2, void *p1, void *p2), 
        void *cl) 
{
        
        /* Make sure uarray2 isn't null */
        assert(uarray2 != NULL);

        /* Make our row and column variables for readability */
        int numRows = uarray2->height;
        int numCols = uarray2->width;

        /* Col major = rows vary more rapidly */
        for (int col = 0; col < numCols; col++) {
                for (int row = 0; row < numRows; row++) {
                        /* Use array arithmetic to find curr index */
                        int index = uarray2->width * row + col;

                        /* Define our value as the element at this index */
                        void *value = UArray_at(uarray2->elements, index);

                        /* Now, call our apply function on every index */
                        apply(col, row, uarray2, value, cl);
                }
        }
}


