/*
 * =====================================================================================
 *
 *       Filename:  fifo_event.h
 *
 *    Description:  Fifo event notifier API
 *
 *        Version:  1.0
 *        Created:  19.04.2015 18:47:29
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Lucjan Bryndza (LB), lucck(at)boff(dot)pl
 *   Organization:  BoFF
 *
 * =====================================================================================
 */

#pragma once
#ifdef ISIX_CONFIG_FIFO_EVENT_NOTIFY
#include <isix/config.h>
#include "fifo.h"
#include "events.h"


/** Connect event API to the FIFO queue for notify selected
 * if the fifo is readeable or fifo is writable
 * @param[in] fifo handle to the fifo which is interested in read/write
 * @param[in] evt  handle to the osevent notifier
 * @param[in] inbit Bit number for notify read is possible
 */
int isix_fifo_event_connect( osfifo_t fifo, osevent_t evt, int inbit );


/** Disconnect selected event from the FIFO 
 * @param[in] fifo handle to the fifo which is interested in read/write
 * @param[in] evt  handle to the osevent notifier
 */
int isix_fifo_event_disconnect( osfifo_t fifo, osevent_t evt );


#endif /* ISIX_CONFIG_FIFO_EVENT_NOTIFY */
