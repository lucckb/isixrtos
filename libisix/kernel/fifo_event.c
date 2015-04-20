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
#define  _ISIX_KERNEL_CORE_
#include <isix/fifo_event.h>
#include <isix/prv/fifo_lock.h>
#include <isix/printk.h>

#ifdef ISIX_CONFIG_FIFO_EVENT_NOTIFY

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
	if( sys_cmpxchg((uintptr_t*)&fifo->evt,(uintptr_t)NULL, (uintptr_t)evt) )
	{	//Already assigned
		return ISIX_EBUSY;
	}
	sys_atomic_write_uint8_t( &fifo->bitno, inbit );
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
	if( sys_cmpxchg((uintptr_t*)&fifo->evt,(uintptr_t)evt,(uintptr_t)NULL)
			!= (uintptr_t)evt )
	{	//Already free
		return ISIX_EBUSY;
	}
	sys_atomic_write_uint8_t( &fifo->bitno, ISIX_FIFO_EVENT_INVALID_BITS );
	return ISIX_EOK;
}

/** Private function called when event API notification should be made
 * @param[in] fifo Input fifo handle
 */
void _isixp_fifo_rxavail_event_raise( osfifo_t fifo, bool isr )
{
	osevent_t ev = (osevent_t)port_atomic_read_uintptr_t( (uintptr_t*)&fifo->evt );
	uint8_t bitno = sys_atomic_read_uint8_t( &fifo->bitno );
	if( ev && bitno <= 31 )
	{
		if(!isr ) {
			isix_event_set( ev, 1U<<bitno );
		} else {
			isix_event_set_isr( ev, 1U<<bitno );
		}
	}
}

#endif /* ISIX_CONFIG_FIFO_EVENT_NOTIFY */

