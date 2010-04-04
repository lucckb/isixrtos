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
static void strrev(char *str, int len)
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
void fnd_uitoa(char *str, unsigned val ,int fmt)
{
    int digits;
    if(fmt>10) return;
    for(digits=0; val>0; val/=10,digits++)
        str[digits] = val % 10 + '0';
    for(int i=fmt-digits; i>0; i--,digits++)
        str[digits] = '0';
    str[digits] = '\0';
    strrev(str,digits);
}

/* ------------------------------------------------------------ */
