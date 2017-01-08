
/*
 * utils.h
 *
 *  Created on: 2010-03-26
 *      Author: lucck
 */

#pragma once
#include <stddef.h>

#ifdef __cplusplus
namespace fnd {
extern "C" {
#endif

//! Convert string to unsigned
unsigned fnd_atoui(const char *str);


//Convert unsigned int to ascii
void fnd_uitoa(char *str, unsigned val , int fmt, char fmtch, int base);
void fnd_ulltoa(char *str, unsigned long long val , int fmt, char fmtch, int base);

//Convert int to ascii
static inline void fnd_itoa(char *str, int val , int fmt, char fmtch )
{
	if(val < 0 ) {
		val = -val;
		*str++ = '-';
	}
	fnd_uitoa( str, val, fmt, fmtch, 10 );
}


/** \brief convert a floating point number to char buffer with a
 *         variable-precision format, and no trailing zeros
 *
 * This is similar to "%.[0-9]f" in the printf style, except it will
 * NOT include trailing zeros after the decimal point.  This type
 * of format oddly does not exists with printf.
 *
 * If the input value is greater than 1<<31, then the output format
 * will be switched exponential format.
 *
 * \param[in] value
 * \param[out] buf  The allocated output buffer.  Should be 32 chars or more.
 * \param[in] precision  Number of digits to the right of the decimal point.
 */
void fnd_dtoa(double value, char* str, int prec);
//Float ver
void fnd_ftoa(float value, char* str, int prec);

/** Convert hex string to binary buffer
 * @param[in] hex Input hex string buffer
 * @param[out] buf Output buffer 
 * @param[in]  len Buffer len
 * @return 0 if sucessful */
int fnd_hexstr2bin(const char *hex, unsigned char *buf, size_t len);


/** Convert binary buffer to hex string
 * @param[in] bin Binary buffer for input
 * @param[out] buf Binary buffer for output
 * @param[in] len Input length
   @return Buffer to the end of string
*/
char* fnd_bin2hexstr( const unsigned char* bin, char *buf, size_t len);


#ifdef __cplusplus
 }}
#endif

