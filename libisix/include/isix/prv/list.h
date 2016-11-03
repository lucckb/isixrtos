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
static inline void list_init( list_entry_t *list )
{
   list->head.next = &list->head;
   list->head.prev = &list->head;
}

//Insert beetween
static inline void list_insert_between( list_t *a,list_t *b, list_t *elem )
{
   b->prev = elem;
   a->next = elem;
   elem->prev = a;
   elem->next = b;
}

//Insert element before
static inline void list_insert_before( list_t *before, list_t *elem )
{
    list_insert_between(before->prev,before,elem);
}


//Insert element after
static inline void list_insert_after( list_t *after,list_t *elem )
{
    list_insert_between(after,after->next,elem);
}

// Add element at end of list
static inline void list_insert_end( list_entry_t *list,list_t *elem )
{
    list_insert_after(list->head.prev,elem);
}


// Add element at end of list
static inline void list_insert_first( list_entry_t *list,list_t *elem )
{
    list_insert_after(list->head.next,elem);
}


// offsetof(TYPE,MEMBER) ?? <stddef.h>
#define list_entry(list,type,member) \
	(type*)((ptrdiff_t)(list)-offsetof(type,member))

 //List is empty check
#define list_isempty(entryp) \
	((entryp)->head.next==&(entryp)->head)

// list_next_entry - get the next element in list
#define list_next_entry(pos, member) \
	list_entry((pos)->member.next, typeof(*(pos)), member)


// list_prev_entry - get the prev element in list
#define list_prev_entry(pos, member) \
	list_entry((pos)->member.prev, typeof(*(pos)), member)


//Get first element of list
#define list_first_entry(entryp,field,type) \
    (type*)((ptrdiff_t)(entryp)->head.next - offsetof(type,field))


//Get first element of list
#define list_last_entry(entryp,field,type) \
    (type*)((ptrdiff_t)(entryp)->head.prev - offsetof(type,field))

//list for each element
#define list_for_each(entryp,iter) \
      for( (iter)=(entryp)->head.next; \
           (iter)!=&(entryp)->head;   \
           (iter)=(iter)->next \
         )

//list for each element safe for removal
#define list_for_each_safe(entryp,iter,tmp) \
      for( (iter)=(entryp)->head.next, (tmp)=(iter)->next; \
           (iter)!=&(entryp)->head;   \
           (iter)=(tmp),(tmp)=(iter)->next \
         )

//List for each element backward
#define list_for_each_reverse(entryp,iter) \
      for( (iter)=(entryp)->head.prev; \
           (iter)!=&(entryp)->head;   \
           (iter)=(iter)->prev \
         )

//list for each with given type
#define list_for_each_entry(entryp,iter,field) \
  for( (iter)=list_first_entry((entryp),field,typeof(*(iter))); \
       &(iter)->field!=&(entryp)->head;   \
	   (iter)=list_next_entry((iter),field) \
	 )

//list for each reverse order
#define list_for_each_entry_reverse(entryp,iter,field) \
  for( (iter)=list_last_entry((entryp),field,typeof(*(iter))); \
       &(iter)->field!=&(entryp)->head;   \
	   (iter)=list_prev_entry((iter),field) \
	 )


//list for each with given type safe for removal
#define list_for_each_entry_safe(entryp,iter,tmp,field) \
  for( (iter)=list_first_entry((entryp),field,typeof(*(iter))), \
	   (tmp)=list_next_entry((iter),field); \
       &(iter)->field!=&(entryp)->head; \
	   (iter)=(tmp),(tmp)=list_next_entry((tmp),field) \
	 )

//list for each reverse order safe for removal
#define list_for_each_entry_safe_reverse(entryp,iter,tmp,field) \
  for( (iter)=list_last_entry((entryp),field,typeof(*(iter))), \
	   (tmp)=list_prev_entry((iter),field); \
       &(iter)->field!=&(entryp)->head;   \
	   (iter)=(tmp),(tmp)=list_prev_entry((tmp),field) \
	 )

//Delete from list
static inline void list_delete( list_t *elem )
{
    elem->prev->next = elem->next;
    elem->next->prev = elem->prev;
    elem->next = NULL;
    elem->prev = NULL;
}

//Check if the element is free
static inline bool list_is_elem_assigned( list_t *elem )
{
	return elem->next!=NULL && elem->prev!=NULL;
}

