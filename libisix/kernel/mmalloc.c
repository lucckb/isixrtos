/*
 * =====================================================================================
 *
 *       Filename:  mmalloc.c
 *
 *    Description:  Isix standard kernel memory allocator
 *
 *        Version:  1.0
 *        Created:  16.06.2017 18:05:59
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Lucjan Bryndza (LB), lbryndza.p@boff.pl
 *   Organization:  BoFF
 *
 * =====================================================================================
 */
#include <isix/memory.h>
#include <isix/types.h>
#include <isix/semaphore.h>
#include <isix/prv/mutex.h>
#include <isix/config.h>
#define _ISIX_KERNEL_CORE_
#include <isix/prv/scheduler.h>

#ifdef CONFIG_ISIX_LOGLEVEL_MEMORY
#	undef CONFIG_ISIX_LOGLEVEL
#	define CONFIG_ISIX_LOGLEVEL CONFIG_ISIX_LOGLEVEL_MEMORY
#endif
#include <isix/prv/printk.h>


//! Select memory allocator
#ifndef CONFIG_ISIX_MEMORY_ALLOCATOR
#define CONFIG_ISIX_MEMORY_ALLOCATOR ISIX_MEMORY_ALLOCATOR_TLSF
#endif


#if CONFIG_ISIX_MEMORY_ALLOCATOR==ISIX_MEMORY_ALLOCATOR_SEQFIT
#	include <isix/prv/mm/malloc/seqfit.h>
#	define mm_alloc_init() _isixp_seqfit_alloc_init()
#	define mm_alloc(size) _isixp_seqfit_alloc(size)
#	define mm_free(ptr) _isixp_seqfit_free(ptr)
#	define mm_heap_stats(mminfo) _isixp_seqfit_heap_stats(mminfo)
#	define mm_realloc(ptr,size) _isixp_seqfit_realloc(ptr,size)
#	define mm_getsize(ptr) _isixp_seqfit_heap_getsize(ptr)
#elif CONFIG_ISIX_MEMORY_ALLOCATOR==ISIX_MEMORY_ALLOCATOR_TLSF
#	include <isix/prv/mm/malloc/tlsf.h>

static inline void mm_alloc_init()
{
	extern char __heap_start;
	extern char __heap_end;
	size_t ret = init_memory_pool(
			&__heap_end - &__heap_start,
			&__heap_start
	);
	if( ret == (size_t)-1 ) {
		isix_bug("Unable to create TSLF memory pool");
	}
}
static inline void* mm_alloc( size_t size )
{
	return tlsf_malloc( size );
}
static inline void mm_free( void* ptr )
{
	tlsf_free( ptr );
}
static inline void* mm_realloc(void *ptr, size_t size )
{
	return tlsf_realloc( ptr, size );
}
static inline size_t mm_getsize(void *ptr)
{
	return get_block_size(ptr);
}
static inline void mm_heap_stats( isix_memory_stat_t* meminfo )
{
	meminfo->used = get_used_size(NULL);
	meminfo->free = get_free_size(NULL);
	meminfo->fragments = 1U;
}

#endif /* CONFIG_ISIX_MEMORY_ALLOCATOR==ISIX_MEMORY_ALLOCATOR_ */




//! Semaphore for locking the memory allocator
static struct isix_mutex mlock;


//!Lock the memory
static void mem_lock_init(void)
{
	//Create unlocked semaphore
	if( !isix_mutex_create( &mlock ) ) {
		isix_bug("Memlock create failed");
	}
}

//!Lock the memory
static void mem_lock(void)
{
	if(schrun) {
		if( isix_mutex_lock( &mlock ) ) {
			isix_bug("Memlock lock failed");
		}
	}
}


//!Unlock the memory
static void mem_unlock(void)
{
	if(schrun) {
		if( isix_mutex_unlock( &mlock ) ) {
			isix_bug("Memlock unlock failed");
		}
	}
}



//! Initialize global heap
void _isixp_alloc_init(void)
{
	mem_lock_init();
	mm_alloc_init();
}


// Allocate memory
void* isix_alloc( size_t size )
{
	mem_lock();
	void* ptr = mm_alloc( size );
	mem_unlock();
	return ptr;
}


// Isix free memory
void isix_free( void* ptr )
{
	mem_lock();
	mm_free( ptr );
	mem_unlock();
}

//! Memory allocation stats
void isix_heap_stats( isix_memory_stat_t* meminfo )
{
	mem_lock();
	mm_heap_stats( meminfo );
	mem_unlock();
	//return siz;
}

//! Reallocate memory
void* isix_realloc(void *ptr, size_t size )
{
	mem_lock();
	void* mem = mm_realloc( ptr, size );
	mem_unlock();
	return mem;
}

size_t isix_heap_getsize( void* ptr )
{
	return mm_getsize( ptr );
}
