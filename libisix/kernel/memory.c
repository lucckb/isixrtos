/*------------------------------------------------------*/
/*
 * memory.c
 *  New heap allocator for the ISIX
 *  Created on: 2009-11-11
 *      Author: lucck
 */
/*------------------------------------------------------*/
#include <isix/memory.h>
#include <isix/types.h>
#include <isix/semaphore.h>
#include <prv/semaphore.h>
#include <prv/scheduler.h>
#include <isix/config.h>

#ifndef ISIX_DEBUG_MEMORY
#define ISIX_DEBUG_MEMORY ISIX_DBG_OFF
#endif


#if ISIX_DEBUG_MEMORY == ISIX_DBG_ON
#include <isix/printk.h>
#else
#undef isix_printk
#define isix_printk(...) do {} while(0)
#endif

/*------------------------------------------------------*/

#define MAGIC 0x19790822
#define ALIGN_MASK      (ISIX_CONFIG_BYTE_ALIGNMENT_SIZE - 1)
#define ALIGN_SIZE(p)   (((size_t)(p) + ALIGN_MASK) & ~ALIGN_MASK)

struct header
{
  union
  {
    struct header       *h_next;
    size_t              h_magic;
  } h;
  size_t                h_size;
};
/*------------------------------------------------------*/
static struct
{
  struct header         free;   /* Guaranteed to be not adjacent to the heap */

} heap;

/*------------------------------------------------------*/
//! Semaphore for locking the memory allocator
static sem_t mem_sem;


/*------------------------------------------------------*/
//!Lock the memory
static void mem_lock_init(void)
{
	//Create unlocked semaphore
	isix_sem_create( &mem_sem, 1 );
}
/*------------------------------------------------------*/
//!Lock the memory
static void mem_lock(void)
{
	if(isix_scheduler_running)
		isix_sem_wait( &mem_sem, ISIX_TIME_INFINITE );
}

/*------------------------------------------------------*/
//!Unlock the memory
static void mem_unlock(void)
{
	if(isix_scheduler_running)
		isix_sem_signal( &mem_sem );
}

/*------------------------------------------------------*/
//! Initialize global heap
void isix_alloc_init(void)
{
  struct header *hp;

  extern char __heap_start;
  extern char __heap_end;

  mem_lock_init();

  hp = (void *)&__heap_start;
  hp->h_size = &__heap_end - &__heap_start - sizeof(struct header);

  hp->h.h_next = NULL;
  heap.free.h.h_next = hp;
  heap.free.h_size = 0;

}

/*------------------------------------------------------*/
void *isix_alloc(size_t size)
{
  struct header *qp, *hp, *fp;

  size = ALIGN_SIZE(size);
  qp = &heap.free;
  mem_lock();

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

      mem_unlock();
      return (void *)(hp + 1);
    }
    qp = hp;
  }

  mem_unlock();
  return NULL;
}

/*------------------------------------------------------*/
#define LIMIT(p) (struct header *)((char *)(p) + \
                                   sizeof(struct header) + \
                                   (p)->h_size)

/*------------------------------------------------------*/
void isix_free(void *p)
{
  struct header *qp, *hp;


  hp = (struct header *)p - 1;
  /*chDbgAssert(hp->h_magic == MAGIC,
              "chHeapFree(), #1",
              "it is not magic"); */
  qp = &heap.free;
  mem_lock();

  while (1) {

/*    chDbgAssert((hp < qp) || (hp >= LIMIT(qp)),
                "chHeapFree(), #2",
                "within free block"); */

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

      mem_unlock();
      return;
    }
    qp = qp->h.h_next;
  }
  mem_unlock();
}
/*------------------------------------------------------*/
size_t isix_heap_free(int *fragments)
{
	int frags = 0; size_t mem = 0;
	mem_lock();
	for(struct header *qp=&heap.free; qp;  qp=qp->h.h_next)
	{
		mem += qp->h_size;
		frags++;
	}
	mem_unlock();
	if(fragments)
		*fragments = frags;
	return mem;
}
/*------------------------------------------------------*/
