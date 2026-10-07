# Coroutines guide

## Enabling

The layer needs a toolchain with C++20 coroutines (the project builds with `gnu++23`, exceptions and
RTTI off). Define `CONFIG_ISIX_CPP_COROUTINES` before including `isix.h`, or pass
`-DCONFIG_ISIX_CPP_COROUTINES=1`. A program that does not include the coroutine headers pays
nothing for them.

Optional kernel features used by two adapters (both enabled in the test build):

- `fifo_event_notify` - `co::fifo_reader`
- `sem_event_notify` - `co::sem_adapter`

A scheduler serves up to 8 adapters (`fifo_reader`, `sem_adapter`) because the kernel can only raise
an event bit; every other primitive has no such limit.

Diagnostics are selected with Meson options `co_debug`, `co_trace` and `co_shuffle`
(`auto`, `enabled`, `disabled`). `auto` turns them on in test builds that are not optimized for
size, so a size-optimized board image carries none of their cost.

## The model

A coroutine function returns `co::task<T>`. The task is lazy: nothing runs until it is handed to
a scheduler, and it owns its frame, so destroying the task destroys the coroutine.

A `co::scheduler` serves one RTOS task. It owns one isix event and, once a timeout is used,
one virtual timer. `step()` does one round:

1. wait for events, unless a coroutine is ready (an idle scheduler sleeps, so tickless idle works);
2. dispatch the primitives that were signalled;
3. fire due timers;
4. resume every coroutine that was ready when the round started.

`run()` repeats `step()` until the scheduler is idle. `co::run(sched, task)` runs one task to
completion and returns its value. `co::spawn(sched, task)` queues a task that the caller keeps and
drives with `step()` or `run()`; it returns `ISIX_ENOMEM` for a task whose frame could not be
allocated.

`co_await some_task()` runs a child task from the scheduler and resumes the parent when the child
finishes. There is no symmetric transfer, so the stack never grows with nesting.

## Threading rules

Everything that belongs to a scheduler (its ready and timer lists, waiter lists, spawned tasks)
is touched only by the scheduler thread. Other threads and interrupts reach a coroutine only
through the state of a primitive, changed in a critical section, and a wake-up signal.

| Operation | Scheduler thread | Other thread | ISR |
|---|---|---|---|
| `co_await`, `spawn`, `run`, `step`, `task::cancel`, `async_scope` calls | yes | no | no |
| `scheduler::post(task)` | yes | yes | no (the task frame is allocated by the caller) |
| `event::set`, `semaphore::signal`, `channel::try_*`, `stream_buffer::write`, `stop_source::request_stop` | yes | yes | only the `*_isr` variants |
| `worker_thread::run` | yes | no | no |
| `frame_pool` allocation | yes | yes | no |
| Creating or destroying a primitive | when nobody waits | no | no |

Starting a coroutine function allocates its frame, so it must not be done in an ISR either.
A scheduler is driven by one thread for its whole life: the first `step()` binds it, and every later
`step()`, `run()` or `run_forever()` must come from that thread. It may be created in one thread and
started in another. The primitives rely on this: `event::set()` called from the driving thread
updates the event in place instead of going through the scheduler inbox. With `co_debug` enabled, using
a scheduler from a thread other than the one that steps it stops the system with a panic message.

## Timeouts and results

Every waiting operation takes a timeout in ticks. **`ISIX_TIME_INFINITE` is `0`**, so a timeout
of `0` waits forever; `ISIX_TIME_DONTWAIT` tests without waiting.

| Awaited | Result |
|---|---|
| `event`, `semaphore::take`, `mutex::lock`, `channel::send`, `worker_thread::run` | `int`: `ISIX_EOK`, `ISIX_ETIMEOUT`, `ISIX_EDESTROY`, `ISIX_ECANCELED`, `ISIX_EINVARG`, `ISIX_EBUSY` |
| `channel::recv`, `fifo_reader::read` | `co::result<T>` (`std::expected<T,int>`) with the same error codes |
| `stream_buffer::read` | `co::result<std::size_t>` |
| `with_timeout` | `co::result<T>` |
| `when_any` | index of the first finished task |

`co::result` is used without exceptions: test it with `has_value()` or `operator bool`, read the
code with `error()`, or use `value_or()`. Calling `value()` on an error aborts.

Destroying a primitive wakes its waiters with `ISIX_EDESTROY`. `event::set(status)` lets the
signaller choose the code the waiters receive.

## Cancellation

Cancellation is cooperative. `task::cancel()` (scheduler thread) marks the task and every child it
awaits at the moment. The operation the innermost coroutine is suspended on completes with
`ISIX_ECANCELED`, and every later suspension of a canceled coroutine returns immediately with
that code, so a canceled coroutine unwinds through its normal code path (locks are released by
`mutex::scoped`, destructors run). A coroutine that was already woken but not yet resumed finishes
that wait normally and sees the cancellation at its next `co_await`.

A coroutine can ask `co_await co::cancelled()` and `task::error()` reports `ISIX_ECANCELED` for a
task that finished after a request. `co::stop_source` carries a request from another thread
(`request_stop()`) or an interrupt (`request_stop_isr()`) to a bound task. Cancellation never
destroys a frame: a coroutine that is not suspended on a `co::` primitive finishes its current
work first.

## Composition

```cpp
auto a = read_sensor_a();
auto b = read_sensor_b();
co_await isix::co::when_all(a, b);            // both finished; results in a.value(), b.value()

auto fast = query();
auto stop = wait_for_button();
const auto first = co_await isix::co::when_any(fast, stop);   // losers are canceled and finish
                                                              // before the caller resumes

const auto r = co_await isix::co::with_timeout(query(), isix::ms2tick(50));
if (!r) { /* r.error() == ISIX_ETIMEOUT */ }
```

The tasks stay owned by the caller (`when_all` and `when_any` take references; `with_timeout` also
accepts a task rvalue). Results are read from the tasks afterwards: `value()`, `error()`,
`result()`. The caller resumes only when every child has finished, so a canceled child may use
the caller's locals safely.

## Detached tasks

`co::async_scope` owns coroutines that nobody awaits: `spawn(task)`, `cancel_all()`, `size()` and
`co_await scope.join()`. Finished coroutines are destroyed by the scheduler after the round, never
inside their own final suspension. `scheduler::post(task)` hands a detached task to a scheduler from
any thread; the scheduler adopts it on its next round.

## Coroutine frames

A coroutine frame is allocated once, when the coroutine function is called. The default source
is the heap. Two other sources avoid it, chosen by passing an object as the first parameter of the
coroutine function:

```cpp
isix::co::frame_storage<256> storage;            // one frame at a time, size includes an 8 byte header
isix::co::task<void> job(isix::co::frame_storage<256>&);

isix::co::frame_pool<128, 4> pool;               // four blocks of 128 bytes
isix::co::task<void> worker(isix::co::frame_pool<128, 4>&, int id);
```

`set_frame_allocator()` replaces the global source. When an allocation fails the call returns an
empty task: `spawn` returns `ISIX_ENOMEM`, `post` and `async_scope::spawn` return `ISIX_ENOMEM`,
`co::run` stops with a message. The system does not panic.

The size of a frame is only known to the compiler. `extras/scripts/coro_frames.sh <elf>` lists the
frame size of every coroutine in an image, and `scheduler::stats()` reports live and peak frame
bytes when `co_debug` is on. The compiler does not share frame slots between consecutive
`co_await`s, so a long coroutine grows with its number of awaits: split long coroutines into
child tasks. Heap allocation elision does not happen with these tasks.

## Executors and priorities

`co::scheduler_thread` is a scheduler with its own RTOS thread. A common layout is one executor at
high priority for protocols driven by interrupts and one at low priority for the application;
they talk through `channel::try_send` or `event::set` (the "thread" variants). A primitive belongs
to the scheduler it was created with; awaiting it from another scheduler returns
`ISIX_EINVARG`.

## Tickless idle

An idle scheduler waits in `isix_event_wait` and its timers live in the kernel timer list, so the
kernel can stop the periodic tick while all coroutines wait. No coroutine code is needed for this.

## Pitfalls

- Do not rely on `std::coroutine_handle::done()` on this toolchain; completion is kept in the promise.
- Never call Unity `TEST_ASSERT*` in a coroutine body, an ISR or a timer callback; store results
  and assert afterwards.
- Destroy primitives and tasks before the scheduler, and in the scheduler thread.
- Waiting on a plain `isix::semaphore` or `isix::mutex` inside a coroutine blocks the whole scheduler
  task. Use the `co::` primitives, `co::sem_adapter`, or `worker_thread`.
- Priority inheritance applies to the RTOS task that runs the scheduler, not to a coroutine.
- `stop_source::bind` stores a pointer to the task's promise: call `unbind()` before the task goes away.
- With exceptions enabled, an exception escaping a coroutine is stored in its promise; with
  `-fno-exceptions` it terminates.
