#ifndef __ISIX_MEMORY_H
#define __ISIX_MEMORY_H

#include <isix/types.h>
/*------------------------------------------------------*/
//Allocate memory
void* isix_alloc(size_t size);

/*------------------------------------------------------*/
//Free memory
void isix_free(void *mem);

/*------------------------------------------------------*/
//! Initialize global heap
void isix_alloc_init(void);

/*------------------------------------------------------*/

#endif
