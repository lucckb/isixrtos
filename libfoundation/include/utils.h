/* ------------------------------------------------------------ */
/*
 * utils.h
 *
 *  Created on: 2010-03-26
 *      Author: lucck
 */
/* ------------------------------------------------------------ */
#ifndef UTILS_H_
#define UTILS_H_
/* ------------------------------------------------------------ */
#ifdef __cplusplus
namespace fnd {
extern "C" {
#endif
/* ------------------------------------------------------------ */
//Convert string to unsigned
unsigned fnd_atoui(const char *str);

/* ------------------------------------------------------------ */
//Convert unsigned int to ascii
void fnd_uitoa(char *str, unsigned val , int fmt, char fmtch, int base);
/* ------------------------------------------------------------ */
//Convert int to ascii
static inline void fnd_itoa(char *str, int val , int fmt, char fmtch )
{
	if(val < 0 )
	{
		val = -val;
		*str++ = '-';
	}
	fnd_uitoa( str, val, fmt, fmtch, 10 );
}

/* ------------------------------------------------------------ */
#ifdef __cplusplus
 }
 }
#endif
/* ------------------------------------------------------------ */
#endif /* UTILS_H_ */
