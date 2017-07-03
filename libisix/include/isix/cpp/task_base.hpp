/*
 * =====================================================================================
 *
 *       Filename:  task_base.hpp
 *
 *    Description:  Task base C++ wrapper for isix 
 *
 *        Version:  1.0
 *        Created:  02.04.2015 23:09:01
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
#include <cstddef>
#include <isix/task.h>

namespace isix {

	//! C++ wrapper for the task/thread
	class task_base
	{
	public:
		/** Default constructor (empty) */
		[[deprecated]]
		task_base() {

		}
		/** Construct the task
		* @param[in] stack_depth Stack depth of the thread/task
		* @param[in] priority Thread/task priority
		*/
		void start_thread(std::size_t stack_depth, osprio_t priority, unsigned flags=0) noexcept
		{
			task_id = isix_task_create( start_task, this, stack_depth, priority, flags );
		}
		virtual ~task_base()
		{
			if( task_id ) {
				isix_task_kill(task_id);
				task_id = nullptr;
			}
		}
		/** Get thread task id
		* @return Task control object
		*/
		ostask_t get_taskid() const noexcept { return task_id; }
		ostask_t tid() const noexcept { return task_id; }
		/** Check the fifo object is in valid state
		* @return True when the object is in valid state
		*/
		bool is_valid() noexcept { return task_id!=0; }

		/** Kill the task */
		void kill() noexcept {
			if( task_id ) {
				isix_task_kill(task_id);
				task_id = nullptr;
			}
		}
		task_base(const task_base&) = delete;
		task_base(task_base&&) = default;
		const task_base& operator=(const task_base&) = delete;
		task_base& operator=(task_base&&) = delete;

	protected:
		/** Pure virtual method for the object main thread */
		virtual void main() noexcept = 0;
	private:
		static void start_task(void *ptr) noexcept
		{
			static_cast<task_base*>(ptr)->main();
			static_cast<task_base*>(ptr)->task_id = nullptr;
		}
	private:
		ostask_t task_id { nullptr };
	};
}
#endif /* __cplusplus */
