#ifndef __ASM_RTC_H
#define __ASM_RTC_H

#include <isix/config.h>
#include <isix/types.h>
#include <asm/lpc214x.h>

/*------------------------------------*/
//Get microsecond from time
static inline u32 get_usec(void)
{
    return T0TC;
}
/*------------------------------------*/

#endif
