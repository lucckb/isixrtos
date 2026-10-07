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

#include <atomic>
#include <bit>
#include <coroutine>
#include <cstddef>
#include <cstdint>
#include <exception>
#include <expected>
#include <type_traits>
#include <utility>

#include <isix/arch/irq_global.h>
#include <isix/events.h>
#include <isix/error.h>
#include <isix/ostime.h>
#include <isix/scheduler.h>
#include <isix/softtimers.h>

#ifndef CONFIG_ISIX_CO_DEBUG
#define CONFIG_ISIX_CO_DEBUG 0
#endif
#ifndef CONFIG_ISIX_CO_TRACE
#define CONFIG_ISIX_CO_TRACE 0
#endif
#ifndef CONFIG_ISIX_CO_TEST_SHUFFLE
#define CONFIG_ISIX_CO_TEST_SHUFFLE 0
#endif
#ifndef CONFIG_ISIX_CO_SLOW_RESUME_US
#define CONFIG_ISIX_CO_SLOW_RESUME_US 100000U
#endif

#if CONFIG_ISIX_CO_DEBUG
#include <foundation/sys/dbglog.h>
#include <isix/assert.h>
#endif

namespace isix::co {

class scheduler;

namespace detail {
struct scheduler_access;
}

//! Value or an ISIX_E* error code
template<typename T>
using result = std::expected<T, int>;

//! Coroutine lifecycle events delivered to the trace hook
enum class trace_event : std::uint8_t { spawn, suspend, resume, finish, cancel, timeout, destroy };

//! Trace hook: event, task name (null when unknown) and the coroutine frame address
using trace_hook_fn = void (*)(trace_event, const char*, const void*);

namespace detail {

#if CONFIG_ISIX_CO_TRACE
inline trace_hook_fn g_trace_hook {};

inline void trace(trace_event e, const char* name, const void* frame) noexcept
{
	if (g_trace_hook) {
		g_trace_hook(e, name, frame);
	}
}
#else
inline void trace(trace_event, const char*, const void*) noexcept {}
#endif

} // namespace detail

#if CONFIG_ISIX_CO_TRACE
//! Install the lifecycle trace hook, null removes it. The hook must not block.
inline void set_trace_hook(trace_hook_fn hook) noexcept { detail::g_trace_hook = hook; }
#endif

#if CONFIG_ISIX_CO_TEST_SHUFFLE
//! Test mode: coroutines that are ready at the same time resume in a pseudo random order
namespace test {

namespace detail {
inline bool g_enabled { false };
inline std::uint32_t g_seed { 1U };
inline std::uint32_t g_state { 1U };
} // namespace detail

//! Turn the randomized order on or off
inline void enable(bool on) noexcept { detail::g_enabled = on; }

//! Restart the random sequence; the same seed gives the same order
inline void set_seed(std::uint32_t seed) noexcept
{
	detail::g_seed = seed != 0U ? seed : 1U;
	detail::g_state = detail::g_seed;
}

[[nodiscard]] inline std::uint32_t get_seed() noexcept { return detail::g_seed; }

//! Index in [0, n) of the next coroutine to resume
[[nodiscard]] inline unsigned pick(unsigned n) noexcept
{
	if (!detail::g_enabled || n < 2U) {
		return 0U;
	}
	auto x = detail::g_state;
	x ^= x << 13;
	x ^= x >> 17;
	x ^= x << 5;
	detail::g_state = x;
	return x % n;
}

} // namespace test
#endif

namespace detail {

//! Intrusive doubly linked list link, unlinkable in O(1)
struct dlink final {
	dlink* prev {};
	dlink* next {};

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

	//! Move all elements of another list to the end of this one
	void take(dlist& from) noexcept
	{
		if (from.empty()) {
			return;
		}
		auto* const first = from.m_head.next;
		auto* const last = from.m_head.prev;
		first->prev = m_head.prev;
		m_head.prev->next = first;
		last->next = &m_head;
		m_head.prev = last;
		from.m_head.prev = from.m_head.next = &from.m_head;
	}

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

enum class node_state : std::uint8_t { idle, waiting, ready };

//! Entry of the scheduler ready or waiting list: a coroutine to resume
struct ready_item {
	dlink sched_link {};
	std::coroutine_handle<> h {};
	node_state state { node_state::idle };

	ready_item() noexcept = default;
	ready_item(const ready_item&) = delete;
	ready_item& operator=(const ready_item&) = delete;
	~ready_item() { detach(); }

	void detach() noexcept
	{
		sched_link.unlink();
		state = node_state::idle;
	}
};

//! Suspension record living in the coroutine frame (awaiter)
struct wait_node {
	using abort_fn = void (*)(wait_node&, int);

	ready_item rdy {};
	dlink wait_link {};	//! List of the primitive waiters
	dlink timer_link {};	//! Scheduler timer list
	ostick_t deadline {};
	abort_fn on_abort {};	//! Replaces the wake-up on timeout or cancellation
	std::int16_t result { ISIX_EOK };
	std::int8_t timeout_result { ISIX_ETIMEOUT };

	wait_node() noexcept = default;
	wait_node(const wait_node&) = delete;
	wait_node& operator=(const wait_node&) = delete;
	~wait_node() { detach(); }

	//! Remove the node from every list it is on
	void detach() noexcept
	{
		wait_link.unlink();
		timer_link.unlink();
		rdy.detach();
	}
};

static_assert(std::is_standard_layout_v<wait_node>);
static_assert(std::is_standard_layout_v<ready_item>);
static_assert(offsetof(wait_node, rdy) == 0);
static_assert(sizeof(wait_node) <= 44);

[[nodiscard]] inline char* link_base(dlink* l, std::size_t offset) noexcept
{
	return reinterpret_cast<char*>(l) - offset;
}

//! Node that owns the given primitive waiter link
[[nodiscard]] inline wait_node* from_wait_link(dlink* l) noexcept
{
	return reinterpret_cast<wait_node*>(link_base(l, offsetof(wait_node, wait_link)));
}

//! Node that owns the given scheduler timer link
[[nodiscard]] inline wait_node* from_timer_link(dlink* l) noexcept
{
	return reinterpret_cast<wait_node*>(link_base(l, offsetof(wait_node, timer_link)));
}

//! Ready list entry that owns the given scheduler link
[[nodiscard]] inline ready_item* from_sched_link(dlink* l) noexcept
{
	return reinterpret_cast<ready_item*>(link_base(l, offsetof(ready_item, sched_link)));
}

//! Wait node embedding the given list entry (waiting list members only)
[[nodiscard]] inline wait_node* node_of(ready_item* it) noexcept
{
	return reinterpret_cast<wait_node*>(it);
}

//! Fail a suspension of a canceled coroutine before it is registered anywhere
template<typename P>
[[nodiscard]] inline bool abort_if_cancelled(std::coroutine_handle<P> h, wait_node& n) noexcept
{
	if (h.promise().cancel_requested) {
		n.result = ISIX_ECANCELED;
		return true;
	}
	return false;
}

//! Entry in the scheduler inbox, embedded in a primitive that can be signalled from any context
struct signal_link {
	dlink link {};
	void (*fn)(void*) {};
	void* ctx {};
	bool queued { false };

	signal_link(void (*f)(void*), void* c) noexcept : fn(f), ctx(c) {}
	signal_link(const signal_link&) = delete;
	signal_link& operator=(const signal_link&) = delete;
};

static_assert(std::is_standard_layout_v<signal_link>);

//! No state change for scheduler::signal()
struct no_update {
	void operator()() const noexcept {}
};

//! Link of a coroutine owned by an async_scope
struct scope_entry {
	dlink scope_link {};
};


//! Coroutine promise base; the ready_item is the start entry holding the coroutine handle
struct promise_common : scope_entry, ready_item {
#if CONFIG_ISIX_CO_DEBUG
	dlink dbg_link {};	//! Scheduler task registry
	const char* name {};
#endif
	bool completed { false };
	bool cancel_requested { false };
	bool detached { false };
	scheduler* sched {};
	wait_node* continuation {};	//! Parent resumed when this coroutine finishes
	promise_common* child {};	//! Child task awaited right now
	void (*on_finish)(promise_common&) noexcept {};	//! Replaces the continuation wake-up (join nodes)

	inline void finish() noexcept;

	[[nodiscard]] const char* task_name() const noexcept
	{
#if CONFIG_ISIX_CO_DEBUG
		return name;
#else
		return nullptr;
#endif
	}

#if CONFIG_ISIX_CO_DEBUG || CONFIG_ISIX_CO_TRACE
	~promise_common()
	{
#if CONFIG_ISIX_CO_DEBUG
		dbg_link.unlink();
#endif
		trace(trace_event::destroy, task_name(), this);
	}
#endif
};

//! Critical section shared between threads and ISRs
class critical_guard final {
public:
	critical_guard() noexcept { isix::enter_critical(); }
	critical_guard(const critical_guard&) = delete;
	critical_guard& operator=(const critical_guard&) = delete;
	~critical_guard() { isix::exit_critical(); }
};

//! Bookkeeping of coroutines started without an owning task
struct scope_core : wait_node {
	dlist tasks {};
	std::size_t count {};
	wait_node* joiner {};
	scheduler* sched {};

	inline void adopt(promise_common& p) noexcept;
	inline void cancel_all() noexcept;
	inline void destroy_all() noexcept;
	static inline void finish_cb(promise_common& p) noexcept;
};

//! Frame accounting shared by all schedulers (CONFIG_ISIX_CO_DEBUG only)
struct frame_counters {
	std::uint32_t live {};
	std::uint32_t bytes {};
	std::uint32_t peak_bytes {};
};

inline frame_counters g_frame_counters {};

} // namespace detail

//! State of a coroutine as reported by for_each_task()
enum class task_state : std::uint8_t { ready, waiting, awaiting_child, running, finished };

//! One entry of the task listing
struct task_info {
	const char* name;
	task_state state;
};

//! Scheduler statistics; counters other than waiting, timers and posted need CONFIG_ISIX_CO_DEBUG
struct scheduler_statistics {
	std::uint32_t resumes;
	std::uint32_t live_frames;	//! Frames of all schedulers
	std::uint32_t frame_bytes;
	std::uint32_t peak_frame_bytes;
	std::uint32_t max_resume_us;
	std::uint32_t waiting;
	std::uint32_t timers;
	std::uint32_t posted;
};

class scheduler final {
public:
	using bit_handler = void (*)(void*);
	using statistics = scheduler_statistics;

	scheduler() noexcept : m_ev(isix_event_create())
	{
		m_scope.sched = this;
		if (m_ev) {
			m_used = c_timer_bit | c_fence_bit | c_inbox_bit | c_post_bit;
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
		drain_posted(true);
		m_scope.destroy_all();
		destroy_dead();
#if CONFIG_ISIX_CO_DEBUG
		report_leaks();
#endif
		while (auto* l = m_waiting.front()) {
			detail::node_of(detail::from_sched_link(l))->detach();
		}
		while (auto* l = m_ready.front()) {
			detail::from_sched_link(l)->detach();
		}
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

	/**
	 * Queue a primitive for dispatch (thread context). The callable updates the
	 * primitive state in the same critical section that queues the link.
	 */
	template<typename F = detail::no_update>
	void signal(detail::signal_link& s, F&& update = {}) noexcept
	{
		// While coroutines are resumed the scheduler thread checks the inbox before
		// every blocking wait, so the event bit is not needed
		if (push_signal(s, update) && !m_resuming) {
			isix_event_set(m_ev, c_inbox_bit);
		}
	}

	//! Queue a primitive for dispatch (ISR context), see signal()
	template<typename F = detail::no_update>
	void signal_isr(detail::signal_link& s, F&& update = {}) noexcept
	{
		if (push_signal(s, update)) {
			isix_event_set_isr(m_ev, c_inbox_bit);
		}
	}

	//! Drop a queued signal (primitive destruction)
	void unsignal(detail::signal_link& s) noexcept
	{
		detail::critical_guard g;
		if (s.queued) {
			s.link.unlink();
			s.queued = false;
		}
	}

	//! Signal an adapter bit (thread context)
	void signal(osbitset_t bit) noexcept
	{
		isix_event_set(m_ev, bit);
	}

	//! Signal a primitive bit (ISR context)
	void signal_isr(osbitset_t bit) noexcept
	{
		isix_event_set_isr(m_ev, bit);
	}

	/**
	 * Hand a task over to the scheduler; it runs detached and is destroyed when it
	 * finishes. Safe from any thread, not from an ISR. Returns ISIX_ENOMEM for a task
	 * without a frame.
	 */
	template<typename Task>
	int post(Task&& t, [[maybe_unused]] const char* name = nullptr) noexcept
	{
		if (isix_irq_in_isr()) {
			return ISIX_EINVARG;
		}
		if (!m_ev || !t.valid()) {
			return ISIX_ENOMEM;
		}
		const auto h = t.release();
		detail::promise_common& p = h.promise();
		p.h = h;
#if CONFIG_ISIX_CO_DEBUG
		p.name = name;
#endif
		{
			detail::critical_guard g;
			m_posted.push_back(p.sched_link);
		}
		isix_event_set(m_ev, c_post_bit);
		return ISIX_EOK;
	}

	//! Number of coroutines that were posted or spawned in the internal scope and are still alive
	[[nodiscard]] std::size_t detached_count() const noexcept { return m_scope.count; }

	//! Start a coroutine that was never run on this scheduler (scheduler thread)
	void start(detail::promise_common& p, std::coroutine_handle<> h) noexcept
	{
#if CONFIG_ISIX_CO_DEBUG
		check_owner(false);
		if (!p.dbg_link.linked()) {
			m_tasks.push_back(p.dbg_link);
		}
#endif
		p.sched = this;
		p.h = h;
		detail::trace(trace_event::spawn, p.task_name(), &p);
		enqueue(p);
	}

	//! Put a never run coroutine on the ready queue
	void enqueue(detail::ready_item& n) noexcept
	{
		if (n.state != detail::node_state::idle) {
			return;
		}
		n.state = detail::node_state::ready;
		m_ready.push_back(n.sched_link);
	}

	//! Suspend a node, optionally with a timeout (ISIX_TIME_INFINITE = none)
	void suspend(detail::wait_node& n, std::coroutine_handle<> h, ostick_t timeout,
		int timeout_result = ISIX_ETIMEOUT) noexcept
	{
		n.rdy.h = h;
		detail::trace(trace_event::suspend, nullptr, h.address());
		n.result = ISIX_EOK;
		n.timeout_result = static_cast<std::int8_t>(timeout_result);
		n.rdy.state = detail::node_state::waiting;
		m_waiting.push_back(n.rdy.sched_link);
		if (timeout != ISIX_TIME_INFINITE) {
			n.deadline = isix::get_jiffies() + timeout;
			insert_timer(n);
		}
	}

	//! Move a waiting node to the ready queue with the given result
	void wake(detail::wait_node& n, int result) noexcept
	{
		if (n.rdy.state != detail::node_state::waiting) {
			return;
		}
		n.wait_link.unlink();
		n.timer_link.unlink();
		n.rdy.sched_link.unlink();
		n.result = static_cast<std::int16_t>(result);
		n.rdy.state = detail::node_state::ready;
		m_ready.push_back(n.rdy.sched_link);
	}

	/**
	 * Wake the coroutine suspended on a node with ISIX_ECANCELED (scheduler thread).
	 * Does nothing when it is not waiting, e.g. already on the ready queue.
	 */
	void cancel_wait(std::coroutine_handle<> h) noexcept
	{
		for (auto* l = m_waiting.first(); l != m_waiting.sentinel(); l = l->next) {
			auto& n = *detail::node_of(detail::from_sched_link(l));
			if (n.rdy.h == h) {
				if (n.on_abort) {
					n.on_abort(n, ISIX_ECANCELED);
				} else {
					wake(n, ISIX_ECANCELED);
				}
				return;
			}
		}
	}

	/**
	 * Run one scheduling step: wait for events (unless something is ready),
	 * dispatch signalled primitives, fire due timers and resume ready coroutines.
	 * Returns false only when there is nothing to run and nothing to wait for.
	 */
	bool step() noexcept
	{
		return step_impl(0U);
	}

	//! Snapshot of the counters
	[[nodiscard]] statistics stats() noexcept
	{
		statistics st {};
		for (auto* l = m_waiting.first(); l != m_waiting.sentinel(); l = l->next) {
			++st.waiting;
		}
		for (auto* l = m_timers.first(); l != m_timers.sentinel(); l = l->next) {
			++st.timers;
		}
		{
			detail::critical_guard g;
			for (auto* l = m_posted.first(); l != m_posted.sentinel(); l = l->next) {
				++st.posted;
			}
		}
#if CONFIG_ISIX_CO_DEBUG
		st.resumes = m_resumes;
		st.max_resume_us = m_max_resume_us;
		st.live_frames = detail::g_frame_counters.live;
		st.frame_bytes = detail::g_frame_counters.bytes;
		st.peak_frame_bytes = detail::g_frame_counters.peak_bytes;
#endif
		return st;
	}

#if CONFIG_ISIX_CO_DEBUG
	//! Call f(task_info) for every coroutine started on this scheduler and still alive
	template<typename F>
	void for_each_task(F&& f) noexcept
	{
		for (auto* l = m_tasks.first(); l != m_tasks.sentinel(); l = l->next) {
			const auto& p = *reinterpret_cast<const detail::promise_common*>(
				reinterpret_cast<const char*>(l) - dbg_link_offset());
			f(task_info{ p.name, state_of(p) });
		}
	}
#endif

	//! Coroutine being resumed right now, empty outside of a resume
	[[nodiscard]] std::coroutine_handle<> running() const noexcept { return m_running; }

	/**
	 * Serve the scheduler until stop() is called: sleeps while idle and wakes for
	 * posted tasks and events. Coroutines still running are canceled on stop.
	 */
	void run_forever() noexcept
	{
		while (!stop_requested()) {
			if (!step_impl(c_burst)) {
				wait_for_events();
			}
		}
		m_scope.cancel_all();
		while (m_scope.count != 0U && step()) {
		}
	}

	//! Ask run_forever() to return (any thread)
	void stop() noexcept
	{
		m_stop.store(true, std::memory_order_relaxed);
		isix_event_set(m_ev, c_post_bit);
	}

	[[nodiscard]] bool stop_requested() const noexcept
	{
		return m_stop.load(std::memory_order_relaxed);
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

	static constexpr unsigned c_bits = 12U;
	static constexpr osbitset_t c_timer_bit = 1U << 0;
	static constexpr osbitset_t c_fence_bit = 1U << 1;
	static constexpr osbitset_t c_inbox_bit = 1U << 2;
	static constexpr osbitset_t c_post_bit = 1U << 3;
	static constexpr unsigned c_first_user_bit = 4U;
	static constexpr unsigned c_burst = 4U;

	friend struct detail::scope_core;
	friend struct detail::scheduler_access;

#if CONFIG_ISIX_CO_DEBUG
	//! Byte offset of dbg_link, computed without offsetof on a non standard-layout type
	static std::size_t dbg_link_offset() noexcept
	{
		const detail::promise_common* const p = nullptr;
		return static_cast<std::size_t>(
			reinterpret_cast<const char*>(&p->dbg_link) - reinterpret_cast<const char*>(p));
	}

	void check_owner(bool take) noexcept
	{
		const auto self = isix_task_self();
		if (!m_owner_task) {
			if (take) {
				m_owner_task = self;
			}
		} else if (m_owner_task != self) {
			isix_bug("co::scheduler used from a foreign thread");
		}
	}

	void note_resume(osutick_t us) noexcept
	{
		++m_resumes;
		m_max_resume_us = us > m_max_resume_us ? static_cast<std::uint32_t>(us) : m_max_resume_us;
		if (us > CONFIG_ISIX_CO_SLOW_RESUME_US) {
			dbprintf("co: coroutine resume took %lu us", static_cast<unsigned long>(us));
		}
	}

	[[nodiscard]] task_state state_of(const detail::promise_common& p) noexcept
	{
		if (p.completed) {
			return task_state::finished;
		}
		if (p.state == detail::node_state::ready) {
			return task_state::ready;
		}
		for (auto* l = m_waiting.first(); l != m_waiting.sentinel(); l = l->next) {
			if (detail::node_of(detail::from_sched_link(l))->rdy.h == p.h) {
				return task_state::waiting;
			}
		}
		return p.child ? task_state::awaiting_child : task_state::running;
	}

	void report_leaks() noexcept
	{
		unsigned alive = 0;
		while (auto* l = m_tasks.front()) {
			l->unlink();
			++alive;
		}
		if (alive != 0U) {
			dbprintf("co: scheduler destroyed with %u coroutines alive", alive);
		}
	}
#endif

	//! One scheduling step; up to burst more rounds of ready coroutines run without polling the kernel
	bool step_impl(unsigned burst) noexcept
	{
		if (!m_ev) {
			return false;
		}
#if CONFIG_ISIX_CO_DEBUG
		check_owner(true);
#endif
		if (!m_driver) {
			m_driver = isix_task_self();
		}
		process_due_timers();
		bool block = m_ready.empty() && !m_waiting.empty() && m_inbox.empty();
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
		if ((bits & c_inbox_bit) != 0U || !m_inbox.empty()) {
			drain_inbox();
			progress = true;
		}
		if ((bits & c_post_bit) != 0U) {
			drain_posted(false);
		}
		bits &= ~(c_timer_bit | c_inbox_bit | c_post_bit);
		while (bits != 0U) {
			const auto n = static_cast<unsigned>(std::countr_zero(bits));
			bits &= bits - 1U;
			const auto& hd = m_handlers[n];
			if (hd.fn) {
				hd.fn(hd.ctx);
			}
		}
		process_due_timers();

		m_resuming = true;
		progress = resume_ready() || progress;
		while (burst-- > 0U && !m_ready.empty()) {
			process_due_timers();
			if (!m_inbox.empty()) {
				drain_inbox();
			}
			resume_ready();
		}
		m_resuming = false;
		// Inbox reads below must not move above the flag clear
		std::atomic_signal_fence(std::memory_order_seq_cst);
		destroy_dead();
		return progress || !m_ready.empty() || !m_waiting.empty() || !m_inbox.empty();
	}

	//! Resume the coroutines that are ready now; returns true when any was resumed
	bool resume_ready() noexcept
	{
		bool progress = false;
		unsigned count = 0;
		for (auto* l = m_ready.first(); l != m_ready.sentinel(); l = l->next) {
			++count;
		}
		while (count-- > 0 && !m_ready.empty()) {
			auto* head = m_ready.front();
#if CONFIG_ISIX_CO_TEST_SHUFFLE
			for (auto skip = test::pick(count + 1U); skip > 0U && head->next != m_ready.sentinel(); --skip) {
				head = head->next;
			}
#endif
			auto* n = detail::from_sched_link(head);
			n->sched_link.unlink();
			n->state = detail::node_state::idle;
			const auto h = n->h;
			progress = true;
			m_running = h;
			detail::trace(trace_event::resume, nullptr, h.address());
#if CONFIG_ISIX_CO_DEBUG
			const auto t0 = isix::get_ujiffies();
			h.resume();
			note_resume(isix::get_ujiffies() - t0);
#else
			h.resume();
#endif
			m_running = {};
		}
		return progress;
	}

	//! Sleep until any event bit is set; the bits are consumed by the next step()
	void wait_for_events() noexcept
	{
		isix_event_wait(m_ev, m_used & ~c_fence_bit, false, false, ISIX_TIME_INFINITE);
	}

	//! Adopt tasks handed over with post(); with discard they are destroyed instead
	void drain_posted(bool discard) noexcept
	{
		detail::dlist local;
		{
			detail::critical_guard g;
			local.take(m_posted);
		}
		while (auto* l = local.front()) {
			l->unlink();
			auto& p = static_cast<detail::promise_common&>(*detail::from_sched_link(l));
			if (discard) {
				p.h.destroy();
			} else {
				m_scope.adopt(p);
			}
		}
	}

	//! Destroy coroutines that finished detached; never done inside their own resume
	void destroy_dead() noexcept
	{
		while (auto* l = m_dead.front()) {
			l->unlink();
			detail::from_sched_link(l)->h.destroy();
		}
	}

	//! Returns true when the link was not queued yet and the inbox bit must be set
	template<typename F>
	bool push_signal(detail::signal_link& s, F& update) noexcept
	{
		detail::critical_guard g;
		update();
		if (s.queued) {
			return false;
		}
		s.queued = true;
		m_inbox.push_back(s.link);
		return true;
	}

	//! Take the first link off the private list and release it; call inside a critical section
	static detail::signal_link* pop_released(detail::dlist& list) noexcept
	{
		auto* const l = list.front();
		if (!l) {
			return nullptr;
		}
		l->unlink();
		auto* const sl = static_cast<detail::signal_link*>(static_cast<void*>(l));
		sl->queued = false;
		return sl;
	}

	//! Dispatch all queued primitives; a link is released before its handler runs
	void drain_inbox() noexcept
	{
		detail::dlist local;
		detail::signal_link* sl {};
		{
			detail::critical_guard g;
			local.take(m_inbox);
			sl = pop_released(local);
		}
		while (sl) {
			sl->fn(sl->ctx);
			if (local.empty()) {
				break;
			}
			detail::critical_guard g;
			sl = pop_released(local);
		}
	}

	static bool time_before(ostick_t a, ostick_t b) noexcept
	{
		return static_cast<std::int32_t>(a - b) < 0;
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
		while (pos != m_timers.sentinel() && !time_before(n.deadline, detail::from_timer_link(pos)->deadline)) {
			pos = pos->next;
		}
		detail::dlist::insert_before(*pos, n.timer_link);
	}

	void process_due_timers() noexcept
	{
		if (m_timers.empty()) {
			return;
		}
		const ostick_t now = isix::get_jiffies();
		while (auto* l = m_timers.front()) {
			auto* n = detail::from_timer_link(l);
			if (time_before(now, n->deadline)) {
				break;
			}
			l->unlink();
			detail::trace(trace_event::timeout, nullptr, n->rdy.h.address());
			if (n->on_abort) {
				n->on_abort(*n, n->timeout_result);
			} else {
				wake(*n, n->timeout_result);
			}
		}
	}

	//! Arm the vtimer for the nearest deadline. Returns false if it is already due.
	bool arm_timer() noexcept
	{
		auto* l = m_timers.front();
		if (!l) {
			return true;
		}
		const ostick_t deadline = detail::from_timer_link(l)->deadline;
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
	detail::dlist m_inbox {};
	detail::dlist m_posted {};
	detail::dlist m_dead {};
	detail::scope_core m_scope {};
	std::coroutine_handle<> m_running {};
	ostask_t m_driver {};
	std::atomic<bool> m_stop { false };
	volatile bool m_resuming { false };
#if CONFIG_ISIX_CO_DEBUG
	detail::dlist m_tasks {};
	ostask_t m_owner_task {};
	std::uint32_t m_resumes {};
	std::uint32_t m_max_resume_us {};
#endif
};

namespace detail {

//! Internal view of the scheduler for the primitives
struct scheduler_access {
	/**
	 * True in the thread that drives the scheduler, never in an ISR. Primitives
	 * use it to update their state directly instead of going through the inbox.
	 */
	[[nodiscard]] static bool in_driver_thread(const scheduler& s) noexcept
	{
		return !isix_irq_in_isr() && s.m_driver != nullptr && s.m_driver == isix_task_self();
	}
};

//! Cancel a coroutine and the chain of children it awaits (scheduler thread)
inline void cancel_promise(promise_common& root) noexcept
{
	trace(trace_event::cancel, root.task_name(), &root);
	promise_common* p = &root;
	for (;;) {
		p->cancel_requested = true;
		if (!p->child) {
			break;
		}
		p = p->child;
	}
	if (p->sched && !p->completed) {
		p->sched->cancel_wait(p->h);
	}
}

inline void scope_core::adopt(promise_common& p) noexcept
{
	p.detached = true;
	p.continuation = this;
	p.on_finish = &scope_core::finish_cb;
	tasks.push_back(p.scope_link);
	++count;
	sched->start(p, p.h);
}

inline void scope_core::cancel_all() noexcept
{
	for (auto* l = tasks.first(); l != tasks.sentinel(); l = l->next) {
		cancel_promise(static_cast<promise_common&>(*reinterpret_cast<scope_entry*>(l)));
	}
}

inline void scope_core::destroy_all() noexcept
{
	while (auto* l = tasks.front()) {
		auto& p = static_cast<promise_common&>(*reinterpret_cast<scope_entry*>(l));
		l->unlink();
		--count;
		p.on_finish = nullptr;
		p.continuation = nullptr;
		p.h.destroy();
	}
	joiner = nullptr;
}

inline void scope_core::finish_cb(promise_common& p) noexcept
{
	auto& core = static_cast<scope_core&>(*p.continuation);
	p.scope_link.unlink();
	--core.count;
	core.sched->m_dead.push_back(p.sched_link);
	if (core.count == 0U && core.joiner) {
		auto* const j = core.joiner;
		core.joiner = nullptr;
		core.sched->wake(*j, ISIX_EOK);
	}
}

inline void promise_common::finish() noexcept
{
	completed = true;
	trace(trace_event::finish, task_name(), this);
	if (on_finish) {
		on_finish(*this);
	} else if (continuation && sched) {
		sched->enqueue(continuation->rdy);
	}
}

} // namespace detail

} // namespace isix::co

#endif // CONFIG_ISIX_CPP_COROUTINES && __cplusplus
