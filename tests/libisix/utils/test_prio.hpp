/*
 * Copyright (c) 2025 Lucjan Bryndza
 *
 * Helpers for tests that lower the priority of the test thread
 */
#pragma once

#include <isix.h>

namespace test_utils {

//! Busy loop for the given number of ms without yielding
inline void cpu_busy(unsigned ms_duration)
{
	const auto t1 = isix::get_jiffies();
	do {
		asm volatile("nop\n");
	} while (!isix::timer_elapsed(t1, ms_duration));
}

//! Priority of the test thread saved by lower_test_prio
inline osprio_t& saved_test_prio()
{
	static osprio_t prio;
	return prio;
}

//! True when the test priority is currently lowered
inline bool& test_prio_lowered()
{
	static bool lowered;
	return lowered;
}

//! Lower the priority of the calling test thread; restore in TEST_TEAR_DOWN
inline void lower_test_prio(osprio_t prio)
{
	if (!test_prio_lowered()) {
		saved_test_prio() = isix_get_task_priority(nullptr);
		test_prio_lowered() = true;
	}
	isix_task_change_prio(nullptr, prio);
}

//! Restore the priority saved by lower_test_prio (safe to call always)
inline void restore_test_prio()
{
	if (test_prio_lowered()) {
		isix_task_change_prio(nullptr, saved_test_prio());
		test_prio_lowered() = false;
	}
}

//! Set of helper tasks created with an extra reference, released in TEST_TEAR_DOWN
class task_pool {
	static constexpr auto max_tasks = 8U;
public:
	//! Create a helper task, nullptr on failure or when the pool is full
	ostask_t spawn(task_func_ptr_t fn, void* arg, osprio_t prio, unsigned long stack = 2048)
	{
		if (m_count >= max_tasks) {
			return nullptr;
		}
		const auto t = isix_task_create(fn, arg, stack, prio, isix_task_flag_ref);
		if (t) {
			m_tasks[m_count++] = t;
		}
		return t;
	}
	//! Kill and unreference all the tasks (safe for already finished ones)
	void release()
	{
		for (auto i = 0U; i < m_count; ++i) {
			isix_task_kill(m_tasks[i]);
			isix_task_unref(m_tasks[i]);
			m_tasks[i] = nullptr;
		}
		m_count = 0;
	}
private:
	ostask_t m_tasks[max_tasks] {};
	unsigned m_count {};
};

}
