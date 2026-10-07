# Coroutines reference

All names are in `namespace isix::co`. `ostick_t` timeouts are in ticks (`isix::ms2tick(ms)`);
`ISIX_TIME_INFINITE` is `0` and `ISIX_TIME_DONTWAIT` tests without waiting. "Scheduler thread" is
the RTOS thread that calls `step()`.

Common error codes: `ISIX_EOK`, `ISIX_ETIMEOUT`, `ISIX_EDESTROY` (primitive destroyed while
waiting), `ISIX_ECANCELED`, `ISIX_EINVARG` (invalid object or a primitive of another scheduler),
`ISIX_EBUSY`, `ISIX_ENOMEM`, `ISIX_ESTATE`, `ISIX_EPERM`.

## Core

### `template<class T> class task`

Lazy, move-only owner of a coroutine frame. Destroying it destroys the coroutine.

| Member | Contract |
|---|---|
| `bool valid()` | false for an empty task (frame allocation failed or moved from) |
| `bool done()` | finished or empty |
| `int error()` | `ISIX_EOK` finished, `ISIX_ECANCELED` finished after a cancel request, `ISIX_ESTATE` not finished |
| `T& value()` | value of a finished non-void task; stops the system when there is none |
| `result<T> result()` | moves the value out, or the error code |
| `void cancel()` | cooperative cancel (scheduler thread) |
| `co_await std::move(task)` | runs it as a child; result is the value of `T` |
| `handle()`, `release()`, `destroy()` | raw frame access |

### `template<class T> using result = std::expected<T, int>`

Value or `ISIX_E*` code. Use `has_value()`, `error()`, `value_or()`.

### `class scheduler`

| Member | Contract |
|---|---|
| `bool is_valid()` | false when the kernel event could not be created |
| `bool step()` | one round (see the guide); false when nothing is ready and nothing waits |
| `void run()` | `step()` until idle |
| `void run_forever()` | serve until `stop()`; sleeps while idle; cancels running coroutines on stop |
| `void stop()` | any thread; makes `run_forever()` return |
| `int post(Task&&, const char* name = nullptr)` | hand a task over from any thread; `ISIX_EINVARG` from an ISR, `ISIX_ENOMEM` for an empty task |
| `statistics stats()` | waiting, timers and posted counts always; resumes, frame and timing counters with `co_debug` |
| `for_each_task(f)` | `co_debug` only; calls `f(task_info{name, state})` for every live coroutine |
| `std::size_t detached_count()` | live coroutines started with `post` |

Constructed on any thread; one thread drives it (calls `step()`, `run()` or `run_forever()`) for its whole
life. Primitives and tasks must be destroyed before it.

### Free functions

| Function | Contract |
|---|---|
| `int spawn(scheduler&, task<T>&, const char* name = nullptr)` | start a task the caller keeps; `ISIX_ENOMEM` for an empty task |
| `auto run(scheduler&, task<T>&&)` | run to completion, return the value; stops the system for an empty task or a coroutine waiting on something outside `isix::co` |
| `auto run(task<T>&&)` | same with a temporary scheduler |
| `cancelled_awaiter cancelled()` | `co_await co::cancelled()` returns whether a cancel was requested; never suspends |

### `class scheduler_thread`

A scheduler on its own RTOS thread. `start(stack, priority)` returns `ISIX_EOK` or `ISIX_ENOMEM`;
`post(task)`; `stop()` cancels the running coroutines and ends the thread; `sched()` gives the
scheduler for creating primitives. The destructor stops and joins the thread.

### `class async_scope`

Scheduler thread only. `spawn(task&&, name = nullptr)` returns `ISIX_EOK` or `ISIX_ENOMEM`;
`cancel_all()`; `size()`; `co_await join()` returns `ISIX_EOK` or `ISIX_ECANCELED` (`ISIX_EBUSY` when
another coroutine already joins). The destructor destroys the coroutines still alive: destroy the
scope outside of the coroutines it runs.

## Primitives

### `class event`

Auto-reset event; a signal with no waiter stays latched; one signal wakes all current waiters.

| Member | Contract |
|---|---|
| `event(scheduler&)` | |
| `set(int status = ISIX_EOK)` | any thread; waiters get `status` |
| `set_isr(int status = ISIX_EOK)` | ISR; with several signals before the dispatch the last status wins |
| `co_await ev` / `ev.wait_for(t)` | `int` |

### `class semaphore`

`semaphore(scheduler&, int initial = 0, int limit = INT_MAX)`. `signal()` (thread), `signal_isr()`,
`try_take()` (any thread), `co_await sem.take([t])` returns `int`. Tokens go to waiters in FIFO order.

### `class mutex`

Not recursive, FIFO hand-off, for coroutines of one scheduler. `lock([t])` returns `int`;
`scoped([t])` returns a `scoped_lock` (`owns_lock()`, `error()`, `unlock()`), released on
destruction; `try_lock()`. `unlock()` returns `ISIX_ENOTLOCKED` when not locked and `ISIX_EPERM`
when called by a coroutine that does not own it. A mutex taken outside of a coroutine is
released outside of one.

### `template<class T, std::size_t N> class channel`

Bounded queue. `try_send(v)`, `try_recv()` from any thread, `try_send_isr(v)`, `try_recv_isr()` from an ISR
(trivially copyable `T`). `co_await send(v[, t])` returns `int`; `co_await recv([t])` returns
`result<T>`.

### `template<std::size_t N> class stream_buffer`

Byte ring with one producer (thread or ISR) and one reading coroutine.
`write(span)`, `write_isr(span)` return the bytes stored; excess bytes are dropped and counted in
`dropped()`; `available()`. `co_await read(span<byte|uint8_t> buf, min_bytes = 1, timeout)` returns
`result<std::size_t>`: it waits until `min_bytes` are available and then reads up to `buf.size()`.
A second concurrent reader gets `ISIX_EBUSY`.

### `sleep_ms(ms)`, `sleep_ticks(n)`, `sleep_until(deadline)`, `yield()`

`sleep_*` suspend; a delay of `0` or a deadline in the past returns at once. `yield()` lets the other
ready coroutines of the scheduler run first. They return `void`.

### `class fifo_reader<T>` (needs `fifo_event_notify`)

`fifo_reader(scheduler&, isix::fifo<T>&)`. `try_read()` returns `std::optional<T>`;
`co_await read([t])` returns `result<T>`. `is_valid()` is false when the fifo already has an event
connected. The reader should be the only consumer of the fifo.

### `class sem_adapter` (needs `sem_event_notify`)

`sem_adapter(scheduler&, ossem_t)`; `try_take()`, `co_await take([t])` returns `int`. A task blocked
on the semaphore has priority over the coroutine.

## Cancellation

### `class stop_source`

`stop_source(scheduler&)`; `bind(task&)` (scheduler thread; cancels at once when a stop was already
requested), `unbind()`, `reset()`, `request_stop()` (any thread), `request_stop_isr()`, `stop_requested()`.
The bound task must outlive the binding or be unbound first.

## Composition

| Function | Awaits to |
|---|---|
| `when_all(task&...)` | `void`; results through the tasks |
| `when_any(task&...)` | `std::size_t` index of the first finished task; the others are canceled and finish first |
| `with_timeout(task&, ticks)` / `with_timeout(task&&, ticks)` | `result<T>`; `ISIX_ETIMEOUT` when time ran out (the task is canceled and finished first), `ISIX_ECANCELED` when the caller was canceled |

Empty tasks are skipped. `when_all`/`when_any` start tasks that were not started.

## Frames

| Item | Contract |
|---|---|
| `frame_allocator{alloc, free, ctx}`, `set_frame_allocator`, `get_frame_allocator`, `reset_frame_allocator` | global frame source, set before any coroutine is created |
| `frame_storage<Size>` | one frame at a time; `in_use()`, `capacity()` |
| `frame_pool<BlockSize, Count>` | up to 32 blocks, thread safe; `used()`, `capacity()` |

Size parameters include the 8 byte frame header.

## Bridge to blocking code

### `class worker_thread`

`worker_thread(scheduler&)`, `start(stack, priority)`. `co_await worker.run(callable)` runs
`callable()` (returning `void` or `int`) on the worker thread and returns `ISIX_EOK` or its value;
`ISIX_EBUSY` while another job is claimed, `ISIX_ESTATE` when not started. A job cannot be
interrupted: after a cancel the wait ends with `ISIX_ECANCELED` once the job finished, and
destroying the awaiting coroutine blocks until the job is done. The callable must outlive the
`co_await`.

## Generators

`generator<T>`: a coroutine with `co_yield`, consumed by a range-for loop; it needs no scheduler.
`co_await` is not allowed inside it. `valid()` is false when the frame allocation failed (the range
is then empty).

## Diagnostics

| Item | Enabled by |
|---|---|
| `scheduler::stats()`, `for_each_task`, task names, thread ownership check, slow resume log (`CONFIG_ISIX_CO_SLOW_RESUME_US`, 100 ms) | `co_debug` |
| `set_trace_hook(void(*)(trace_event, const char* name, const void* frame))` | `co_trace` |
| `test::enable(bool)`, `test::set_seed(uint32_t)`, `test::get_seed()` | `co_shuffle` |

`trace_event`: `spawn`, `suspend`, `resume`, `finish`, `cancel`, `timeout`, `destroy`. The hook runs in
the context of the event and must not block. Task names are available for `spawn`, `finish`,
`cancel` and `destroy`; `suspend`, `resume` and `timeout` pass a null name.
