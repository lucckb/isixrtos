#pragma once

/* L. Bryndza list version 1.0  */

#include <stddef.h>
#include <stdbool.h>


//List linkage structure
typedef struct list
{
    struct list *next;
    struct list *prev;
} list_t;


//List entry structure
typedef struct list_entry
{
   list_t head;
} list_entry_t;


//Initialize list
static inline void list_init(list_entry_t *list)
{
   list->head.next = &list->head;
   list->head.prev = &list->head;
}

//Insert beetween
static inline void list_insert_between(list_t *a,list_t *b,list_t *elem)
{
   b->prev = elem;
   a->next = elem;
   elem->prev = a;
   elem->next = b;
}

//Insert element before
static inline void list_insert_before(list_t *before,list_t *elem)
{
    list_insert_between(before->prev,before,elem);
}


//Insert element after
static inline void list_insert_after(list_t *after,list_t *elem)
{
    list_insert_between(after,after->next,elem);
}

// Add element at end of list
static inline void list_insert_end(list_entry_t *list,list_t *elem)
{
    list_insert_after(list->head.prev,elem);
}


// offsetof(TYPE,MEMBER) ?? <stddef.h>
#define list_getitem(list,type,member) (type*)((char*)(list)-offsetof(type,member))

//list for each element
#define list_for_each(list_entryp,iterator) \
      for( (iterator)=(list_entryp)->head.next; \
           (iterator)!=&(list_entryp)->head;   \
           (iterator) = (iterator)->next \
         )
//List for each element backward
#define list_for_each_reverse(list_entryp,iterator) \
      for( (iterator)=(list_entryp)->head.prev; \
           (iterator)!=&(list_entryp)->head;   \
           (iterator) = (iterator)->prev \
         )

//list for each with given type
#define list_for_each_entry(list_entryp,iterator,field) \
  for( list_t *_ilist_=(list_entryp)->head.next; \
       (iterator) = list_getitem(_ilist_,typeof(*(iterator)),field),_ilist_!=&(list_entryp)->head;   \
           _ilist_ = _ilist_->next \
         )

//list for each reverse order
#define list_for_each_entry_reverse(list_entryp,iterator,field) \
  for( list_t *_ilist_=(list_entryp)->head.prev; \
       (iterator) = list_getitem(_ilist_,typeof(*(iterator)),field),_ilist_!=&(list_entryp)->head;   \
           _ilist_ = _ilist_->prev \
         )


//Get first element of list
#define list_get_first(list_entryp,field,type) \
    (type*)((char*)(list_entryp)->head.next - offsetof(type,field))


//Get first element of list
#define list_get_last(list_entryp,field,type) \
    (type*)((char*)(list_entryp)->head.prev - offsetof(type,field))

//Delete from list
static inline void list_delete(list_t *elem)
{
    elem->prev->next = elem->next;
    elem->next->prev = elem->prev;
    elem->next = NULL;
    elem->prev = NULL;
}


 //List is empty check
#define list_isempty(list_entryp) ((list_entryp)->head.next==&(list_entryp)->head)

//Check if the element is free
static inline bool list_is_elem_assigned( list_t *elem )
{
	return elem->next!=NULL && elem->prev!=NULL;
}


