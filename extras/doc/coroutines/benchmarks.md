# Coroutine benchmarks

All numbers are measured; nothing here is an estimate. Sizes come from the build of the unit-test
image, so they include the instantiations the tests use. Timings need the real board: the QEMU
timer does not count cycles.

## Method

| Quantity | How to repeat |
|---|---|
| Frame sizes | `extras/scripts/coro_frames.sh build/tests/libisix/isixtests`; the runtime allocation is the printed size plus the 8 byte frame header |
| Library code size | `arm-none-eabi-nm -C --size-sort <elf> \| grep 'isix::co' \| grep -v TEST_`, summed |
| Structure sizes | the `frame_sizes_report` test prints `sizeof` of the main types |
| Cycle counts | the `coroutines_bench` test group on the STM32F411E-DISCO, DWT cycle counter at 100 MHz |

Run the groups with `meson compile -C build-disco tests/libisix/isixtests`, flash the ELF and read the
console (115200 8N1).

## Results

Board: STM32F411E-DISCO at 100 MHz (1 cycle = 10 ns), `-Os` with LTO, unit-test image (debug checks
of the kernel on, coroutine diagnostics off). Frame sizes are what the compiler passes to the
allocator; add 8 bytes for the frame header.

### Frame sizes (bytes)

| Coroutine | Frame |
|---|---|
| `task<void>` with no `co_await` | 56 |
| `task<int>` with no `co_await` | 64 |
| `task<void>` with one `co_await sleep_ms(1)` | 104 |
| `task<void>` with one `co_await event` | 112 |
| the same with 2, 3, 4 awaits | 160, 208, 256 |
| `task<void>` that awaits a child task | 120 |

Every further `co_await` in the same function adds 48 bytes: the compiler does not share the frame
slots of consecutive awaiters, so split long coroutines into child tasks.

### Structures and code

| Item | Value |
|---|---|
| `sizeof(wait_node)` | 44 bytes |
| `sizeof(task_promise<void>)` | 40 bytes |
| `sizeof(scheduler)` | 240 bytes without `co_debug` (it was 296 bytes with the per primitive handler table) |
| Library code in the test image | 9136 bytes (nm, `isix::co` symbols without test functions, instantiations for the types the tests use; the test image instantiates more of the library than before) |

### Timing

Cycle counts are for 1000 repetitions, taken with the DWT cycle counter; the per operation figure is the
total divided by 1000.

| Operation | Total cycles | Per operation |
|---|---|---|
| `event::set()` with nobody waiting, from the driving thread | 179412 | 179 cycles (1.8 us) |
| `set()` + `co_await ev` completing without a suspension | 533485 | 533 cycles, so the ready `co_await` costs about 350 |
| `set()` + `step()` + resume of a coroutine suspended on an event | 1269016 | 1269 cycles (12.7 us) |
| the same when the wait has a timeout | 1303008 | 1303 cycles |
| create and destroy a `task<void>` frame from the heap | 1307167 | 1307 cycles |
| create and destroy a frame from a `frame_pool` | 448836 | 449 cycles |

### Interrupt to coroutine latency

A 1 ms timer interrupt stamps the cycle counter and signals; the woken code reads the counter. 100 samples,
cycles at 100 MHz:

| Path | min | avg | max |
|---|---|---|---|
| interrupt, `event::set_isr`, scheduler thread, coroutine resumes | 1664 (16.6 us) | 1771 (17.7 us) | 1795 |
| interrupt, `semaphore::signal_isr`, thread resumes | 826 (8.3 us) | 935 (9.4 us) | 959 |

The coroutine path takes 1.9 times as long as a thread waiting on a semaphore: the wake goes through the
kernel event of the scheduler thread, the scheduler drains its inbox and resumes the coroutine. The
design target is at most twice the thread latency; it is met with a small margin. The numbers include
the kernel debug checks of the test image.

Before the signalling path was shortened (one critical section for the state update and the queueing,
one for draining the inbox, no timer work when no timer is pending) the same measurement gave 2026
cycles on average, against 945 for the thread.

Stage breakdown of the earlier 2026 cycle path, measured with a debugger that stops the timers and reads
the cycle counter at breakpoints: 1154 cycles from the interrupt to the return of the kernel event wait in
the scheduler thread, 227 for the inbox drain, 244 for the event dispatch that moves the coroutine to the
ready queue, 98 for the handler loop, 100 for timers and the ready queue, about 138 from the resume to the
first instruction after the `co_await`. The stage breakdown was not repeated after the changes.

### Switch between two tasks

Two threads (or two coroutines on one scheduler thread) hand a token to each other, 500 round trips,
cycles per hand-off (one direction):

| Pair | Cycles per hand-off |
|---|---|
| thread to thread, two semaphores | 1007 (10.1 us) |
| coroutine to coroutine, two events | 987 (9.9 us) |

A switch between coroutines is as fast as a switch between threads here. The hand-off is cheap because
the signal never leaves the scheduler thread: `event::set()` from a coroutine wakes the waiter in place,
without the inbox and without a critical section for the wake-up, and `run_forever()` resumes the next
ready coroutine without polling the kernel event (up to four rounds per step). Before these changes the
same test gave 1606 cycles, against 1028 for the threads. Signals from other threads and from
interrupts still take the inbox path, which is what the interrupt latency above measures.
Coroutines also win on memory (a frame of tens of bytes instead of a task stack) and on the number of
tasks.

### Idle power

An idle scheduler sleeps in `isix_event_wait`: the `idle_scheduler_is_tickless` and
`scheduler_wakes_only_on_events` tests count the tickless sleeps of the kernel while a coroutine
waits for 300 ms and for an event 200 ms away; the kernel reports a handful of long sleeps in both cases.
