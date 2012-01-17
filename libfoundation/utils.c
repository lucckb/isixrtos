/* ------------------------------------------------------------ */
/*
 * utils.c
 *
 *  Created on: 2010-03-26
 *      Author: lucck
 */

/* ------------------------------------------------------------ */
#include "utils.h"
#include <string.h>

/* ------------------------------------------------------------ */
//Convert ascii to unsigned int
unsigned fnd_atoui(const char *str)
{
    unsigned ret = 0;
    if(str==NULL || *str==0) return ret;
    //Move to end of string
    const char *s;
    for( s=str; *s; s++ );
    s--;
     for(unsigned w=1; s>=str ;s--,w*=10)
    {
        if(*s<'0' || *s>'9') break;
        ret += (*s-'0')*w;
    }
    return ret;
}


/* ------------------------------------------------------------ */
//Strev without allocate extra memory using xor
static inline void strrev(char *str, int len)
{
	char *p1, *p2;

	if (! str || ! *str)
	   return;
	for (p1 = str, p2 = str + len - 1; p2 > p1; ++p1, --p2)
	{
		*p1 ^= *p2;
		*p2 ^= *p1;
		*p1 ^= *p2;
	}
}
/* ------------------------------------------------------------ */
//Convert value to digit
static inline char base_to_digit( int digit )
{
	if( digit >=0 && digit <= 9)
		return digit + '0';
	else if( digit - 10 <= 'Z' - 'A')
		return digit - 10 + 'A';
	else
		return '.';
}

/* ------------------------------------------------------------ */
//Convert unsigned int to ascii
void fnd_uitoa(char *str, unsigned val ,int fmt, char fmtch, int base)
{
    int digits;
    if(fmt>16) return;
    for(digits=0; val>0; val/=base,digits++)
        str[digits] = base_to_digit(val % base);
    for(int i=fmt-digits; i>0; i--,digits++)
        str[digits] = fmtch;
    str[digits] = '\0';
    strrev(str,digits);
}

/* ------------------------------------------------------------ */
