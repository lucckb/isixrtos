/*
 * =====================================================================================
 *
 *       Filename:  fifo_event.c
 *
 *    Description:  Fifo event API
 *
 *        Version:  1.0
 *        Created:  19.04.2015 19:07:39
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Lucjan Bryndza (LB), lucck(at)boff(dot)pl
 *   Organization:  BoFF
 *
 * =====================================================================================
 */
#include <isix/fifo_event.h>
#include <isix/prv/fifo_lock.h>

#ifdef CONFIG_ISIX_LOGLEVEL_FIFO
#undef CONFIG_ISIX_LOGLEVEL
#define CONFIG_ISIX_LOGLEVEL CONFIG_ISIX_LOGLEVEL_FIFO
#endif
#include <isix/prv/printk.h>

#if CONFIG_ISIX_FIFO_EVENT_NOTIFY

/** Connect event API to the FIFO queue for notify selected
 * if the fifo is readeable or fifo is writable
 * @param[in] fifo handle to the fifo which is interested in read/write
 * @param[in] evt  handle to the osevent notifier
 * @param[in] inbit Bit number for notify read is possible
 */
int isix_fifo_event_connect( osfifo_t fifo, osevent_t evt, int inbit )
{
	if( !fifo || !evt ) {
		return ISIX_EINVARG;
	}
	if( inbit < 0 || inbit > 31 ) {
		return ISIX_EINVARG;
	}
	uintptr_t excepted = 0;
	if( !atomic_compare_exchange_strong((atomic_uintptr_t*)&fifo->evt,
				&excepted, (uintptr_t)evt) ) {
		//Already assigned
		return ISIX_EBUSY;
	}
	atomic_store( &fifo->bitno, inbit );
	return ISIX_EOK;
}

/** Disconnect selected event from the FIFO
 * @param[in] fifo handle to the fifo which is interested in read/write
 * @param[in] evt  handle to the osevent notifier
 */
int isix_fifo_event_disconnect( osfifo_t fifo, osevent_t evt )
{
	if( !fifo || !evt ) {
		return ISIX_EINVARG;
	}
	uintptr_t excepted = (uintptr_t)evt;
	if( !atomic_compare_exchange_strong((atomic_uintptr_t*)&fifo->evt,
				&excepted, (uintptr_t)NULL) ) {
		//Already free
		return ISIX_EBUSY;
	}
	atomic_store( &fifo->bitno, ISIX_FIFO_EVENT_INVALID_BITS );
	return ISIX_EOK;
}

/** Private function called when event API notification should be made
 * @param[in] fifo Input fifo handle
 */
void _isixp_fifo_rxavail_event_raise( osfifo_t fifo, bool isr )
{
	osevent_t ev = (osevent_t)atomic_load( (atomic_uintptr_t*)&fifo->evt );
	uint8_t bitno = atomic_load( &fifo->bitno );
	if( ev && bitno <= 31 )
	{
		if(!isr ) {
			isix_event_set( ev, 1U<<bitno );
		} else {
			isix_event_set_isr( ev, 1U<<bitno );
		}
	}
}

#endif /* CONFIG_ISIX_FIFO_EVENT_NOTIFY */

