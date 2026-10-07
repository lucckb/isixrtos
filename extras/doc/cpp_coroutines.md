# C++20 coroutines (`isix::co`)

Optional header-only layer for stackless coroutines. Many coroutines run on **one RTOS task**
under a `co::scheduler`. Wake-ups are event driven (one isix event plus one vtimer per
scheduler, no polling), so an idle scheduler sleeps in `isix_event_wait` and works with tickless idle.

## Requirements

- C++20 coroutines in the toolchain (the project builds with `gnu++23`).
- Define **`CONFIG_ISIX_CPP_COROUTINES`** before including [`isix.h`](../../libisix/include/isix.h)
  or pass `-DCONFIG_ISIX_CPP_COROUTINES=1` for the target. The unit tests define it only in
  [`tests/libisix/11_coroutines.cpp`](../../tests/libisix/11_coroutines.cpp).
- Optional kernel options (off by default, both enabled in the test build):
  - `fifo_event_notify` (`CONFIG_ISIX_FIFO_EVENT_NOTIFY`) enables `co::fifo_reader`.
  - `sem_event_notify` (`CONFIG_ISIX_SEM_EVENT_NOTIFY`) enables `co::sem_adapter`
    (adds `isix_sem_event_connect/disconnect`; +8 B RAM per semaphore, fifos hold two).

## Model

- `co::task<T>` is the return type of a coroutine function and owns its frame.
- `co::scheduler` owns the event, the timer and the list of suspended coroutines.
  `step()` waits for events (unless something is ready), dispatches signalled primitives,
  fires due timers and resumes ready coroutines. `run()` loops `step()` until idle.
- `co::run(sched, task)` drives a scheduler until the given task completes and returns its
  value; `co::run(task)` uses a temporary scheduler. `co::spawn(sched, task)` queues a task
  that the caller keeps and drives with `step()`/`run()`.
- `co_await task` nests coroutines without symmetric transfer: the child runs from the
  scheduler and the parent is resumed when the child finishes.
- All scheduler structures belong to the scheduler thread. Other threads and ISRs only change
  primitive state under a critical section and set an event bit.
- Several schedulers can live on different tasks. A primitive belongs to exactly one scheduler;
  awaiting it from a coroutine of another scheduler returns `ISIX_EINVARG`.

## Primitives

| Primitive | Thread | ISR | Awaited in a coroutine |
|-----------|--------|-----|------------------------|
| `co::event` (auto-reset, latched, wakes all waiters) | `set()` | `set_isr()` | `co_await ev`, `ev.wait_for(t)` |
| `co::semaphore(sched, initial, limit)` | `signal()`, `try_take()` | `signal_isr()` | `sem.take([t])` |
| `co::mutex` (not recursive, FIFO hand-off) | `try_lock()` | - | `mtx.lock([t])`, `mtx.scoped([t])` |
| `co::channel<T, N>` | `try_send()`, `try_recv()` | `try_send_isr()`, `try_recv_isr()` | `send(v[, t])`, `recv([t])` |
| `co::sleep_ms(ms)`, `co::sleep_ticks(n)` | - | - | `co_await` |
| `co::fifo_reader<T>(sched, fifo)` | regular `isix::fifo` | `push_isr()` | `rd.read([t])`, `rd.try_read()` |
| `co::sem_adapter(sched, sem)` | `isix_sem_signal` | `isix_sem_signal_isr` | `ad.take([t])`, `ad.try_take()` |

Results: awaiters return an `int` (`ISIX_EOK`, `ISIX_ETIMEOUT`, `ISIX_EDESTROY`,
`ISIX_EINVARG`); `recv` and `read` return `std::optional<T>` (`std::nullopt` on timeout).
Destroying a primitive wakes its waiters with `ISIX_EDESTROY`.

Timeouts: `ISIX_TIME_INFINITE` is `0` (no timeout) and `ISIX_TIME_DONTWAIT` means try only.
Each scheduler has 28 user event bits; every primitive uses one (`is_valid()` is false when
they are exhausted).

## Adapters for regular isix objects

- `co::fifo_reader<T>` connects to an `isix::fifo<T>` (or an `osfifo_t`) through
  `isix_fifo_event_connect`. It must be the only consumer of that fifo. The kernel raises the
  event after the item is counted, so a reader never misses an element.
- `co::sem_adapter` connects to an `ossem_t` through `isix_sem_event_connect`. The event is
  raised only when a signal stores a token because no task waits; a task blocked in
  `isix_sem_wait` receives the token directly and has priority over the coroutine. A semaphore
  and a fifo accept one connected event at a time (`ISIX_EBUSY` otherwise, adapter `is_valid()`
  is false). Tokens added with `isix_sem_reset` raise no event.
- Adapters disconnect from the kernel object in their destructor.

## Patterns

- Thread to coroutine result: `co::channel<T, 1>` with `try_send()` from the thread.
- ISR to coroutine: `event::set_isr()`, `semaphore::signal_isr()` or `channel::try_send_isr()`.
- Coroutine to thread: write to a regular `isix::fifo` with `push()` and read it in a thread.

## Exceptions

With exceptions enabled `unhandled_exception` stores the exception, which `run()` rethrows.
With `-fno-exceptions` it calls `std::terminate()`.

## Pitfalls

- Do not rely on `std::coroutine_handle::done()` on this toolchain; completion is tracked in
  the promise.
- Never call Unity `TEST_ASSERT*` inside a coroutine body, an ISR or a vtimer callback (Unity
  uses `longjmp`). Store results and assert after `run()`.
- Destroy primitives and tasks in the scheduler thread and before the scheduler itself.
- Do not call `coroutine_handle::resume()` from an ISR.
- Priority inheritance applies to the RTOS task running the scheduler, not to a coroutine.
- Waiting on a plain `isix::semaphore` or `isix::mutex` from a coroutine blocks the whole
  scheduler task; use `co::sem_adapter` or the `co::` primitives instead.

## Tests

[`tests/libisix/11_coroutines.cpp`](../../tests/libisix/11_coroutines.cpp) covers the scheduler,
all primitives, nesting, ISR and multi-thread use. Build and run `isixtests` under QEMU as
described in [README.md](../../README.md) and [unit_test_qemu.md](unit_test_qemu.md).
