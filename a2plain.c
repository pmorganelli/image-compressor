/*
 *      a2plain.c
 *      by Peter Morganelli and Amelia Bermack, 10/9/24
 *      locality assignment
 *
 *      This file contains the implementation for our a2plan methods
 *      
 */

#include <string.h>
#include <a2plain.h>
#include "uarray2.h"
#include <assert.h>

/************************************************/
/* Define a private version of each function in */
/* A2Methods_T that we implement.               */
/************************************************/
typedef A2Methods_UArray2 A2;   // private abbreviation
typedef void UArray2_applyfun(int i, int j, UArray2_T array2b, 
                              void *elem, void *cl);
                              
struct small_closure {
        A2Methods_smallapplyfun *apply; 
        void                    *cl;
};

/********** new ********
 *
 * Description: Creates a new A2
 *
 * Input Parameters:
 *      int width:      the width of the uarray2
 *      int height:     the height of the new uarray2
 *      int size:       the size of the uarray2
 *
 * Returns:
 *      a new A2
 *
 * Output:
 *      none
 *
 * Notes:
 *      Calls the UArray2_new function, which allocates memory for the A2
 *      
 ************************/
static A2 new(int width, int height, int size)
{
        return UArray2_new(width, height, size);
}

/********** new ********
 *
 * Description: Creates a new A2
 *
 * Input Parameters:
 *      int width:      the width of the uarray2
 *      int height:     the height of the new uarray2
 *      int size:       the size of the uarray2
 *      int blocksize:  the number of cells on each side of a block
 *
 * Returns:
 *      a new A2
 *
 * Output:
 *      none
 *
 * Notes:
 *      Calls the UArray2_new function, which allocates memory for the A2
 *      
 ************************/
static A2 new_with_blocksize(int width, int height, int size, int blocksize) 
{
        (void) blocksize;
        return UArray2_new(width, height, size);
}

/********** a2free ********
 *
 * Description: Frees memory created in regards to A2s
 *
 * Input Parameters:
 *      A2 uarray2:     an A2 to free
 *
 * Returns:
 *      none
 *
 * Output:
 *      none
 *
 * Notes:
 *      Frees memory allocated in the new() function
 *      
 ************************/
static void a2free(A2 *uarray2)
{
        assert(uarray2 != NULL);
        UArray2_free((UArray2_T *) uarray2);
}

/********** height ********
 *
 * Description: Returns the height of the A2
 *
 * Input Parameters:
 *      A2 uarray2:     the A2 to find the height of 
 *
 * Returns:
 *      an integer, the height of the A2
 *
 * Output:
 *      none
 *
 * Notes:
 *      Will CRE if a null A2 is passed in
 *      
 ************************/
static int height(A2 uarray2)
{
        assert(uarray2 != NULL);
        return UArray2_height(uarray2);
}

/********** width ********
 *
 * Description: Returns the width of the A2
 *
 * Input Parameters:
 *      A2 uarray2:     the A2 to find the width of 
 *
 * Returns:
 *      an int, the width of the A2
 *
 * Output:
 *      none
 *
 * Notes:
 *      Will CRE if uarray2 is NULL
 *      
 ************************/
static int width(A2 uarray2)
{
        assert(uarray2 != NULL);
        return UArray2_width(uarray2);
}

/********** size ********
 *
 * Description: Returns the size of elements in the the A2
 *
 * Input Parameters:
 *      A2 uarray2:     the A2 that we're finding the size of the elements
 *
 * Returns:
 *      an int, the size of elements in the A2
 *
 * Output:
 *      none
 *
 * Notes:
 *      Will CRE if uarray2 is NULL
 *      
 ************************/
static int size(A2 uarray2)
{
        assert(uarray2 != NULL);
        return UArray2_size(uarray2);
}

/********** blocksize ********
 *
 * Description: Returns the blocksize of the A2
 *
 * Input Parameters:
 *      A2 uarray2:     the A2 to find the blocksize of 
 *
 * Returns:
 *      an int, the width of the A2
 *
 * Output:
 *      none
 *
 * Notes:
 *      Will CRE if uarray2 != NULL
 *      
 ************************/
static int blocksize(A2 uarray2){
        assert(uarray2 != NULL);
        (void) uarray2;
        return 1;
}

/********** at ********
 *
 * Description: Returns a single element (cell) from the uarray2
 *
 * Input Parameters:
 *      A2 uarray2:     the A2 to get the element from 
 *
 * Returns:
 *      an element in the A2
 *
 * Output:
 *      none
 *
 * Notes:
 *      Will CRE if uarray2 is NULL 
 *      
 ************************/
static A2 at(A2 uarray2, int i, int j)
{
        assert(uarray2 != NULL);
        return UArray2_at(uarray2, i, j);
}

/********** map_row_major ********
 *
 * Description: Maps over an A2 in row-major order 
 *
 * Input Parameters:
 *      A2 uarray2:             the A2 to map over
 *      A2Methods_applyfun:     the apply function 
 *      void *cl:               the closure variable 
 *
 * Returns:
 *      none
 *
 * Output:
 *      whatever the apply function specifies
 *
 * Notes:
 *      Will CRE if uarray2 is NULL 
 *      
 ************************/
static void map_row_major(A2 uarray2,
                          A2Methods_applyfun apply,
                          void *cl)
{
        assert(uarray2 != NULL);
        assert(apply != NULL);
        // assert(cl != NULL);
        UArray2_map_row_major(uarray2, (UArray2_applyfun*)apply, cl);
}

/********** map_col_major ********
 *
 * Description: Maps over an A2 in col-major order 
 *
 * Input Parameters:
 *      A2 uarray2:             the A2 to map over
 *      A2Methods_applyfun:     the apply function 
 *      void *cl:               the closure variable 
 *
 * Returns:
 *      none
 *
 * Output:
 *      whatever the apply function specifies
 *
 * Notes:
 *      Will CRE if uarray2 is NULL 
 *      
 ************************/
static void map_col_major(A2 uarray2,
                          A2Methods_applyfun apply,
                          void *cl)
{
        assert(uarray2 != NULL);
        assert(apply != NULL);
        assert(cl != NULL);
        UArray2_map_col_major(uarray2, (UArray2_applyfun*)apply, cl);
}

/********** apply_small ********
 *
 * Description: An apply function 
 *
 * Input Parameters:
 *      int i:                  the column of the element
 *      int j:                  the row of the element
 *      UArray2_t uarray2:      the uarray2 to map over
 *      void *elem:             the actual element to call the apply function on
 *      void *vcl:              the closure variable 
 *
 * Returns:
 *      none
 *
 * Output:
 *      whatever the apply function specifies
 *
 * Notes:
 *      Will CRE if uarray2 is NULL 
 *      
 ************************/
static void apply_small(int i, int j, UArray2_T uarray2,
                        void *elem, void *vcl)
{
        assert(uarray2 != NULL);
        assert(elem != NULL);
        assert(vcl != NULL);
        (void) i; (void) j; (void) uarray2;
        struct small_closure *cl = vcl;
        cl->apply(elem, cl->cl);
}

/********** small_map_row_major ********
 *
 * Description: Maps over an A2 in row-major order 
 *
 * Input Parameters:
 *      A2 a2:                  the A2 to map over
 *      A2Methods_applyfun:     the apply function 
 *      void *cl:               the closure variable 
 *
 * Returns:
 *      none
 *
 * Output:
 *      whatever the apply function specifies
 *
 * Notes:
 *      Will CRE if uarray2 is NULL 
 *      
 ************************/
static void small_map_row_major(A2 a2, A2Methods_smallapplyfun apply, void *cl)
{
        assert(a2 != NULL);
        assert(apply != NULL);
        assert(cl != NULL);
        struct small_closure mycl = { apply, cl };
        UArray2_map_row_major(a2, apply_small, &mycl);
}

/********** small_map_col_major ********
 *
 * Description: Maps over an A2 in col-major order 
 *
 * Input Parameters:
 *      A2 a2:                  the A2 to map over
 *      A2Methods_applyfun:     the apply function 
 *      void *cl:               the closure variable 
 *
 * Returns:
 *      none
 *
 * Output:
 *      whatever the apply function specifies
 *
 * Notes:
 *      Will CRE if uarray2 is NULL 
 *      
 ************************/
static void small_map_col_major(A2        a2,
                                A2Methods_smallapplyfun  apply,
                                void *cl)
{
        assert(a2 != NULL);
        assert(apply != NULL);
        assert(cl != NULL);
        struct small_closure mycl = { apply, cl };
        UArray2_map_col_major(a2, apply_small, &mycl);
}


static struct A2Methods_T uarray2_methods_plain_struct = {
        new,
        new_with_blocksize,
        a2free,
        width,
        height,
        size,
        blocksize,                           //blocksize 
        at,
        map_row_major,  
        map_col_major,     
        NULL,                             //map_block_major
        map_row_major,                    //map_default
        small_map_row_major,             
        small_map_col_major,      
        NULL,                             //small_map_block_major
        small_map_row_major,              //small_map_default
};

// finally the payoff: here is the exported pointer to the struct
A2Methods_T uarray2_methods_plain = &uarray2_methods_plain_struct;
