/*
*      bitpack.c
*      by Peter Morganelli and Shepard Rodgers, 10/22/24
*      arith assignment
*
*      This file contains the implementation for the bitpack 
*      abstraction with functions to create, get, and check if a bit can be
*      fit in n
*/

#include "bitpack.h"
#include "assert.h"
#include "except.h"


/* Define a global variable for a 64-bit word */
const unsigned WORD_SIZE = 64;

/* Define a message to be thrown in the case of a bitpack overflow */
Except_T Bitpack_Overflow = { "Overflow packing bits" };

/* Our helper functions for shifting */
uint64_t unsignedRightShift(uint64_t n, unsigned width);
int64_t signedRightShift(int64_t value, unsigned shift);
uint64_t leftShift(uint64_t value, unsigned shift);


/********** Bitpack_fitsu ********
 *
 *  To see if a 64-bit unsigned integer can be represented in width bits
 *
 * Parameters:
 *      uint64_t n: the 64-bit unsigned integer we are checking
 *      unsigned width: the number n is checked against to see if it can fit
 *
 * Return: 
 *      True if n can fit in width bits, false otherwise
 *
 * Expects
 *     none
 * 
 * Notes:
 *      Will return false if width is 0
 *      Will return true if width is greater than 64
 *      
 ************************/
bool Bitpack_fitsu(uint64_t n, unsigned width)
{
        if (width == 0) {
                return false;
        }
        
        if (width > WORD_SIZE) {
                return true;
        }
        /* Min of an unsigned is 0 */
        uint64_t min = (uint64_t)0;

        /* Max of an unsigned is 2^(width) - 1 */
        uint64_t max = leftShift((uint64_t)1, width) - 1;

        /* See if n is within this range */
        bool fits = (n <= max && n >= min);
        return fits;
}

/********** Bitpack_fitss ********
 *
 *  To see if a 64-bit signed integer can be represented in width bits
 *
 * Parameters:
 *      int64_t n:      the 64-bit signed integer we are checking
 *      unsigned width: the number n is checked against to see if it can fit
 *
 * Return: 
 *      True if n can fit in width bits, false otherwise
 *
 * Expects
 *      Nine
 * 
 * Notes:
 *      Will return false if width is 0
 *      Will return true if width is greater than 64
 *      
 ************************/
bool Bitpack_fitss(int64_t n, unsigned width)
{
        if (width == 0) {
                return false;
        }

        if (width > WORD_SIZE) {
                return true;
        }
        
        /* Expecting Min = -2^(width - 1) */
        int64_t min = -(int64_t)leftShift((uint64_t)1, width - 1);

        /* Expecting Max = 2^(width - 1) - 1 */
        int64_t max = (int64_t)leftShift((uint64_t)1, width - 1) - 1;

        /* See if n is within this range */
        bool fits = (n <= max && n >= min);
        return fits;
}

/********** Bitpack_getu ********
 *
 *  To extract an unsigned value from the given codeword
 *
 * Parameters:
 *      uint64_t word:  the codeword to be extracted from
 *      unsigned width: the width of the value we want
 *      unsigned lsb:   the index of the least significant bit of the value
 *                      we want in word
 *
 * Return: 
 *      An unsigned representation of the value we are looking to extract
 *
 * Expects
 *      Width + height >= 64
 *      Width <= 64
 * 
 * Notes:
 *      Will CRE if width + height is not >= 64
 *      Will CRE if width is not <= 64
 *      
 ************************/
uint64_t Bitpack_getu(uint64_t word, unsigned width, unsigned lsb)
{
        /* Assert our expectations */
        assert(width + lsb <= WORD_SIZE);
        assert(width <= WORD_SIZE);

        /* Create our mask with all 1's */
        uint64_t mask = ~0;
        /* Left shift mask by 64 - width to propogate 0's */
        mask = leftShift(mask, WORD_SIZE - width);

        /* Right shift mask by 64 - (lsb + width) to propogate 0's 
           on other sise */
        mask = unsignedRightShift(mask, WORD_SIZE - (lsb + width));
        
        /* Bitwise and the mask with the word to update the value */
        uint64_t value = mask & word;

        /* Right shift value by LSB to finish this process */
        value = unsignedRightShift(value, lsb);
        
        return value;
}

/********** Bitpack_gets ********
 *
 *  To extract a signed value from the given codeword
 *
 * Parameters:
 *      uint64_t word:  the codeword to be extracted from
 *      unsigned width: the width of the value we want
 *      unsigned lsb:   the index of the least significant bit of the value
 *                      we want in word
 *
 * Return: 
 *      A signed representation of the value we are looking to extract
 *
 * Expects
 *      Width + height >= 64
 *      Width <= 64
 * 
 * Notes:
 *      Will CRE if width + height is not >= 64
 *      Will CRE if width is not <= 64
 *      
 ************************/
int64_t Bitpack_gets(uint64_t word, unsigned width, unsigned lsb)
{
        /* Assert our expectations */
        assert(width + lsb <= WORD_SIZE);
        assert(width <= 64);
        
        /* Perform getu and store result as a signed int */
        int64_t value = (int64_t)Bitpack_getu(word, width, lsb);

        /* Perform a left and then right shifts to update our value */
        value = leftShift(value, (WORD_SIZE - width));
        value = signedRightShift(value, (WORD_SIZE - width));
        
        return value;
}

/********** Bitpack_newu ********
 *
 *  To insert an unsigned value into a given word
 *
 * Parameters:
 *      uint64_t word:  the word to be added to
 *      unsigned width: the width of the value we want
 *      unsigned lsb:   the index of the least significant bit of the value
 *                      we want in word
 *      uint64_t value: the value we want to add
 * 
 *
 * Return: 
 *      The word that was passed in with the added value
 *
 * Expects
 *      Width + height >= 64
 *      Width <= 64
 * 
 * Notes:
 *      Will CRE if width + height is not >= 64
 *      Will CRE if width is not <= 64
 *      
 ************************/
uint64_t Bitpack_newu(uint64_t word, unsigned width, unsigned lsb, 
                      uint64_t value)
{
        /* Assert our parameters */
        assert(width + lsb <= WORD_SIZE);
        assert(width <= 64);

        /* If Bitpack_fitsu fails, bitpack overflow should be thrown */
        if (!Bitpack_fitsu(value, width)) {
                RAISE(Bitpack_Overflow);
        }
        
        /* Create our high mask */
        uint64_t maskHigh = ~(uint64_t)0;
        maskHigh = leftShift(maskHigh, (lsb + width));

        /* Create our low mask */
        uint64_t maskLow = ~(uint64_t)0;
        maskLow = unsignedRightShift(maskLow, (WORD_SIZE - lsb));
        
        /* Bitwise or the two masks to get our actual mask */
        uint64_t mask = maskHigh | maskLow;

        /* Bitwise and the word with teh mask to get a cleared spot for where
           we want value to be compared */
        uint64_t clearedValue = word & mask;

        /* Shift value by LSB to align with the cleared value */
        uint64_t shiftedValue = leftShift(value, lsb);

        /* Bitwise or the shifted value and the cleared value to get our
           updated word */
        uint64_t newWord = shiftedValue | clearedValue;
        return newWord;
}

/********** Bitpack_news ********
 *
 *  To insert a signed value into a given word
 *
 * Parameters:
 *      uint64_t word:  the word to be added to
 *      unsigned width: the width of the value we want
 *      unsigned lsb:   the index of the least significant bit of the value
 *                      we want in word
 *      int64_t value: the value we want to add
 *
 * Return: 
 *      The word that was passed in with the added value
 *
 * Expects
 *      Width + height >= 64
 *      Width <= 64
 * 
 * Notes:
 *      Will CRE if width + height is not >= 64
 *      Will CRE if width is not <= 64
 *      
 ************************/
uint64_t Bitpack_news(uint64_t word, unsigned width, unsigned lsb, 
                      int64_t value)
{
        /* Assert our expectations */
        assert(width + lsb <= WORD_SIZE);
        assert(width <= 64);

        /* If the value doesn't fit in width, raise a bitpack overflow */
        if (!Bitpack_fitss(value, width)) {
                RAISE(Bitpack_Overflow);
        }

        /* Cast our value to an unsigned int */
        uint64_t uValue = (uint64_t)value;

        /* Left shift this value by word_size - width to propogate 0's */
        uValue = leftShift(uValue, (WORD_SIZE - width));

        /* Right shift this value by the same amount to propogate 0's 
           on other side*/
        uValue = unsignedRightShift(uValue, (WORD_SIZE - width));

        /* Call Bitpack_newu on this value and store the result */
        uValue = Bitpack_newu(word, width, lsb, uValue);
        
        return uValue;
}       

/********** unsignedRightShift ********
 *
 *  A helper function to right shift an unsigned value by the 
 *  specified shift amount
 *
 * Parameters:
 *      uint64_t value: the value we want to shift
 *      unsigned shift: the amount we want to shift value to the right
 *
 * Return: 
 *      The value after it has been shifted
 *
 * Expects
 *      none
 * 
 * Notes:
 *      none
 *      
 ************************/
uint64_t unsignedRightShift(uint64_t value, unsigned shift)
{
        /* If the shift is greater than 64, return all 0s */
        if (shift >= WORD_SIZE) {
                uint64_t clearedBits = (uint64_t)0;
                return clearedBits;
        }
        /* Otherwise, perform the right shift and store in an unsigned */
        uint64_t result = value >> shift;
        return result;
}

/********** signedRightShift ********
 *
 *  A helper function to right shift a signed value by the 
 *  specified shift amount
 *
 * Parameters:
 *      uint64_t value: the value we want to shift
 *      unsigned shift: the amount we want to shift value to the right
 *
 * Return: 
 *      The value after it has been shifted
 *
 * Expects
 *      none
 * 
 * Notes:
 *      none
 *      
 ************************/
int64_t signedRightShift(int64_t value, unsigned shift)
{
        /* If the shift is greater than 64, return all 0s */
        if (shift >= WORD_SIZE) {
                uint64_t clearedBits = (uint64_t)0;
                return clearedBits;
        }
        /* Otherwise, perform the right shift and store in a signed */
        int64_t result = (value >> shift);
        return result;
}

/********** leftShift ********
 *
 *  A helper function to left shift a value (signed OR unsigned) by the 
 *  specified shift amount
 *
 * Parameters:
 *      uint64_t value: the value we want to shift
 *      unsigned shift: the amount we want to shift value to the right
 *
 * Return: 
 *      The value after it has been shifted
 *
 * Expects
 *      none
 * 
 * Notes:
 *      none
 *      
 ************************/
uint64_t leftShift(uint64_t value, unsigned shift)
{
        /* If the shift is greater than 64, return all 0s */
        if (shift >= WORD_SIZE) {
                uint64_t clearedBits = (uint64_t)0;
                return clearedBits;
        }
        /* Otherwise, perform the left shift */
        uint64_t result = value << shift;
        return result;
}
