/*
 * Coroutine scheduler for isix: runs many coroutines on one RTOS task.
 *
 * Wakeups are event driven (one isix event + one vtimer, no polling).
 * Scheduler structures belong to the scheduler thread; other threads and ISRs
 * only change primitive state and set an event bit (see co_event.hpp).
 *
 * Author: Lucjan Bryndza
 */
#pragma once

#if defined(CONFIG_ISIX_CPP_COROUTINES) && defined(__cplusplus)

#include <bit>
#include <coroutine>
#include <cstdint>
#include <exception>
#include <utility>

#include <isix/events.h>
#include <isix/error.h>
#include <isix/ostime.h>
#include <isix/scheduler.h>
#include <isix/softtimers.h>

namespace isix::co {

class scheduler;

namespace detail {

//! Intrusive doubly linked list link, unlinkable in O(1)
struct dlink final {
	dlink* prev {};
	dlink* next {};
	void* owner {};

	[[nodiscard]] bool linked() const noexcept { return next != nullptr; }

	void unlink() noexcept
	{
		if (next) {
			prev->next = next;
			next->prev = prev;
			next = nullptr;
			prev = nullptr;
		}
	}
};

//! Circular list with a sentinel head
class dlist final {
public:
	dlist() noexcept { m_head.prev = m_head.next = &m_head; }
	dlist(const dlist&) = delete;
	dlist& operator=(const dlist&) = delete;

	[[nodiscard]] bool empty() const noexcept { return m_head.next == &m_head; }

	[[nodiscard]] dlink* front() noexcept { return empty() ? nullptr : m_head.next; }

	[[nodiscard]] dlink* first() noexcept { return m_head.next; }

	[[nodiscard]] dlink* sentinel() noexcept { return &m_head; }

	void push_back(dlink& l) noexcept { insert_before(m_head, l); }

	static void insert_before(dlink& pos, dlink& l) noexcept
	{
		l.prev = pos.prev;
		l.next = &pos;
		pos.prev->next = &l;
		pos.prev = &l;
	}

private:
	dlink m_head {};
};

enum class node_state : unsigned char { idle, waiting, ready };

//! Suspension record living in the coroutine frame (awaiter or promise)
struct wait_node {
	dlink wait_link {};	//! List of the primitive waiters
	dlink timer_link {};	//! Scheduler timer list
	dlink sched_link {};	//! Scheduler waiting or ready list
	std::coroutine_handle<> h {};
	scheduler* sched {};
	ostick_t deadline {};
	int result { ISIX_EOK };
	int timeout_result { ISIX_ETIMEOUT };
	node_state state { node_state::idle };

	wait_node() noexcept
	{
		wait_link.owner = this;
		timer_link.owner = this;
		sched_link.owner = this;
	}
	wait_node(const wait_node&) = delete;
	wait_node& operator=(const wait_node&) = delete;
	~wait_node();
};

[[nodiscard]] inline wait_node* node_of(dlink* l) noexcept
{
	return static_cast<wait_node*>(l->owner);
}

struct promise_common {
	scheduler* sched {};
	bool completed { false };
	wait_node start_node {};
	wait_node* continuation {};	//! Parent resumed when this coroutine finishes

	inline void finish() noexcept;
};

//! Critical section shared between threads and ISRs
class critical_guard final {
public:
	critical_guard() noexcept { isix::enter_critical(); }
	critical_guard(const critical_guard&) = delete;
	critical_guard& operator=(const critical_guard&) = delete;
	~critical_guard() { isix::exit_critical(); }
};

} // namespace detail

class scheduler final {
public:
	using bit_handler = void (*)(void*);

	scheduler() noexcept : m_ev(isix_event_create())
	{
		if (m_ev) {
			m_used = c_timer_bit | c_fence_bit;
		}
	}

	scheduler(const scheduler&) = delete;
	scheduler& operator=(const scheduler&) = delete;

	//! Primitives and tasks must be destroyed before the scheduler
	~scheduler()
	{
		if (!m_ev) {
			return;
		}
		detach_all(m_waiting);
		detach_all(m_ready);
		if (m_vtimer) {
			// Fence: the worker runs commands in order, so once it fires
			// no command referencing this object is pending.
			isix_vtimer_start(m_vtimer, &scheduler::fence_cb, this, 1, false);
			isix_event_wait(m_ev, c_fence_bit, true, false, ISIX_TIME_INFINITE);
			isix_vtimer_destroy(m_vtimer);
		}
		isix_event_destroy(m_ev);
	}

	[[nodiscard]] bool is_valid() const noexcept { return m_ev != nullptr; }

	//! Event object the scheduler waits on, for connecting kernel notifiers
	[[nodiscard]] osevent_t event_handle() const noexcept { return m_ev; }

	//! Allocate an event bit and its dispatch handler. Returns 0 when exhausted.
	osbitset_t alloc_bit(bit_handler fn, void* ctx) noexcept
	{
		if (!m_ev) {
			return 0U;
		}
		for (unsigned n = c_first_user_bit; n < c_bits; ++n) {
			const osbitset_t bit = 1U << n;
			if ((m_used & bit) == 0U) {
				m_used |= bit;
				m_handlers[n] = { fn, ctx };
				return bit;
			}
		}
		return 0U;
	}

	void free_bit(osbitset_t bit) noexcept
	{
		if (bit != 0U) {
			m_used &= ~bit;
			m_handlers[std::countr_zero(bit)] = {};
		}
	}

	//! Signal a primitive bit (thread context)
	void signal(osbitset_t bit) noexcept
	{
		isix_event_set(m_ev, bit);
	}

	//! Signal a primitive bit (ISR context)
	void signal_isr(osbitset_t bit) noexcept
	{
		isix_event_set_isr(m_ev, bit);
	}

	//! Put a never run coroutine on the ready queue
	void enqueue(detail::wait_node& n) noexcept
	{
		if (n.state != detail::node_state::idle) {
			return;
		}
		n.sched = this;
		n.state = detail::node_state::ready;
		m_ready.push_back(n.sched_link);
	}

	//! Suspend a node, optionally with a timeout (ISIX_TIME_INFINITE = none)
	void suspend(detail::wait_node& n, std::coroutine_handle<> h, ostick_t timeout,
		int timeout_result = ISIX_ETIMEOUT) noexcept
	{
		n.h = h;
		n.sched = this;
		n.result = ISIX_EOK;
		n.timeout_result = timeout_result;
		n.state = detail::node_state::waiting;
		m_waiting.push_back(n.sched_link);
		if (timeout != ISIX_TIME_INFINITE) {
			n.deadline = isix::get_jiffies() + timeout;
			insert_timer(n);
		}
	}

	//! Move a waiting node to the ready queue with the given result
	void wake(detail::wait_node& n, int result) noexcept
	{
		if (n.state != detail::node_state::waiting) {
			return;
		}
		n.wait_link.unlink();
		n.timer_link.unlink();
		n.sched_link.unlink();
		n.result = result;
		n.state = detail::node_state::ready;
		m_ready.push_back(n.sched_link);
	}

	//! Remove a node from every scheduler list (frame destruction)
	void detach(detail::wait_node& n) noexcept
	{
		n.wait_link.unlink();
		n.timer_link.unlink();
		n.sched_link.unlink();
		n.state = detail::node_state::idle;
	}

	/**
	 * Run one scheduling step: wait for events (unless something is ready),
	 * dispatch signalled primitives, fire due timers and resume ready coroutines.
	 * Returns false only when there is nothing to run and nothing to wait for.
	 */
	bool step() noexcept
	{
		if (!m_ev) {
			return false;
		}
		process_due_timers();
		bool block = m_ready.empty() && !m_waiting.empty();
		if (block && !arm_timer()) {
			block = false;
		}
		const osbitset_t mask = m_used & ~c_fence_bit;
		const osbitset_ret_t r = isix_event_wait(m_ev, mask, true, false,
			block ? ISIX_TIME_INFINITE : ISIX_TIME_DONTWAIT);
		if (r < 0) {
			std::terminate();
		}
		osbitset_t bits = static_cast<osbitset_t>(r) & mask;
		bool progress = bits != 0U;
		if ((bits & c_timer_bit) != 0U) {
			m_vt_armed = false;
		}
		bits &= ~c_timer_bit;
		while (bits != 0U) {
			const auto n = static_cast<unsigned>(std::countr_zero(bits));
			bits &= bits - 1U;
			const auto& hd = m_handlers[n];
			if (hd.fn) {
				hd.fn(hd.ctx);
			}
		}
		process_due_timers();

		unsigned count = 0;
		for (auto* l = m_ready.first(); l != m_ready.sentinel(); l = l->next) {
			++count;
		}
		while (count-- > 0 && !m_ready.empty()) {
			auto* n = detail::node_of(m_ready.front());
			n->sched_link.unlink();
			n->state = detail::node_state::idle;
			const auto h = n->h;
			progress = true;
			h.resume();
		}
		return progress || !m_ready.empty() || !m_waiting.empty();
	}

	//! Drive the scheduler until it becomes idle
	void run() noexcept
	{
		while (step()) {
		}
	}

private:
	struct handler {
		bit_handler fn {};
		void* ctx {};
	};

	static constexpr unsigned c_bits = 30U;
	static constexpr osbitset_t c_timer_bit = 1U << 0;
	static constexpr osbitset_t c_fence_bit = 1U << 1;
	static constexpr unsigned c_first_user_bit = 2U;

	static bool time_before(ostick_t a, ostick_t b) noexcept
	{
		return static_cast<std::int32_t>(a - b) < 0;
	}

	static void detach_all(detail::dlist& list) noexcept
	{
		while (auto* l = list.front()) {
			auto* n = detail::node_of(l);
			n->wait_link.unlink();
			n->timer_link.unlink();
			n->sched_link.unlink();
			n->state = detail::node_state::idle;
			n->sched = nullptr;
		}
	}

	static void vtimer_cb(void* arg)
	{
		isix_event_set(static_cast<scheduler*>(arg)->m_ev, c_timer_bit);
	}

	static void fence_cb(void* arg)
	{
		isix_event_set(static_cast<scheduler*>(arg)->m_ev, c_fence_bit);
	}

	void insert_timer(detail::wait_node& n) noexcept
	{
		auto* pos = m_timers.first();
		while (pos != m_timers.sentinel() && !time_before(n.deadline, detail::node_of(pos)->deadline)) {
			pos = pos->next;
		}
		detail::dlist::insert_before(*pos, n.timer_link);
	}

	void process_due_timers() noexcept
	{
		const ostick_t now = isix::get_jiffies();
		while (auto* l = m_timers.front()) {
			auto* n = detail::node_of(l);
			if (time_before(now, n->deadline)) {
				break;
			}
			wake(*n, n->timeout_result);
			l->unlink();
		}
	}

	//! Arm the vtimer for the nearest deadline. Returns false if it is already due.
	bool arm_timer() noexcept
	{
		auto* l = m_timers.front();
		if (!l) {
			return true;
		}
		const ostick_t deadline = detail::node_of(l)->deadline;
		const ostick_t now = isix::get_jiffies();
		if (!time_before(now, deadline)) {
			return false;
		}
		if (m_vt_armed && m_vt_deadline == deadline) {
			return true;
		}
		if (!m_vtimer) {
			m_vtimer = isix_vtimer_create();
			if (!m_vtimer) {
				std::terminate();
			}
		}
		if (isix_vtimer_start(m_vtimer, &scheduler::vtimer_cb, this, deadline - now, false) != ISIX_EOK) {
			std::terminate();
		}
		m_vt_armed = true;
		m_vt_deadline = deadline;
		return true;
	}

	osevent_t m_ev {};
	osvtimer_t m_vtimer {};
	osbitset_t m_used {};
	ostick_t m_vt_deadline {};
	bool m_vt_armed { false };
	handler m_handlers[c_bits] {};
	detail::dlist m_ready {};
	detail::dlist m_waiting {};
	detail::dlist m_timers {};
};

namespace detail {

inline void promise_common::finish() noexcept
{
	completed = true;
	if (continuation && sched) {
		sched->enqueue(*continuation);
	}
}

inline wait_node::~wait_node()
{
	if (sched) {
		sched->detach(*this);
	} else {
		wait_link.unlink();
		timer_link.unlink();
		sched_link.unlink();
	}
}

} // namespace detail

} // namespace isix::co

#endif // CONFIG_ISIX_CPP_COROUTINES && __cplusplus
