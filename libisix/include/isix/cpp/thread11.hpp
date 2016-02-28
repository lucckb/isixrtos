/*
 * =====================================================================================
 *
 *       Filename:  thread11.hpp
 *
 *    Description:  C++11 bind thread implementation
 *
 *        Version:  1.0
 *        Created:  28.02.2016 10:23:02
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
#include "task_base.hpp"
#include <functional>

namespace isix {


/** \brief thread class is a template interface for thread */
template <typename FN, typename ... ARGS>
class thread : public task_base 
{
public:
	/** \brief thread constructor
	 *  \param[in] size Stack size 
	 *  \param[in] priority Thread priority
	 *  \param[in] flags Thread flags
	 *  \param[in] fn Function executed in separate thread
	 *  \param[in] args Arguments passed to the function
	 */
	thread( const size_t size, const osprio_t priority, 
			unsigned flags, FN&& function, ARGS&&... args )
		: m_bound_fn( std::bind(std::forward<FN>(function), std::forward<ARGS>(args)... ) )
		 {
			start_thread( size, priority, flags );
		 }
	/** /brief thread Destructor 
	 */
	virtual ~thread() {
	}
	
	thread( const thread& ) = delete;
	thread( thread&& ) = default;
	const thread& operator=(const thread&) = delete;
	thread& operator=(thread&&) = delete;


private:
	
	void main() override {
		m_bound_fn();
	}

	decltype( std::bind(std::declval<FN>(), std::declval<ARGS>()...)) m_bound_fn;
};

/** Helper factory function for thread creation  */
template <typename FN, typename ... ARGS>
	thread<FN,ARGS...> thread_create( const size_t size, const osprio_t priority, 
			unsigned flags, FN&& fn, ARGS&&... args )
	{
		return { size, priority, flags, std::forward<FN>(fn), std::forward<ARGS>(args)... };
	}
}

#endif
