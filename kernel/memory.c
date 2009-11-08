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

//Align Mask
#define ALIGN_MASK 0x03
//Align Bytes
#define ALIGN_BYTES 4

/*------------------------------------------------------*/

typedef struct freelist
{
    size_t size;
    struct freelist *next;
    struct freelist *prev;
} freelist_t;

/*------------------------------------------------------*/

//List of free blocks
static freelist_t *free_list = NULL;

//Definition begin and end heap
extern uint8_t  __heap_start;
extern uint8_t __heap_end;


/*------------------------------------------------------*/
//Simple malloc using global heap
void* isix_alloc(size_t size)
{
     printk("kmalloc: req_size=%d\n",size);
    //Zero return
     if(!size) return NULL;
     //Ckeck chunk size
     if(size < (sizeof(freelist_t)-sizeof(size_t)))
     {
        size = sizeof(freelist_t)-sizeof(size_t);
     }
     //Alignement
     if(size & ALIGN_MASK)
     {
        size += ALIGN_BYTES - (size & ALIGN_MASK);
     }
     printk("kmalloc: heap_req=%d\n",size);
     //Jezeli nie zainicjalizowano list to sprobuj ja zainicjalizowac
     isixp_enter_critical();
     if(free_list==NULL)
     {
        free_list = (freelist_t*)&__heap_start;
        free_list->size = ((&__heap_end) - (&__heap_start)) - sizeof(size_t);
        free_list->next = NULL;
        free_list->prev = NULL;
        printk("kmalloc: Initialize Heap ADR_START=%08x size=%d\n",&free_list->next,free_list->size);
     }
     /* Zaznacz pierwszy wolny blok o wiekszym rozmiarze a
      * nastepnie kontynuj poszukiwania az do momentu
      * znalezienia elementu o takim samym rozmiarze lub
      * elementu mniejszego */
     freelist_t *grt = NULL;
     for(freelist_t *c=free_list; c ;c=c->next)
     {
        if( grt==NULL && c->size>size ) grt = c;
        //if( (c->size-size)<sizeof(freelist_t) ) continue;
        if( (c->size<size) || (c->size>(size+sizeof(freelist_t))) ) continue;
        //Znaleziono wolny blok o tym samym lub nie duzo wiekszym rozmiarze
        if(c->prev) c->prev->next = c->next; else free_list = c->next;
        if(c->next) c->next->prev = c->prev;
        printk("kmalloc: Allocate equal region ADR=%08x size=%d\n",&c->next,c->size);
        isixp_exit_critical();
        return (void*)&c->next;
     }
     //Brak pamieci wyjdz z bledem
     if(grt==NULL) return NULL;
     /* Teraz jestesmy pewni ze mamy jeden blok ktory jest
      * wiekszy niz rozmiar zadanego bloku wiec mozemy go
      * odpowiednio rozparcelowac i przydzielic         */
     freelist_t *new_mem = (freelist_t*)(((uint8_t*)grt)+size+sizeof(size_t));
     *new_mem = *grt;
     new_mem->size -= size + sizeof(size_t);
     grt->size = size;
     if(new_mem->prev) new_mem->prev->next = new_mem;
     else free_list = new_mem;
     printk("kmalloc: Return region ADR=%08x len=%d\n",&grt->next,grt->size);
     printk("kmalloc: Free region ADR=%08x len=%d\n",&new_mem->next,new_mem->size);
     isixp_exit_critical();
     return &grt->next;
}

/*------------------------------------------------------*/
//Memory free from dynamic range
void isix_free(void *mem)
{
    //Check memory management pool...
    if( ((uint8_t*)mem) < &__heap_start || ((uint8_t*)mem) > &__heap_end )
    {
        printk("kfree: ADR=%08x not in dynamic memory management\n",mem);
        return;
    }
    //Cast free list
    isixp_enter_critical();
    freelist_t *rlist = (freelist_t*) (((uint8_t*)mem)-sizeof(size_t));
    rlist->next = rlist->prev = NULL;
    printk("kfree: Region to free ADR=%08x size=%d\n",&rlist->next,rlist->size);
    if(free_list==NULL) { free_list = rlist; return; }
    for(freelist_t *c=free_list; c ; c=c->next)
    {
        if(c<rlist && c->next) continue;
        //Found valid region range
        if( ((uint8_t*)&rlist->next)+rlist->size == ((uint8_t*)c))
        {
            //Concate to one area
	        rlist->size += c->size + sizeof(size_t);
	        rlist->next = c->next;
	        rlist->prev = c->prev;
	        if(c->prev) c->prev->next = rlist;
	        else free_list = rlist;
	        printk("kfree: Concate1 region ADR=%08x len=%d\n",&c->next,c->size);
            isixp_exit_critical();
            return;
        }
        else
        {
            rlist->next = c;
	        rlist->prev = c->prev;
	        if(c->prev) c->prev->next=rlist;
	        else free_list = rlist;
	        c->prev = rlist;
	        printk("kfree: New FIRST free region ADR=%08x len=%d\n",&rlist->next,rlist->size);
            isixp_exit_critical();
            return;
        }
    }
}


/*------------------------------------------------------*/
#if ISIX_DEBUG_MEMORY == ISIX_DBG_ON
void printelem(void)
{
    int j = 0;
    freelist_t *z = NULL;
    for(freelist_t *i=free_list; i; i = i->next)
    {
        printk("Elem %d Adr %08x Size %d\n",j++,&i->next,i->size);
        z = i;
    }
    j = 0;
    for(freelist_t *i = z; i ; i=i->prev)
    {
	printk("Rev Elem %d Adr %08x Size %d\n",j++,&i->next,i->size);
    }
    printk("Reverse count = %d\n",j);
}

#endif /* DEBUG */

/*------------------------------------------------------*/


