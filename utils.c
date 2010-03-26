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
