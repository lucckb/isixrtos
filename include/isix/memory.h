#ifndef __ISIX_MEMORY_H
#define __ISIX_MEMORY_H

#include <isix/types.h>

void* kmalloc(size_t size);

void kfree(void *mem);



#endif
