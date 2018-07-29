/* Queue operations */
#pragma once
#include <isix/types.h>


#ifdef __cplusplus
extern "C" {
#endif

struct isix_fifo;
typedef struct isix_fifo* osfifo_t;


//! Extra flags for fifo create EX
enum isix_fifo_ex_flags
{
	isix_fifo_f_noirq = 0x01
};

/** Create queue for n elements
 * @param[in] n_elem Number of available elements in the fifo
 * @param[in] elem_size Size of the element
 * @param[in] flags Extra flags
 * @return Fifo object
 */
osfifo_t isix_fifo_create_ex( int n_elem, int elem_size, unsigned flags );


/** Compatibility version of the fifo create without ex */
static inline osfifo_t isix_fifo_create( int n_elem, int elem_size )
{
	return isix_fifo_create_ex( n_elem, elem_size, 0 );
}

/** Push element in the queue
 * @param[in] fifo  Pointer to queue
 * @param[in] item item The element push in the queue
 * @param[in] timeout Timeout for selected queue
 * @return 0 if success else return error     */
int isix_fifo_write(osfifo_t fifo, const void *item, ostick_t timeout);


/** Push element in the queue from a ISR
 * @param[in] queue  Pointer to queue
 * @param[in] item The element push in the queue
 * @return ISIX_EOK if success else return an error     */
int isix_fifo_write_isr(osfifo_t queue, const void *item);


/** Delete queue
 * @param[in] fifo Pointer to the fifo object*/
int isix_fifo_destroy(osfifo_t fifo);


/** Return number of the elements available in the fifo
 * @param[in] fifo Pointer to the fifo object
 * @return ISIX_EOK if success else return an error */
int isix_fifo_count(osfifo_t fifo);


/** Read element from the queue if available
 * @param[in] fifo Pointer to the fifo object
 * @param[out] item Pointer to the bufer
 * @param[in] timeout Max waiting timeout
 * @return ISIX_EOK if success else return an error */
int isix_fifo_read(osfifo_t fifo,void *item, ostick_t timeout);


/** Read element from the queue if available. ISR version
 * @param[in] queue Pointer to the fifo object
 * @param[out] item Pointer to the bufer
 * @return ISIX_EOK if success else return an error */
int isix_fifo_read_isr(osfifo_t queue, void *item);


#ifdef __cplusplus
}	//end extern-C
#endif /* __cplusplus */


#ifdef __cplusplus
namespace isix {
namespace {
	using fifo_t = osfifo_t;
	inline osfifo_t fifo_create_ex( int n_elem, int elem_size, unsigned flags ) {
		return ::isix_fifo_create_ex( n_elem, elem_size, flags );
	}
	inline osfifo_t fifo_create( int n_elem, int elem_size ) {
		return ::isix_fifo_create( n_elem, elem_size );
	}
	inline int fifo_write( osfifo_t fifo, const void *item, ostick_t timeout=ISIX_TIME_INFINITE) {
		return ::isix_fifo_write( fifo, item, timeout );
	}
	inline int fifo_write_isr( osfifo_t queue, const void *item ) {
		return ::isix_fifo_write_isr( queue, item );
	}
	inline int fifo_destroy( osfifo_t fifo ) {
		return ::isix_fifo_destroy( fifo );
	}
	inline int fifo_count( osfifo_t fifo ) {
		return ::isix_fifo_count( fifo );
	}
	inline int fifo_read( osfifo_t fifo,void *item, ostick_t timeout=ISIX_TIME_INFINITE) {
		return ::isix_fifo_read( fifo, item, timeout );
	}
	inline int fifo_read_isr( osfifo_t queue, void *item ) {
		return ::isix_fifo_read_isr( queue, item );
	}
}}
#endif /* __cplusplus */
