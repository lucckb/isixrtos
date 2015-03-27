/*
 * =====================================================================================
 *
 *       Filename:  scoped_lock.h
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  21.12.2013 22:26:10
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Lucjan Bryndza (LB), lucck(at)boff(dot)pl
 *   Organization:  BoFF
 *
 * =====================================================================================
 */


#pragma once

#ifdef __cplusplus

namespace isix {
	
	template <typename T> class scoped_lock {
	public:
		scoped_lock( T& lock ) 
			: m_lock( lock )
		{
			m_lock.lock();
		}
		~scoped_lock( ) {
			m_lock.unlock();
		}
	private:
		T& m_lock;
	};
}

#endif
