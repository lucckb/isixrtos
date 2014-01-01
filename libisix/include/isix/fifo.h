/* Queue operations */
#ifndef _ISIX_FIFO_H
#define _ISIX_FIFO_H

/*--------------------------------------------------------------*/
#ifdef __cplusplus
extern "C" {
namespace isix {
#endif /*__cplusplus*/

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
}	//end namespace
}	//end extern-C
#endif /* __cplusplus */
/*--------------------------------------------------------------*/
//****************************** C++ API ***********************************
#ifdef __cplusplus
#include <cstddef>

namespace isix {
/*--------------------------------------------------------------*/
#ifdef ISIX_CONFIG_USE_MULTIOBJECTS
union ihandle;
#endif

/*--------------------------------------------------------------*/
//! The base class for fifo contains only data
class fifo_base
{
#ifdef ISIX_CONFIG_USE_MULTIOBJECTS
	friend union ihandle;
#endif
public:
	explicit fifo_base(fifo_t *hwnd_) : hwnd(hwnd_) {}
protected:
	fifo_t *hwnd;
private:
	fifo_base(const fifo_base&);
	fifo_base& operator=(const fifo_base&);
};

/*--------------------------------------------------------------*/
//! The C++ class wrapper for the queue
template <typename T> class fifo : public fifo_base
{
public:
	/** Construct fifo object with the selected elements
	 * @param n_elem Number of elements in the fifo
	 */
	explicit fifo(std::size_t n_elem, unsigned flags=0)
		: fifo_base(isix_fifo_create_ex(n_elem,sizeof(T),flags))
	{
	}
	//! Destruct fifo object
	~fifo()
	{
		isix_fifo_destroy(hwnd);
	}
	/** Check if the fifo object is in valid state
	 * @return true if object is ok else return false
	 */
	bool is_valid() { return hwnd!=0; }
	/* Push data in the queue
	 * @param[in] c Reference to the object
	 * @param[in] timeout Wait time when queue is not empty
	 * @return ISIX_EOK if success else return an error
	 */
	int push(const T &c,tick_t timeout=0)
	{
		return isix_fifo_write( hwnd, &c, timeout );
	}
	/* Push data in the queue from the ISR
	 * @param[in] c Reference to the object
	 * @param[in] timeout Wait time when queue is not empty
	 * @return ISIX_EOK if success else return an error
	 */
	int push_isr(const T &c)
	{
		return isix_fifo_write_isr( hwnd, &c );
	}
	/** Get available elements in the fifo
	 * @return available elements in the fifo
	 */
	int size() const
	{
		return isix_fifo_count( hwnd );
	}
	/** Pop the element from the queue
	 * @param[in] c Reference to the buffer
	 * @param[in] timeout Max waiting time
	 * @return ISIX_EOK if success else return an error
	 */
	int pop(T &c, tick_t timeout=0)
	{
		return isix_fifo_read( hwnd, &c, timeout );
	}
	/** Pop the element from the queue. Called from the ISR
	 * @param[in] c Reference to the buffer
	 * @param[in] timeout Max waiting time
	 * @return ISIX_EOK if success else return an error
	 */
	int pop_isr(T &c)
	{
		return isix_fifo_read_isr( hwnd, &c );
	}
private:
	fifo(const fifo&);
	fifo& operator=(const fifo&);
};

/*--------------------------------------------------------------*/
}
#endif /* __cplusplus */
/*--------------------------------------------------------------*/

#endif /* __ISIX_FIFO_H */
/*--------------------------------------------------------------*/
