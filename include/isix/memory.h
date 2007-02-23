#ifndef __ISIX_MEMORY_H
#define __ISIX_MEMORY_H

#include <isix/types.h>
/*------------------------------------------------------*/
//Allocate memory
void* kmalloc(size_t size);

/*------------------------------------------------------*/
//Free memory
void kfree(void *mem);

/*------------------------------------------------------*/
//Zero of selected memory region
void zero_memory(void *s, size_t n);

/*------------------------------------------------------*/
#endif
