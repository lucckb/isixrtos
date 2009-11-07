#ifndef __ISIX_TYPES_H
#define __ISIX_TYPES_H

#include <asm/types.h>
#include <stddef.h>
#include <isix/error.h>

/*-----------------------------------------------------*/
//Null defs
#ifndef NULL
#define NULL ((void*)0)
#endif /* NULL */

/*-----------------------------------------------------*/
//Bit shift 
#define _BV(x) (1<<x)

/*-----------------------------------------------------*/

#endif /* __ISIX_TYPES_H */
