/*
 * mempool.h
 *
 *  Created on: 16-11-2012
 *      Author: lucck
 */
/*-----------------------------------------------------------------------*/
#ifndef ISIX_PRV_MEMPOOL_H_
#define ISIX_PRV_MEMPOOL_H_
/*-----------------------------------------------------------------------*/
/** Private mempool isix structure */
struct isix_mempool
{
    list_entry_t free_elems;
    void *mem;
};

/*-----------------------------------------------------------------------*/
#endif /* MEMPOOL_H_ */
