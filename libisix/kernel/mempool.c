
/*
 * mempool.c
 *
 *  Created on: 16-11-2012
 *      Author: lucck
 */

#include <stdbool.h>
#include <stddef.h>
#include <isix/prv/list.h>
#include <isix/mempool.h>
#include <isix/prv/mempool.h>
#include <isix/prv/common.h>
#include <isix/memory.h>
#include <isix/prv/scheduler.h>

//! Internal inode representation
struct mempool_node
{
    list_t inode;
};

//! Range check internal function
static inline bool addr_in_mempool( osmempool_t mp, const void* p )
{
	return (uintptr_t)p >= (uintptr_t)mp->mem &&
		   (uintptr_t)p <  (uintptr_t)mp->mem+mp->mem_size ;
}

/* Create the mempool */
osmempool_t isix_mempool_create( size_t elems, size_t elem_size )
{
    elem_size = _isixp_align_size( elem_size );
    if( elem_size < sizeof(struct mempool_node))
    {
		return NULL;
    }
    osmempool_t mempool = isix_alloc( sizeof(struct isix_mempool) );
    if( !mempool )
		return NULL;
    mempool->mem = isix_alloc( elems * elem_size );
	mempool->mem_size = elems * elem_size;
    if( !mempool->mem )
    {
      isix_free( mempool );
      return NULL;
    }
    list_init( &mempool->free_elems );
    for( size_t e=0; e<elems; e++ )
    {
		list_insert_end( &mempool->free_elems,
			&((struct mempool_node*)((char*)mempool->mem + elem_size*e ))->inode
		);
    }
    return mempool;
}

/* Destroy the mempool */
void isix_mempool_destroy( osmempool_t mp )
{
    if( !mp )
      return;
    isix_free( mp->mem );
    isix_free( mp );
}

/* Allocate mempool object */
void* isix_mempool_alloc( osmempool_t mp )
{
    if( !mp ) return NULL;
    isix_enter_critical();
    if( list_isempty( &mp->free_elems ) )
    {
		isix_exit_critical();
		return NULL;
    }
    struct mempool_node *n = list_first_entry( &mp->free_elems, inode, struct mempool_node );
    list_delete( &n->inode );
    isix_exit_critical();
    return n;
}

/* Free inode from mempool */
int isix_mempool_free( osmempool_t mp, void* p )
{
    if( !mp ) return ISIX_EINVARG;
    if( !p ) return ISIX_EINVARG;
	if( !addr_in_mempool(mp,p) ) return ISIX_EINVADDR;
    isix_enter_critical();
    list_insert_end( &mp->free_elems, &((struct mempool_node*)p)->inode );
    isix_exit_critical();
	return ISIX_EOK;
}

