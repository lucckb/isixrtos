/*
 * =====================================================================================
 *
 *       Filename:  mempool.hpp
 *
 *    Description:  Memory pool isix cpp wrapper
 *
 *        Version:  1.0
 *        Created:  02.04.2015 23:02:35
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Lucjan Bryndza (LB), lucck(at)boff(dot)pl
 *   Organization:  BoFF
 *
 * =====================================================================================
 */

#pragma once

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
	int free( T* p )
	{
		p->~T();
		return isix_mempool_free( m_mp, p );
	}
	/** Check if the fifo object is in valid state
	 * @return true if object is ok else return false
	 */
	bool is_valid() { return m_mp!=0; }
private:
	osmempool_t m_mp;
private:	//Noncopyable
	mempool(const mempool&);
	mempool& operator=(const mempool&);
};
/*-----------------------------------------------------------------------*/
}
/*-----------------------------------------------------------------------*/
#endif /* __cplusplus */
/*-----------------------------------------------------------------------*/
