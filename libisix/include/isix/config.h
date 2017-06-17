/* 
    Isix configuration file (C) 2007 L. Bryndza
*/
#pragma once

/** Memory protection model  */
#define ISIX_MPROT_NONE 0
#define ISIX_MPROT_LITE 1
#define ISIX_MPROT_FULL 2


/** Default memory allocator */
#define ISIX_MEMORY_ALLOCATOR_TLSF		1
#define ISIX_MEMORY_ALLOCATOR_SEQFIT	2

/** Default isix alignemnt size */
#define ISIX_BYTE_ALIGNMENT_SIZE (2*sizeof(void*))

#include <isixconf.h>


