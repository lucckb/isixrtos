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
}	//end extern-C
#endif /* __cplusplus */
/*-----------------------------------------------------------------------*/
#ifdef __cplusplus
#include <cstddef>
/*-----------------------------------------------------------------------*/
namespace isix {
/*-----------------------------------------------------------------------*/
/** Mempool class for pool memory management
 *  WARNING! This class is indented only for
 *  usage with POD types, and it is helper class only.
 */
template <typename T> class mempool
{
public:
	/** Constructor
	 * @param[in] elems Number of elements
	 */
	explicit mempool( size_t elems )
		: m_mp( isix_mempool_create( elems, sizeof(T) ) )
	{
	}
	//! Destructor
	~mempool()
	{
		isix_mempool_destroy( m_mp );
	}
	/** Alloc mempool element
	 * @return allocated element
	 */
	T* alloc()
	{
		void* mem = isix_mempool_alloc(m_mp);
		T* obj = new(mem) T;
		return obj;
	}
#if __cplusplus > 199711L
	template <typename... ARGS>
		T* alloc(ARGS... args) noexcept
		{
			void* mem = isix_mempool_alloc(m_mp);
			T* obj = new(mem) T(args...);
			return obj;
		}
#endif
	void free( T* p )
	{
		p->~T();
		isix_mempool_free( m_mp, p );
	}
	/** Check if the fifo object is in valid state
	 * @return true if object is ok else return false
	 */
	bool is_valid() { return m_mp!=0; }
private:
	isix_mempool_t m_mp;
private:	//Noncopyable
	mempool(const mempool&);
	mempool& operator=(const mempool&);
};
/*-----------------------------------------------------------------------*/
}
/*-----------------------------------------------------------------------*/
#endif /* __cplusplus */
/*-----------------------------------------------------------------------*/
