/*-----------------------------------------------------------------------*/
/*
 * mempool.h
 *
 *  Created on: 16-11-2012
 *      Author: lucck
 */
/*-----------------------------------------------------------------------*/
#ifndef ISIX_MEMPOOL_H_
#define ISIX_MEMPOOL_H_
/*-----------------------------------------------------------------------*/
#include <isix/config.h>
#include <isix/types.h>
/*-----------------------------------------------------------------------*/
#ifdef __cplusplus
extern "C" {
namespace isix {
#endif /*__cplusplus*/

/*-----------------------------------------------------------------------*/
//ISIX mempool handle def
struct isix_mempool;
typedef struct isix_mempool* isix_mempool_t;

/*-----------------------------------------------------------------------*/
/** Create the mempool
 *  @param[in] elem Number of elements
 *  @param[in] elem_size Element size
 *  @return New mempool handle
 */
isix_mempool_t isix_mempool_create( size_t elems, size_t elem_size );

/*-----------------------------------------------------------------------*/
/** Destroy the mempool
 * @param[in] mp Mempool handler
 * @return None
 */
void isix_mempool_destroy( isix_mempool_t mp );

/*-----------------------------------------------------------------------*/
/** Allocate mempool object
 * @param[in] mp Mempool handler to alocate from
 * @return Pointer to allocated memory
 */
void* isix_mempool_alloc( isix_mempool_t mp );

/*-----------------------------------------------------------------------*/
/** Free pool pointer in mempool
 * @param[in] mp Mempool handler
 * @param[in] p memory from mempool
 */
void isix_mempool_free( isix_mempool_t mp, void *p );

/*-----------------------------------------------------------------------*/
#ifdef __cplusplus
}	//end namespace
}	//end extern-C
#endif /* __cplusplus */
/*-----------------------------------------------------------------------*/
#endif /* MEMPOOL_H_ */
