# Coroutines in ISIX (`isix::co`)

ISIX is, to our knowledge, the first classic preemptive RTOS for Cortex-M that ships C++20
coroutines as a first-class, kernel-aware feature: coroutines wake from threads and interrupts
without polling, allocate nothing on the hot path, can run with no heap, cancel cooperatively
and integrate with the scheduler's tickless idle.

The layer is header-only, optional (zero cost when it is not used) and built on the public C
kernel API. Many coroutines run on **one RTOS task** under a `co::scheduler`; the rest of the
system keeps working as before.

## Why coroutines on an RTOS

| | Thread + semaphore | `isix::co` coroutine |
|---|---|---|
| Memory per concurrent activity | a task stack (hundreds of bytes at least) plus a TCB | one heap, pool or static frame; sizes are listed in [benchmarks.md](benchmarks.md) |
| Waiting for an interrupt | blocks a whole task | suspends a coroutine, the scheduler task serves the others |
| Context switch | RTOS switch | resume of a function |
| Timeouts, cancellation, "first of N" | hand-written per case | `with_timeout`, `cancel()`, `when_any` |
| Blocking drivers | native | through `worker_thread` |

Coroutines do not replace threads: priorities, priority inheritance and preemption still come from
the kernel. Use one executor (a scheduler on its own thread) per priority level you need.

## A first example

```cpp
#define CONFIG_ISIX_CPP_COROUTINES 1
#include <isix.h>

isix::co::task<void> blink(isix::co::scheduler&, unsigned period_ms)
{
	for (;;) {
		toggle_led();
		co_await isix::co::sleep_ms(period_ms);
	}
}

isix::co::task<int> read_sensor(isix::co::event& data_ready)
{
	// The interrupt calls data_ready.set_isr(); give up after 50 ms
	const auto rc = co_await data_ready.wait_for(isix::ms2tick(50));
	co_return rc == ISIX_EOK ? sample() : -1;
}
```

`isix::co::run(sched, task)` drives a scheduler until a task completes; `scheduler_thread` gives
a scheduler its own RTOS thread and `post()` hands tasks to it from any thread.

## Features

| Area | What you get |
|---|---|
| Core | `task<T>`, `scheduler`, `run`, `spawn`, `scheduler_thread`, `async_scope`, `post` |
| Primitives | `event`, `semaphore`, `mutex`, `channel<T,N>`, `stream_buffer<N>`, `sleep_*`, `yield` |
| Adapters | `fifo_reader`, `sem_adapter` for regular isix objects |
| Composition | `when_all`, `when_any`, `with_timeout` |
| Cancellation | `task::cancel()`, `cancelled()`, `stop_source` (thread and ISR) |
| Memory | heap (default), `frame_storage<N>`, `frame_pool<Size,Count>`, `set_frame_allocator`; failed allocation gives an empty task and `ISIX_ENOMEM`, never a panic |
| Blocking code | `worker_thread::run` offloads a callable to a thread |
| Generators | `generator<T>` for synchronous sequences |
| Diagnostics | statistics, task names, thread ownership checks, trace hook, randomized resume order for tests |
| Power | an idle scheduler sleeps in an event wait and works with tickless idle |

## Measured on the STM32F411E-DISCO

| Item | Value |
|---|---|
| Frame of a `task<void>` without suspension points | 56 bytes (+ 8 byte header) |
| Cost of one more `co_await` in a function | 48 bytes |
| `sizeof(wait_node)` / `sizeof(scheduler)` | 44 / 236 bytes |
| Library code for the types the tests use (-Os, LTO) | about 8 KB |
| Interrupt to coroutine resume | 19.6 us average, 9.0 us for a thread on a semaphore (same build) |

Details and the method are in [benchmarks.md](benchmarks.md).

## Documents

- [guide.md](guide.md) - model, threading rules, timeouts, cancellation, frames, executors.
- [reference.md](reference.md) - every class and function with its contract and error codes.
- [cookbook.md](cookbook.md) - patterns: interrupt to coroutine, timeouts, offloading, drivers.
- [internals.md](internals.md) - data structures, wake-up protocol, frame lifetime, limits.
- [benchmarks.md](benchmarks.md) - sizes and timings with the method to repeat them.
- Examples: [extras/examples/coroutines](../../examples/coroutines).
