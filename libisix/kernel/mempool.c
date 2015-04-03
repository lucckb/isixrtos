/*-----------------------------------------------------------------------*/
/*
 * mempool.c
 *
 *  Created on: 16-11-2012
 *      Author: lucck
 */
/*-----------------------------------------------------------------------*/
#include <stdbool.h>
#include <isix/prv/list.h>
#include <isix/mempool.h>
#include <isix/prv/mempool.h>
#include <isix/prv/common.h>
#include <isix/memory.h>
#define _ISIX_KERNEL_CORE_
#include <isix/prv/scheduler.h>
/*-----------------------------------------------------------------------*/
//! Internal inode representation
struct mempool_node
{
    list_t inode;
};
/*-----------------------------------------------------------------------*/
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
void isix_mempool_destroy( osmempool_t mp )
{
    if( !mp )
      return;
    isix_free( mp->mem );
    isix_free( mp );
}
/*-----------------------------------------------------------------------*/
/* Allocate mempool object */
void* isix_mempool_alloc( osmempool_t mp )
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
void isix_mempool_free( osmempool_t mp, void *p )
{
    if( !mp ) return;
    if( !p ) return;
    _isixp_enter_critical();
    list_insert_end( &mp->free_elems, &((struct mempool_node*)p)->inode );
    _isixp_exit_critical();
}
/*-----------------------------------------------------------------------*/
