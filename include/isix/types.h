#ifndef __ISIX_TYPES_H
#define __ISIX_TYPES_H

#include <asm/types.h>
#include <stddef.h>

/*-----------------------------------------------------*/
//Null defs
#ifndef NULL
#define NULL ((void*)0)
#endif /* NULL */

/*-----------------------------------------------------*/
//Bool defs
typedef u8 bool;

#define true 1
#define false 0

/*-----------------------------------------------------*/
//sys timer defs
typedef u64 time_t;

/*-----------------------------------------------------*/

#endif /* __ISIX_TYPES_H */
