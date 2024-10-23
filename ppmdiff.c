/*
 *      ppmdiff.c
 *      by Peter Morganelli and Shepard Rodgers, 10/11/24
 *      arith assignments
 *
 *      This file contains the implementation for the diff lab, ppmdiff!
 *      
 *      Run with "make ppmdiff"
 *      Useful to run the executable on local files like:
 *      "cjpeg flowers.ppm | djpeg | ./ppmdiff flowers.ppm -"
 *      
 */

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "assert.h"
#include "pnm.h"
#include "a2methods.h"
#include "a2plain.h"
#include "a2blocked.h"
#include <math.h>
#include "stdbool.h"

static FILE *open_or_abort(char *fname, char *mode);
double squareDiff(int col, int row, Pnm_ppm image1, Pnm_ppm image2);

/********** main ********
 *
 * Purpose: to run the ppmdiff program
 *
 * Parameters:
 *      int argc: the number of command-line arguments
 *      char *argv[]: an array of command-line arguments
 *
 * Return: 0 if everything works (EXIT_SUCCESS)
 *
 * Notes:
 *      calls open_or_abort to open files and squareDiff to perform operations
 ************************/
int main(int argc, char *argv[])
{
        /* Ensure that we have the required input files and check for stdin */
        assert(argc == 3);
        int stdinIndex = 0;
        for (int i = 1; i < argc; i++) {
                if (strcmp(argv[i], "-") == 0) {
                        stdinIndex = i;
                }
        }


        Pnm_ppm I;
        Pnm_ppm IPrime;
        A2Methods_T methods = uarray2_methods_plain;
        assert(methods != NULL);

        if (stdinIndex == 1) {
                I = Pnm_ppmread(stdin, methods);
                FILE *fp = open_or_abort(argv[2], "r");
                IPrime = Pnm_ppmread(fp, methods);
                fclose(fp);
        } else if (stdinIndex == 2) {
                FILE *fp = open_or_abort(argv[1], "r");
                I = Pnm_ppmread(fp, methods);
                IPrime = Pnm_ppmread(stdin, methods);
                fclose(fp);
        } else {
                FILE *fp1 = open_or_abort(argv[1], "r");
                FILE *fp2 = open_or_abort(argv[2], "r");
                I = Pnm_ppmread(fp1, methods);
                IPrime = Pnm_ppmread(fp2, methods);
                fclose(fp1);
                fclose(fp2);
        }

        int diff1 = I->height - IPrime->height;
        int diff2 = I->width - IPrime->width;
        if (abs(diff1) > 1 || abs(diff2) > 1) {
                float num = 1.0;
                fprintf(stderr, 
                "ERROR: Difference in width or height greater than one\n");
                printf("%.1f\n", num);
                exit(1);
        }

        double sum = 0.0;
        unsigned imageHeight = (I->height > IPrime->height) ?
                               IPrime->height : I->height;
        unsigned imageWidth = (I->width > IPrime->width) ?
                              IPrime->width : I->width;
        
        for (unsigned col = 0; col < imageWidth; col++) {
                if (col >= imageWidth) {
                        break;
                }
                for (unsigned row = 0; row < imageHeight; row++) {
                        if (row >= imageHeight) {
                                break;
                        }
                        sum += squareDiff(col, row, I, IPrime);
                }
        }

        double result = sqrt(sum / (3 * imageHeight * imageWidth));

        printf("%.4f\n", result);
        
        Pnm_ppmfree(&I);
        Pnm_ppmfree(&IPrime);

        return EXIT_SUCCESS;
}

/********** open_or_abort ********
 *
 * Opens the provided file and prepares it to be read
 *
 * Parameters:
 *      char *fname:           a pointer to the name of the file
 *      char *mode:            the mode to set up the file as
 *
 * Return: a FILE pointer to the open file, if successful
 *
 * Notes:
 *      Will CRE if the file cannot be opened
 ************************/
static FILE *open_or_abort(char *fname, char *mode) 
{
        /* Declare and open a file */
        FILE *input = fopen(fname, mode);

        /* Checks if the file opened correctly */
        assert(input != NULL);
        
        /* Return the opened file */
        return input;
}

/********** squareDiff ********
 *
 * Purpose: to find the difference in the RGB values between the two
 *          images respectivve to their denominators
 *
 * Parameters:
 *      int col:           a column of the image to analyze
 *      int row:           a row of the image to analyze
 *      Pnm_ppm image1:    the first image's Pnm_ppm struct
 *      Pnm_ppm image2:    the second image's Pnm_ppm struct
 *
 * Return: an integer representing the differences in each RGB pixel
 *         (each squared and then added)
 *
 * Notes:
 *      None
 ************************/
double squareDiff(int col, int row, Pnm_ppm image1, Pnm_ppm image2)
{
        Pnm_rgb pixel1 = image1->methods->at(image1->pixels, col, row);
        Pnm_rgb pixel2 = image2->methods->at(image2->pixels, col, row);

        /* Set the respective denominators */
        unsigned IDenominator = image1->denominator;
        unsigned IPDenominator = image2->denominator;

        /* Find the differences between */
        double redDiff = (((double)pixel1->red / IDenominator) - 
                          ((double)pixel2->red / IPDenominator));
                       
        double greenDiff = (((double)pixel1->green / IDenominator) - 
                            ((double)pixel2->green / IPDenominator));

        double blueDiff = (((double)pixel1->blue / IDenominator) - 
                           ((double)pixel2->blue / IPDenominator));

        /* Square the differences */
        return ((redDiff * redDiff) + 
                 (greenDiff * greenDiff) + 
                 (blueDiff * blueDiff));
}
