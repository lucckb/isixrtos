/* Queue operations */
#pragma once
#include <isix/types.h>

/*--------------------------------------------------------------*/
#ifdef __cplusplus
extern "C" {
#endif
/*--------------------------------------------------------------*/
struct fifo_struct;
typedef struct fifo_struct fifo_t;

/*--------------------------------------------------------------*/
//! Extra flags for fifo create EX
enum isix_fifo_ex_flags 
{
	isix_fifo_f_noirq = 0x01
};
/*--------------------------------------------------------------*/
/** Create queue for n elements
 * @param[in] n_elem Number of available elements in the fifo
 * @param[in] elem_size Size of the element
 * @param[in] flags Extra flags
 * @return Fifo object
 */
fifo_t* isix_fifo_create_ex( int n_elem, int elem_size, unsigned flags );

/*--------------------------------------------------------------*/
/** Compatibility version of the fifo create without ex */
static inline fifo_t* isix_fifo_create( int n_elem, int elem_size )
{
	return isix_fifo_create_ex( n_elem, elem_size, 0 );
}
/*--------------------------------------------------------------*/
/** Push element in the queue
 * @param[in] fifo  Pointer to queue
 * @param[in] item item The element push in the queue
 * @param[in] timeout Timeout for selected queue
 * @return 0 if success else return error     */
int isix_fifo_write(fifo_t *fifo, const void *item, tick_t timeout);

/*--------------------------------------------------------------*/
/** Push element in the queue from a ISR
 * @param[in] queue  Pointer to queue
 * @param[in] item The element push in the queue
 * @return ISIX_EOK if success else return an error     */
int isix_fifo_write_isr(fifo_t *queue, const void *item);

/*--------------------------------------------------------------*/
/** Delete queue
 * @param[in] fifo Pointer to the fifo object*/
int isix_fifo_destroy(fifo_t *fifo);

/*--------------------------------------------------------------*/
/** Return number of the elements available in the fifo
 * @param[in] fifo Pointer to the fifo object
 * @return ISIX_EOK if success else return an error */
int isix_fifo_count(fifo_t *fifo);

/*--------------------------------------------------------------*/
/** Read element from the queue if available
 * @param[in] fifo Pointer to the fifo object
 * @param[out] item Pointer to the bufer
 * @param[in] timeout Max waiting timeout
 * @return ISIX_EOK if success else return an error */
int isix_fifo_read(fifo_t *fifo,void *item, tick_t timeout);

/*--------------------------------------------------------------*/
/** Read element from the queue if available. ISR version
 * @param[in] queue Pointer to the fifo object
 * @param[out] item Pointer to the bufer
 * @return ISIX_EOK if success else return an error */
int isix_fifo_read_isr(fifo_t *queue, void *item);

/*--------------------------------------------------------------*/
#ifdef __cplusplus
}	//end extern-C
#endif /* __cplusplus */
/*--------------------------------------------------------------*/


