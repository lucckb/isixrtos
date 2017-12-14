
/*
 * utils.c
 *
 *  Created on: 2010-03-26
 *      Author: lucck
 */


#include "foundation/algo/utils.h"
#include <string.h>


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


//Convert unsigned int to ascii
void fnd_uitoa(char *str, unsigned val ,int fmt, char fmtch, int base)
{
    int digits;
	char z = val==0;
    if(fmt>16) return;
    for(digits=0; val>0; val/=base,digits++)
        str[digits] = base_to_digit(val % base);
    for(int i=fmt-digits; i>0; i--,digits++)
        str[digits] = fmtch;
    str[digits] = '\0';
    strrev(str,digits);
	if( z && str[digits-1]!='0' ) 
		str[digits-1] = '0';
}

//Convert unsigned int to ascii
void fnd_ulltoa(char *str, unsigned long long val ,int fmt, char fmtch, int base)
{
    int digits;
	char z = val==0;
    if(fmt>16) return;
    for(digits=0; val>0; val/=base,digits++)
        str[digits] = base_to_digit(val % base);
    for(int i=fmt-digits; i>0; i--,digits++)
        str[digits] = fmtch;
    str[digits] = '\0';
    strrev(str,digits);
	if( z && str[digits-1]!='0' )
		str[digits-1] = '0';
}

static int hex2num(char c)
{
	if (c >= '0' && c <= '9')
		return c - '0';
	if (c >= 'a' && c <= 'f')
		return c - 'a' + 10;
	if (c >= 'A' && c <= 'F')
		return c - 'A' + 10;
	return -1;
}

static int hex2byte(const char *hex)
{
        int a, b;
        a = hex2num(*hex++);
        if (a < 0)
			return -1;
        b = hex2num(*hex++);
        if (b < 0)
			return -1;
        return (a << 4) | b;
}

int fnd_hexstr2bin(const char *hex, unsigned char *buf, size_t len)
{
	size_t i;
	int a;
	const char *ipos = hex;
	unsigned char *opos = buf;

	for (i = 0; i < len; i++) {
		a = hex2byte(ipos);
		if (a < 0)
			return -1;
		*opos++ = a;
		ipos += 2;
	}
	return 0;
}


char* fnd_bin2hexstr( const unsigned char* bin, char *buf, size_t bytes)
{
	const char* const hex = "0123456789abcdef";
	for (const unsigned char* binEnd = bin + bytes; bin != binEnd; ++bin)
	{
			*buf++ = hex[*(unsigned char*)bin >> 4];
			*buf++ = hex[*bin & 0xf];
	}
	*buf = '\0';
	return buf;
}
