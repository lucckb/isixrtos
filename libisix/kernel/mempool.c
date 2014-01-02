/*-----------------------------------------------------------------------*/
/*
 * mempool.c
 *
 *  Created on: 16-11-2012
 *      Author: lucck
 */
/*-----------------------------------------------------------------------*/
#include <stdbool.h>
#include <prv/scheduler.h>
#include <prv/list.h>
#include <isix/mempool.h>
#include <prv/mempool.h>
#include <isix/memory.h>
/*-----------------------------------------------------------------------*/
/* Private function for return len alignment */
static inline size_t isix_align_mem_len( size_t len )
{
    return (len/ISIX_CONFIG_BYTE_ALIGNMENT_SIZE+!!(len%ISIX_CONFIG_BYTE_ALIGNMENT_SIZE))*ISIX_CONFIG_BYTE_ALIGNMENT_SIZE;
}

/*-----------------------------------------------------------------------*/
//! Internal inode representation
struct mempool_node
{
    list_t inode;
};
/*-----------------------------------------------------------------------*/
/* Create the mempool */
isix_mempool_t isix_mempool_create( size_t elems, size_t elem_size )
{
    elem_size = isix_align_mem_len( elem_size );
    if( elem_size < sizeof(struct mempool_node))
    {
    	return NULL;
    }
    isix_mempool_t mempool = isix_alloc( sizeof(struct isix_mempool) );
    if( !mempool )
    	return NULL;
    mempool->mem = isix_alloc( elems *  elem_size );
    if( !mempool->mem )
    {
      isix_free( mempool );
      return NULL;
    }
    list_init( &mempool->free_elems );
    for( size_t e=0; e<elems; e++ )
    {
      list_insert_end( &mempool->free_elems, &((struct mempool_node*)((char*)mempool->mem + elem_size*e ))->inode );
    }
    return mempool;
}
/*-----------------------------------------------------------------------*/
/* Destroy the mempool */
void isix_mempool_destroy( isix_mempool_t mp )
{
    if( !mp )
      return;
    isix_free( mp->mem );
    isix_free( mp );
}
/*-----------------------------------------------------------------------*/
/* Allocate mempool object */
void* isix_mempool_alloc( isix_mempool_t mp )
{
    if( !mp ) return NULL;
    _isixp_enter_critical();
    if( list_isempty( &mp->free_elems ) )
    {
    	_isixp_exit_critical();
    	return NULL;
    }
    struct mempool_node *n = list_get_first( &mp->free_elems, inode, struct mempool_node );
    list_delete( &n->inode );
    _isixp_exit_critical();
    return n;
}
/*-----------------------------------------------------------------------*/
/* Free inode from mempool */
void isix_mempool_free( isix_mempool_t mp, void *p )
{
    if( !mp ) return;
    if( !p ) return;
    _isixp_enter_critical();
    list_insert_end( &mp->free_elems, &((struct mempool_node*)p)->inode );
    _isixp_exit_critical();
}
/*-----------------------------------------------------------------------*/
