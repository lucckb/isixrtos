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
#include "thread_base.hpp"
#include <functional>

namespace isix {

	/** \brief thread class is a template interface for thread */
	class thread final : public detail::thread_base
	{
		public:
			/** @brief thread constructor
			 *  @param[in] fn Function executed in separate thread
			 *  @param[in] args Arguments passed to the function
			 */
			template <typename FN, typename ... ARGS>
				thread( FN&& function, ARGS&&... args ) noexcept
				: m_bound_fn( std::bind(std::forward<FN>(function), std::forward<ARGS>(args)... ) )
				{
				}
			/** /brief thread Destructor
			*/
			virtual ~thread()
			{
				stop_thread();
			}
			/** @brief thread constructor which also starts the thread
			 *  @param[in] size Stack size
			 *  @param[in] priority Thread priority
			 *  @param[in] flags Thread flags
			 *  @param[in] function Function executed in separate thread
			 *  @param[in] args Arguments passed to the function
			 */
			template <typename FN, typename ... ARGS>
				thread( detail::thread_start_tag, const size_t size, const osprio_t priority,
						unsigned flags, FN&& function, ARGS&&... args ) noexcept
				: m_bound_fn( std::bind(std::forward<FN>(function), std::forward<ARGS>(args)... ) )
				{
					start_thread( size, priority, flags );
				}
			thread(thread&) = delete;
			thread(thread&&) = default;
			thread& operator=(thread&) = delete;
			thread& operator=(thread&&) = delete;
		private:
			void runner() override
			{
				m_bound_fn();
			}
			std::function<void()> m_bound_fn;
	};

	/** Helper factory function for thread creation  */
	template <typename FN, typename ... ARGS>
		thread thread_create( FN&& fn, ARGS&&... args ) noexcept
		{
			return { std::forward<FN>(fn), std::forward<ARGS>(args)... };
		}

	/** \brief Create thread and run
	 *  \param[in] size Stack size
	 *  \param[in] priority Thread priority
	 *  \param[in] flags Thread flags
	 *  \param[in] fn Function executed in separate thread
	 *  \param[in] args Arguments passed to the function
	 */
	template <typename FN, typename ... ARGS>
		thread thread_create_and_run( const size_t size, const osprio_t priority,
				unsigned flags, FN&& fn, ARGS&&... args ) noexcept
		{
			return thread( detail::thread_start_tag{}, size, priority, flags,
					std::forward<FN>(fn), std::forward<ARGS>(args)... );
		}

}
#endif
