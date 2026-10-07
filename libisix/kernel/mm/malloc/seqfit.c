/*
 * memory.c
 *  New heap allocator for the ISIX
 *  Created on: 2009-11-11
 *      Author: lucck
 */
#include <stddef.h>
#include <isix/config.h>
#include <string.h>
#include <isix/prv/mm/malloc/seqfit.h>
#include <isix/memory.h>
#include "seqfit_config.h"

#define MAGIC 0x19790822
#define ALIGN_MASK      (ISIX_BYTE_ALIGNMENT_SIZE - 1)
#define ALIGN_SIZE(p)   (((size_t)(p) + ALIGN_MASK) & ~ALIGN_MASK)
#define abort() *((long*)(NULL)) = 0


#if !SEQFIT_LOCK
#	define TLSF_CREATE_LOCK(_unused_)   do{}while(0)
#	define TLSF_DESTROY_LOCK(_unused_)  do{}while(0)
#	define TLSF_ACQUIRE_LOCK(_unused_)  do{}while(0)
#	define TLSF_RELEASE_LOCK(_unused_)  do{}while(0)
#endif

struct header
{
  union
  {
    struct header       *h_next;
    size_t              h_magic;
  } h;
  size_t                h_size;
};

static struct
{
	struct header         free;   /* Guaranteed to be not adjacent to the heap */
	size_t 				  used;	  /* Total used memory block */
#ifdef SEQFIT_MLOCK_T
	SEQFIT_MLOCK_T		  lock;	  /* Global memory lock */
#endif
} heap;


//! Initialize global heap
void seqfit_alloc_init(void)
{
  struct header *hp;

  extern unsigned char __heap_start[], __heap_end[];
  if( (uintptr_t)__heap_start % ISIX_BYTE_ALIGNMENT_SIZE ) {
	  abort();
  }
  hp = (void *)__heap_start;
  hp->h_size = __heap_end - __heap_start - sizeof(struct header);

  hp->h.h_next = NULL;
  heap.free.h.h_next = hp;
  heap.free.h_size = 0;
  heap.used = 0;
  SEQFIT_CREATE_LOCK( &heap.lock );
}


void* seqfit_alloc(size_t size)
{
  struct header *qp, *hp, *fp;

  SEQFIT_ACQUIRE_LOCK( &heap.lock );
  size = ALIGN_SIZE(size);
  qp = &heap.free;

  while (qp->h.h_next != NULL) {
    hp = qp->h.h_next;
    if (hp->h_size >= size) {
      if (hp->h_size < size + sizeof(struct header)) {
        /* Gets the whole block even if it is slightly bigger than the
           requested size because the fragment would be too small to be
           useful */
        qp->h.h_next = hp->h.h_next;
      }
      else {
        /* Block bigger enough, must split it */
        fp = (void *)((char *)(hp) + sizeof(struct header) + size);
        fp->h.h_next = hp->h.h_next;
        fp->h_size = hp->h_size - sizeof(struct header) - size;
        qp->h.h_next = fp;
        hp->h_size = size;
      }
      hp->h.h_magic = MAGIC;
	  heap.used += size;
	  SEQFIT_RELEASE_LOCK( &heap.lock );
      return (void *)(hp + 1);
    }
    qp = hp;
  }
  SEQFIT_RELEASE_LOCK( &heap.lock );
  return NULL;
}


#define LIMIT(p) (struct header *)((char *)(p) + \
                                   sizeof(struct header) + \
                                   (p)->h_size)


void seqfit_free(void *p)
{
  struct header *qp, *hp;

  if (p == NULL) {
    return;
  }
  SEQFIT_ACQUIRE_LOCK( &heap.lock );
  hp = (struct header *)p - 1;
  qp = &heap.free;
  const size_t block_siz = hp->h_size;
  while (1) {

    if (((qp == &heap.free) || (hp > qp)) &&
        ((qp->h.h_next == NULL) || (hp < qp->h.h_next))) {
      /* Insertion after qp */
      hp->h.h_next = qp->h.h_next;
      qp->h.h_next = hp;
      /* Verifies if the newly inserted block should be merged */
      if (LIMIT(hp) == hp->h.h_next) {
        /* Merge with the next block */
        hp->h_size += hp->h.h_next->h_size + sizeof(struct header);
        hp->h.h_next = hp->h.h_next->h.h_next;
      }
      if ((LIMIT(qp) == hp)) {  /* Cannot happen when qp == &heap.free */
        /* Merge with the previous block */
        qp->h_size += hp->h_size + sizeof(struct header);
        qp->h.h_next = hp->h.h_next;
      }
	  heap.used -= block_siz;
	  SEQFIT_RELEASE_LOCK( &heap.lock );
      return;
    }
    qp = qp->h.h_next;
  }
  SEQFIT_RELEASE_LOCK( &heap.lock );
}


//Simple realloc implementation
void* seqfit_realloc(void *ptr, size_t size )
{
	if( ptr == NULL ) {
		return seqfit_alloc( size );
	}
	if( size == 0 ) {
		seqfit_free( ptr );
		return NULL;
	}
	const size_t old_size = seqfit_heap_getsize(ptr);
	if( old_size >= size ) {
		return ptr;
	}
	void* mem = seqfit_alloc( size );
	if( mem != NULL ) {
		memcpy( mem, ptr, old_size );
		seqfit_free( ptr );
	}
	return mem;
}


void seqfit_heap_stats( isix_memory_stat_t* meminfo )
{
    SEQFIT_ACQUIRE_LOCK( &heap.lock );
	int frags = 0; size_t mem = 0;
	for(struct header *qp=&heap.free; qp;  qp=qp->h.h_next)
	{
		mem += qp->h_size;
		frags++;
	}
	meminfo->fragments = frags;
	meminfo->free = mem;
	meminfo->used = heap.used;
    SEQFIT_RELEASE_LOCK( &heap.lock );
}

size_t seqfit_heap_getsize( void* ptr )
{
	struct header* hp = (struct header *)ptr - 1;
	if( hp->h.h_magic != MAGIC ) { //Not dyn block
		return 0;
	} else {
		return hp->h_size;
	}
}


#if CONFIG_ISIX_TEST_HOOKS
#include <isix/prv/test_hooks.h>

int _isixp_heap_check(void)
{
	extern unsigned char __heap_start[], __heap_end[];
	int ret = 0;
	SEQFIT_ACQUIRE_LOCK( &heap.lock );
	const struct header *prev = NULL;
	for (const struct header *p = heap.free.h.h_next; p; p = p->h.h_next) {
		const unsigned char *beg = (const unsigned char *)p;
		const unsigned char *end = (const unsigned char *)LIMIT(p);
		if (beg < __heap_start || end > __heap_end || end < beg) {
			ret = -1;
		} else if (p->h_size & ALIGN_MASK) {
			ret = -2;
		} else if (prev && (const unsigned char *)p <= (const unsigned char *)prev) {
			ret = -3;
		} else if (prev && (const unsigned char *)LIMIT(prev) > beg) {
			ret = -4;
		} else if (prev && LIMIT(prev) == p) {
			ret = -5;
		}
		if (ret) {
			break;
		}
		prev = p;
	}
	SEQFIT_RELEASE_LOCK( &heap.lock );
	return ret;
}
#endif
