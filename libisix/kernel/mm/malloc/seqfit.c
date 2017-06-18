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

#define MAGIC 0x19790822
#define ALIGN_MASK      (ISIX_BYTE_ALIGNMENT_SIZE - 1)
#define ALIGN_SIZE(p)   (((size_t)(p) + ALIGN_MASK) & ~ALIGN_MASK)
#define abort() *((long*)(NULL)) = 0

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

} heap;


//! Initialize global heap
void _isixp_seqfit_alloc_init(void)
{
  struct header *hp;

  extern char __heap_start;
  extern char __heap_end;
  if( (uintptr_t)&__heap_start % ISIX_BYTE_ALIGNMENT_SIZE ) {
	  abort();
  }
  hp = (void *)&__heap_start;
  hp->h_size = &__heap_end - &__heap_start - sizeof(struct header);

  hp->h.h_next = NULL;
  heap.free.h.h_next = hp;
  heap.free.h_size = 0;
  heap.used = 0;
}


void* _isixp_seqfit_alloc(size_t size)
{
  struct header *qp, *hp, *fp;

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
      return (void *)(hp + 1);
    }
    qp = hp;
  }

  return NULL;
}


#define LIMIT(p) (struct header *)((char *)(p) + \
                                   sizeof(struct header) + \
                                   (p)->h_size)


void _isixp_seqfit_free(void *p)
{
  struct header *qp, *hp;

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
      return;
    }
    qp = qp->h.h_next;
  }
}


//Simple realloc implementation
void *_isixp_seqfit_realloc(void *ptr, size_t size )
{
	if( ptr == NULL ) {
		return _isixp_seqfit_alloc( size );
	}
	if( size == 0 ) {
		_isixp_seqfit_free( ptr );
		return NULL;
	}
	if( _isixp_seqfit_heap_getsize(ptr) >= size ) {
		return ptr;
	}
	void* mem = _isixp_seqfit_alloc( size );
	if( mem != NULL ) {
		memcpy( mem, ptr, size );
		_isixp_seqfit_free( ptr );
	}
	return mem;
}


void _isixp_seqfit_heap_stats( isix_memory_stat_t* meminfo )
{
	int frags = 0; size_t mem = 0;
	for(struct header *qp=&heap.free; qp;  qp=qp->h.h_next)
	{
		mem += qp->h_size;
		frags++;
	}
	meminfo->fragments = frags;
	meminfo->free = mem;
	meminfo->used = heap.used;
}

size_t _isixp_seqfit_heap_getsize( void* ptr )
{
	struct header* hp = (struct header *)ptr - 1;
	if( hp->h.h_magic != MAGIC ) { //Not dyn block
		return 0;
	} else {
		return hp->h_size;
	}
}

