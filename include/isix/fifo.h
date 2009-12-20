/* Queue operations */
#ifndef __ISIX_FIFO_H
#define __ISIX_FIFO_H

/*--------------------------------------------------------------*/
#ifdef __cplusplus
extern "C" {
namespace isix {
#endif /*__cplusplus*/

/*--------------------------------------------------------------*/
struct fifo_struct;
typedef struct fifo_struct fifo_t;

/*--------------------------------------------------------------*/
/* Create queue for n elements
 * if succes return queue pointer else return null   */
fifo_t* isix_fifo_create(int n_elem, int elem_size);


/* Send element to queue
 * queue - Pointer to queue
 * item -  Element to push in queue
 * timeout - Timeout for selected queue
 * return 0 if succes else return error     */
int isix_fifo_write(fifo_t *fifo, const void *item, tick_t timeout);

int isix_fifo_write_isr(fifo_t *queue, const void *item);

/*--------------------------------------------------------------*/
/* Delete created queue */
int isix_fifo_destroy(fifo_t *fifo);

/*--------------------------------------------------------------*/
/* How many elements is in fifo */
int isix_fifo_count(fifo_t *fifo);

/*--------------------------------------------------------------*/
/* Get element from queue *
 * queue - Pointer to queue
 * item - Pointer to item
 * timeout - Max waiting timeouts */
int isix_fifo_read(fifo_t *fifo,void *item, tick_t timeout);

int isix_fifo_read_isr(fifo_t *queue, void *item);

/*--------------------------------------------------------------*/
#ifdef __cplusplus
}	//end namespace
}	//end extern-C
#endif /* __cplusplus */
/*--------------------------------------------------------------*/
//****************************** C++ API ***********************************
#ifdef __cplusplus
#include <cstddef>

namespace isix {
/*--------------------------------------------------------------*/
template <typename T> class fifo
{
public:
	//Constructor
	explicit fifo(std::size_t n_elem)
	{
		hwnd = isix_fifo_create(n_elem,sizeof(T));
	}
	//Destructor
	~fifo()
	{
		isix_fifo_destroy(hwnd);
	}
	//Check the fifo object is in valid state
	bool is_valid() { return hwnd!=0; }
	//Push data to the queue
	int push(const T &c,tick_t timeout=0)
	{
		return isix_fifo_write( hwnd, &c, timeout );
	}
	//Push data from the ISR
	int push_isr(const T &c)
	{
		return isix_fifo_write_isr( hwnd, &c );
	}
	//Get fifo free count
	int free() const
	{
		return isix_fifo_count( hwnd );
	}
	//Pop element
	int pop(T &c, tick_t timeout=0)
	{
		return isix_fifo_read( hwnd, &c, timeout );
	}
	//Pop element from ISR
	int pop_isr(T &c)
	{
		return isix_fifo_read_isr( hwnd, &c );
	}
private:
	fifo_t *hwnd;
};

/*--------------------------------------------------------------*/
}
#endif /* __cplusplus */
/*--------------------------------------------------------------*/

#endif /* __ISIX_FIFO_H */
/*--------------------------------------------------------------*/
