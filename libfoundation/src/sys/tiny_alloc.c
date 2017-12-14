#include "foundation/sys/tiny_alloc.h"


#if CONFIG_ISIX_WITHOUT_KERNEL

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
  } h;
  size_t                h_size;
};

static struct
{
  struct header         free;   /* Guaranteed to be not adjacent to the heap */

} heap;



//!Lock the memory
static void mem_lock_init(void)
{
}

//!Lock the memory
static void mem_lock(void)
{
}


//!Unlock the memory
static void mem_unlock(void)
{
}


#if !CONFIG_FOUNDATION_NO_DYNAMIC_ALLOCATION
//! Initialize global heap
__attribute__((constructor))
static void tiny_alloc_init(void)
{
  struct header *hp;

  extern unsigned char __heap_start;
  extern unsigned char __heap_end;

  mem_lock_init();

  hp = (void *)&__heap_start;
  hp->h_size = &__heap_end - &__heap_start - sizeof(struct header);

  hp->h.h_next = NULL;
  heap.free.h.h_next = hp;
  heap.free.h_size = 0;

}
#endif


void *tiny_alloc(size_t size)
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


#define LIMIT(p) (struct header *)((char *)(p) + \
                                   sizeof(struct header) + \
                                   (p)->h_size)


void tiny_free(void *p)
{
  struct header *qp, *hp;

  hp = (struct header *)p - 1;

  qp = &heap.free;
  mem_lock();

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
      if ((LIMIT(qp) == hp)) {  /* Cannot happen when qp == &heap.tiny_free */
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
#else

//NOTE: Prevent empty translation unit
static inline void tiny_alloc_dummy_func() {}


#endif /*CONFIG_ISIX_WITHOUT_KERNEL!=0 */


