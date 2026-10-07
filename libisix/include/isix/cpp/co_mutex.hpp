/*
 * Coroutine-native mutex for coroutines of one scheduler. Ownership is
 * handed to waiters in FIFO order on unlock().
 *
 * Author: Lucjan Bryndza
 */
#pragma once

#if defined(CONFIG_ISIX_CPP_COROUTINES) && defined(__cplusplus)

#include <coroutine>
#include <utility>

#include <isix/cpp/coroutine_scheduler.hpp>
#include <isix/error.h>
#include <isix/ostime.h>

namespace isix::co {

class mutex final {
public:
	explicit mutex(scheduler& sched) noexcept : m_sched(&sched) {}

	mutex(const mutex&) = delete;
	mutex& operator=(const mutex&) = delete;

	//! Waiters are woken with ISIX_EDESTROY
	~mutex()
	{
		while (auto* l = m_waiters.front()) {
			m_sched->wake(*detail::from_wait_link(l), ISIX_EDESTROY);
		}
	}

	[[nodiscard]] bool is_locked() const noexcept { return m_locked; }

	//! Take the mutex without waiting
	[[nodiscard]] bool try_lock() noexcept
	{
		if (m_locked) {
			return false;
		}
		m_locked = true;
		m_owner = m_sched->running();
		return true;
	}

	/**
	 * Release the mutex or pass it to the first waiter. Only the owning coroutine
	 * may unlock; a mutex taken outside of a coroutine is released the same way.
	 */
	int unlock() noexcept
	{
		if (!m_locked) {
			return ISIX_ENOTLOCKED;
		}
		if (m_owner != m_sched->running()) {
			return ISIX_EPERM;
		}
		release();
		return ISIX_EOK;
	}

	class lock_awaiter {
	public:
		lock_awaiter(mutex* mtx, ostick_t timeout) noexcept : m_mtx(mtx) { m_node.deadline = timeout; }

		[[nodiscard]] bool await_ready() noexcept
		{
			if (!m_mtx) {
				m_node.result = ISIX_EINVARG;
				return true;
			}
			if (m_mtx->m_waiters.empty() && m_mtx->try_lock()) {
				return true;
			}
			if (m_node.deadline == ISIX_TIME_DONTWAIT) {
				m_node.result = ISIX_ETIMEOUT;
				return true;
			}
			return false;
		}

		template<typename P>
		bool await_suspend(std::coroutine_handle<P> h) noexcept
		{
			if (detail::abort_if_cancelled(h, m_node)) {
				return false;
			}
			auto* const sched = h.promise().sched;
			if (sched != m_mtx->m_sched) {
				m_node.result = ISIX_EINVARG;
				return false;
			}
			m_mtx->m_waiters.push_back(m_node.wait_link);
			sched->suspend(m_node, h, m_node.deadline);
			return true;
		}

		int await_resume() const noexcept { return m_node.result; }

	protected:
		[[nodiscard]] int result() const noexcept { return m_node.result; }

	private:
		mutex* m_mtx {};
		detail::wait_node m_node {};
	};

	//! Lock owner that unlocks on destruction
	class scoped_lock final {
	public:
		scoped_lock() noexcept = default;
		scoped_lock(mutex* mtx, int rc) noexcept : m_mtx(rc == ISIX_EOK ? mtx : nullptr), m_rc(rc) {}
		scoped_lock(scoped_lock&& o) noexcept
			: m_mtx(std::exchange(o.m_mtx, nullptr)), m_rc(o.m_rc) {}
		scoped_lock& operator=(scoped_lock&& o) noexcept
		{
			if (this != &o) {
				unlock();
				m_mtx = std::exchange(o.m_mtx, nullptr);
				m_rc = o.m_rc;
			}
			return *this;
		}
		scoped_lock(const scoped_lock&) = delete;
		scoped_lock& operator=(const scoped_lock&) = delete;
		~scoped_lock() { unlock(); }

		[[nodiscard]] bool owns_lock() const noexcept { return m_mtx != nullptr; }
		//! Result of the lock operation
		[[nodiscard]] int error() const noexcept { return m_rc; }

		void unlock() noexcept
		{
			if (auto* const m = std::exchange(m_mtx, nullptr)) {
				m->release();
			}
		}

	private:
		mutex* m_mtx {};
		int m_rc { ISIX_ESTATE };
	};

	class scoped_awaiter final : public lock_awaiter {
	public:
		scoped_awaiter(mutex* mtx, ostick_t timeout) noexcept : lock_awaiter(mtx, timeout), m_mtx(mtx) {}
		scoped_lock await_resume() const noexcept { return scoped_lock{ m_mtx, result() }; }
	private:
		mutex* m_mtx {};
	};

	//! Lock without a timeout
	lock_awaiter lock() noexcept { return lock_awaiter{ this, ISIX_TIME_INFINITE }; }

	//! Lock with a timeout (ISIX_TIME_DONTWAIT = try only)
	lock_awaiter lock(ostick_t timeout) noexcept { return lock_awaiter{ this, timeout }; }

	//! Lock and return an RAII owner
	scoped_awaiter scoped(ostick_t timeout = ISIX_TIME_INFINITE) noexcept
	{
		return scoped_awaiter{ this, timeout };
	}

private:
	//! Pass the mutex to the first waiter or unlock it
	void release() noexcept
	{
		if (auto* l = m_waiters.front()) {
			auto& n = *detail::from_wait_link(l);
			m_owner = n.rdy.h;
			m_sched->wake(n, ISIX_EOK);
		} else {
			m_locked = false;
			m_owner = {};
		}
	}

	scheduler* m_sched {};
	std::coroutine_handle<> m_owner {};
	bool m_locked { false };
	detail::dlist m_waiters {};
};

} // namespace isix::co

#endif
