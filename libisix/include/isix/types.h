#ifndef _ISIX_TYPES_H
#define _ISIX_TYPES_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <isix/error.h>

/*-----------------------------------------------------*/
//! Null defs
#ifndef NULL
#define NULL ((void*)0)
#endif /* NULL */

/*-----------------------------------------------------*/
#ifdef __cplusplus
namespace isix
{
#endif
/*-----------------------------------------------------*/

//! Define tick type
typedef unsigned int tick_t;

//! Define a micro tick
typedef unsigned long utick_t;
/*-----------------------------------------------------*/
#ifdef __cplusplus
}
#endif
/*-----------------------------------------------------*/
//! Bit shift
#define _BV(x) (1<<x)

/*-----------------------------------------------------*/

#endif /* __ISIX_TYPES_H */
