#include <isix/memory.h>
#include <isix/types.h>
#include <isix/printk.h>
#include <isix/task.h>

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
extern u8 __heap_start,__heap_end;


/*------------------------------------------------------*/
//Simple malloc using global heap
void* kmalloc(size_t size)
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
     sched_lock();
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
        if(c->prev) c->prev->next = c->next;
        if(c->next) c->next->prev = c->prev;
        printk("kmalloc: Allocate equal region ADR=%08x size=%d\n",&c->next,c->size);
        sched_unlock();
        return (void*)&c->next;
     }
     //Brak pamieci wyjdz z bledem
     if(grt==NULL) return NULL;
     /* Teraz jestesmy pewni ze mamy jeden blok ktory jest
      * wiekszy niz rozmiar zadanego bloku wiec mozemy go
      * odpowiednio rozparcelowac i przydzielic         */
     printk("kmalloc: Found region ADR=%08x len=%d\n",&grt->next,grt->size);
     grt->size -= size + sizeof(size_t);
     printk("kmalloc: New region size len=%d\n",grt->size);
     u8 *ptr = ((u8*)grt) + grt->size;
     ((freelist_t*)ptr)->size = size;
     printk("kmalloc: New region ADR=%08x\n",&((freelist_t*)ptr)->next);
     sched_unlock();
     return &((freelist_t*)ptr)->next;
}

/*------------------------------------------------------*/

void kfree(void *mem)
{
    //Check memory management pool...
    if( ((u8*)mem) < &__heap_start || ((u8*)mem) > &__heap_end || free_list==NULL )
    {
        printk("kfree: ADR=%08x not in dynamic memory management\n",mem);
        return;
    }
    //Cast free list
    sched_lock();
    freelist_t *rlist = (freelist_t*) (((u8*)mem)-sizeof(size_t));
    rlist->next = rlist->prev = NULL;
    printk("kfree: Region to free ADR=%08x size=%d\n",&rlist->next,rlist->size);
    for(freelist_t *c=free_list; c ; c=c->next)
    {
        if(c<rlist && c->next) continue;
        //Found valid region range
        if( ((u8*)&c->next)+c->size == ((u8*)&rlist->next))
        {
            //Concate to one area
            c->size += rlist->size + sizeof(size_t);
            printk("kfree: Concate1 region ADR=%08x len=%d\n",&c->next,c->size);
            sched_unlock();
            return;
        }
        else
        {
            if(c<rlist)
            {
                //Jezeli adres jest mniejszy to dodaj na koncu
                rlist->next = c->next;
                rlist->prev = c;
                c->next = rlist;
                printk("kfree: New FIRST free region ADR=%08x len=%d\n",&rlist->next,rlist->size);
                sched_unlock();
                return;
            }
            else
            {
                //Jezeli adres jest wiekszy to dodaj to wszystko w srodek
                rlist->next = c;
                rlist->prev = c->prev;
                if(c->prev) c->prev->next = rlist;
                c->prev = rlist;
                printk("kfree: New LAST free region ADR=%08x len=%d\n",&rlist->next,rlist->size);
                sched_unlock();
                return;
            }
        }
    }
}


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
    for(freelist_t *i = z; i ; i=i->prev) j++;
    printk("Reverse count = %d\n",j);
}
