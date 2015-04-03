/*-----------------------------------------------------------------------*/
/*
 * mempool.h
 *
 *  Created on: 16-11-2012
 *      Author: lucck
 */
/*-----------------------------------------------------------------------*/
#pragma once
/*-----------------------------------------------------------------------*/
#include <isix/config.h>
#include <isix/types.h>
/*-----------------------------------------------------------------------*/
#ifdef __cplusplus
extern "C" {
#endif /*__cplusplus*/

/*-----------------------------------------------------------------------*/
//ISIX mempool handle def
struct isix_mempool;
typedef struct isix_mempool* osmempool_t;

/*-----------------------------------------------------------------------*/
/** Create the mempool
 *  @param[in] elem Number of elements
 *  @param[in] elem_size Element size
 *  @return New mempool handle
 */
osmempool_t isix_mempool_create( size_t elems, size_t elem_size );

/*-----------------------------------------------------------------------*/
/** Destroy the mempool
 * @param[in] mp Mempool handler
 * @return None
 */
void isix_mempool_destroy( osmempool_t mp );

/*-----------------------------------------------------------------------*/
/** Allocate mempool object
 * @param[in] mp Mempool handler to alocate from
 * @return Pointer to allocated memory
 */
void* isix_mempool_alloc( osmempool_t mp );

/*-----------------------------------------------------------------------*/
/** Free pool pointer in mempool
 * @param[in] mp Mempool handler
 * @param[in] p memory from mempool
 */
void isix_mempool_free( osmempool_t mp, void *p );
/*-----------------------------------------------------------------------*/
#ifdef __cplusplus
}	//end extern-C
#endif /* __cplusplus */
