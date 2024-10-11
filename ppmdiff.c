

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

// static void (*diff)(FILE *input);
static FILE *open_or_abort(char *fname, char *mode); 
double squareDiff(int col, int row, Pnm_ppm image1, Pnm_ppm image2, 
                  A2Methods_T methods);

int main(int argc, char *argv[])
{
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
        } else if (stdinIndex == 2) {
                FILE *fp = open_or_abort(argv[1], "r");
                I = Pnm_ppmread(fp, methods);
                IPrime = Pnm_ppmread(stdin, methods);
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
        int imageHeight = (I->height > IPrime->height) ? IPrime->height : 
                                                         I->height;
        int imageWidth = (I->width > IPrime->width) ? IPrime->width : 
                                                      I->width;

        int row = 0;
        int col = 0;
        for (col = 0; col < imageWidth; col++) {
                if (col >= imageWidth) {
                        break;
                }
                for (row = 0; row < imageHeight; row++) {
                        if (row >= imageHeight) {
                                break;
                        }
                        printf("got here\n");
                        sum += squareDiff(col, row, I, IPrime, methods);
                        printf("sum is %f\n", sum);
                }
                printf("out of here sum is %f\n", sum);
        }
        printf("out of HEREEE sum is %f\n", sum);

        // fprintf(stderr, "Final sum: %f\n", sum);

        double inner;
        inner = (sum / (3.0 * imageHeight * imageWidth));
        //fprintf(stderr, "width: %d, height: %d\n", imageWidth, imageHeight);

        double result;
        result = sqrt(inner);
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
        if (input == NULL) {
                assert(false);
        }
        
        /* Return the opened file */
        return input;
}


double squareDiff(int col, int row, Pnm_ppm image1, Pnm_ppm image2, 
                  A2Methods_T methods)
{
        Pnm_rgb pixel1 = methods->at(image1->pixels, col, row);
        Pnm_rgb pixel2 = methods->at(image2->pixels, col, row);

        int redDiff = (pixel1->red - pixel2->red);
        int greenDiff = (pixel1->green - pixel2->green);
        int blueDiff = (pixel1->blue - pixel2->blue);

        /* Square the differences */
        return ((redDiff * redDiff + 
                   greenDiff * greenDiff + 
                   blueDiff * blueDiff));
}
