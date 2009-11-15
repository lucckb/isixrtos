/*------------------------------------------------------*/
/*
 * memory.c
 *  New heap allocator for the ISIX
 *  Created on: 2009-11-11
 *      Author: lucck
 */
/*------------------------------------------------------*/
//TODO: memory should not block interrupts
#include <isix/memory.h>
#include <isix/types.h>
#include <prv/scheduler.h>

#ifndef ISIX_DEBUG_MEMORY
#define ISIX_DEBUG_MEMORY ISIX_DBG_OFF
#endif


#if ISIX_DEBUG_MEMORY == ISIX_DBG_ON
#include <isix/printk.h>
#else
#define printk(...)
#endif

/*------------------------------------------------------*/

#define MAGIC 0x19790822
#define ALIGN_TYPE      void *
#define ALIGN_MASK      (sizeof(ALIGN_TYPE) - 1)
#define ALIGN_SIZE(p)   (((size_t)(p) + ALIGN_MASK) & ~ALIGN_MASK)

struct header
{
  union
  {
    struct header       *h_next;
    size_t              h_magic;
  };
  size_t                h_size;
};
/*------------------------------------------------------*/
static struct
{
  struct header         free;   /* Guaranteed to be not adjacent to the heap */

} heap;

/*------------------------------------------------------*/
//! Initialize global heap
void isix_alloc_init(void)
{
  struct header *hp;

  extern char __heap_start;
  extern char __heap_end;

  hp = (void *)&__heap_start;
  hp->h_size = &__heap_end - &__heap_start - sizeof(struct header);

  hp->h_next = NULL;
  heap.free.h_next = hp;
  heap.free.h_size = 0;

}

/*------------------------------------------------------*/
void *isix_alloc(size_t size)
{
  struct header *qp, *hp, *fp;

  size = ALIGN_SIZE(size);
  qp = &heap.free;
  isixp_enter_critical();

  while (qp->h_next != NULL) {
    hp = qp->h_next;
    if (hp->h_size >= size) {
      if (hp->h_size < size + sizeof(struct header)) {
        /* Gets the whole block even if it is slightly bigger than the
           requested size because the fragment would be too small to be
           useful */
        qp->h_next = hp->h_next;
      }
      else {
        /* Block bigger enough, must split it */
        fp = (void *)((char *)(hp) + sizeof(struct header) + size);
        fp->h_next = hp->h_next;
        fp->h_size = hp->h_size - sizeof(struct header) - size;
        qp->h_next = fp;
        hp->h_size = size;
      }
      hp->h_magic = MAGIC;

      isixp_exit_critical();
      return (void *)(hp + 1);
    }
    qp = hp;
  }

  isixp_exit_critical();
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
  isixp_enter_critical();

  while (1) {

/*    chDbgAssert((hp < qp) || (hp >= LIMIT(qp)),
                "chHeapFree(), #2",
                "within free block"); */

    if (((qp == &heap.free) || (hp > qp)) &&
        ((qp->h_next == NULL) || (hp < qp->h_next))) {
      /* Insertion after qp */
      hp->h_next = qp->h_next;
      qp->h_next = hp;
      /* Verifies if the newly inserted block should be merged */
      if (LIMIT(hp) == hp->h_next) {
        /* Merge with the next block */
        hp->h_size += hp->h_next->h_size + sizeof(struct header);
        hp->h_next = hp->h_next->h_next;
      }
      if ((LIMIT(qp) == hp)) {  /* Cannot happen when qp == &heap.free */
        /* Merge with the previous block */
        qp->h_size += hp->h_size + sizeof(struct header);
        qp->h_next = hp->h_next;
      }

      isixp_exit_critical();
      return;
    }
    qp = qp->h_next;
  }
  isixp_exit_critical();
}

/*------------------------------------------------------*/
