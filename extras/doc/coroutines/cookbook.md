# Coroutines cookbook

Short patterns. The complete, buildable programs are in
[extras/examples/coroutines](../../examples/coroutines); build them with `-Dexamples=true`.

## 1. Interrupt to coroutine

The interrupt only signals; the coroutine does the work. `set_isr()` never touches a coroutine frame.

```cpp
isix::co::event g_dma_done(sched);

ISIX_ISR_VECTOR(dma_isr_vector) { clear_flags(); g_dma_done.set_isr(); }

isix::co::task<int> transfer(std::span<std::byte> buf)
{
	start_dma(buf);
	co_return co_await g_dma_done.wait_for(isix::ms2tick(100));   // ISIX_ETIMEOUT if it never ends
}
```

For a stream of bytes use `stream_buffer::write_isr()`; for messages `channel::try_send_isr()`.

## 2. Thread to coroutine

Any thread may call `event::set`, `semaphore::signal`, `channel::try_send` and
`stream_buffer::write`. To start new work on a scheduler use `scheduler::post(task)`:

```cpp
low_priority_executor.post(handle_request(request_id));   // frame allocated by the caller
```

## 3. Coroutine to thread

Use a regular isix object the thread already waits on:

```cpp
isix::co::task<void> producer(isix::fifo<Msg>& out)
{
	for (;;) {
		co_await isix::co::sleep_ms(100);
		out.push(make_msg(), isix::ms2tick(10));   // short timeout, a full fifo blocks the scheduler task
	}
}
```

Prefer `try`-style calls (`ISIX_TIME_DONTWAIT`) inside coroutines: a blocking kernel call stops
every coroutine on that scheduler.

## 4. Timeout on an operation

```cpp
const auto r = co_await isix::co::with_timeout(read_register(addr), isix::ms2tick(50));
if (!r) {
	// r.error() is ISIX_ETIMEOUT; the operation was canceled and has finished
}
```

## 5. First of two: data or button

```cpp
auto data = wait_for_packet();
auto key = wait_for_button();
if (co_await isix::co::when_any(data, key) == 0) {
	handle(data.value());
}   // the loser was canceled and finished before this line
```

See `04_timeouts_and_cancel.cpp`.

## 6. Timeout on a blocking bus call

Wrap the blocking call with `worker_thread`, then limit the wait:

```cpp
isix::co::task<int> read_sensor(isix::co::worker_thread& w)
{
	auto job = [] { return i2c.transaction(addr, transfer); };
	co_return co_await w.run(job);
}
// caller:
const auto r = co_await isix::co::with_timeout(read_sensor(worker), isix::ms2tick(50));
```

A job cannot be interrupted, so a timeout ends the wait only after the call returned; give the
driver its own timeout shorter than yours.

## 7. No heap

```cpp
isix::co::frame_pool<160, 4> g_frames;                       // 4 frames of up to 152 bytes
isix::co::task<void> session(isix::co::frame_pool<160, 4>&, int id);

auto t = session(g_frames, 1);
if (!t.valid()) { /* pool exhausted or frame too big: ISIX_ENOMEM on spawn */ }
```

`extras/scripts/coro_frames.sh image.elf` prints the frame size of every coroutine so the pool block
can be sized; remember the 8 byte header.

## 8. Supervisor with a scope

```cpp
isix::co::async_scope scope(sched);
scope.spawn(connection_handler(1));
scope.spawn(connection_handler(2));
// ... later
scope.cancel_all();
co_await scope.join();          // returns when every handler has finished
```

## 9. Blocking libraries (libperiph, lwIP sockets)

Call them from a `worker_thread` job instead of from a coroutine (see 6). One worker serves one job
at a time; use several workers for parallel calls. A worker thread's priority decides how the blocking
work competes with the rest of the system.

## 10. Writing an awaitable driver

Complete the operation from the interrupt with an `event` and give it a way to stop the hardware:

```cpp
isix::co::task<int> uart_write(std::span<const std::byte> data, ostick_t timeout)
{
	start_tx(data);                                    // TXE interrupt feeds the bytes
	int rc = co_await tx_done.wait_for(timeout);       // ISR: tx_done.set_isr(ISIX_EOK)
	if (rc != ISIX_EOK) {
		stop_tx();                                     // timeout or cancel: quiesce the hardware first
		co_await tx_stopped.wait_for(isix::ms2tick(5)); // the ISR confirms; the buffer is free again
	}
	co_return rc;
}
```

The rule: the buffer handed to the hardware must stay valid until the interrupt that ends the
operation has run. Waking with a timeout or `ISIX_ECANCELED` does not stop the hardware, so the driver
stops it and waits for the end of the operation before returning. `02_uart_echo.cpp` applies this
to a USART.

## 11. Why does it not wake up?

Build with `co_debug` and look at the scheduler:

```cpp
sched.for_each_task([](const isix::co::task_info& t) {
	dbprintf("%s: %d", t.name ? t.name : "?", static_cast<int>(t.state));
});
const auto s = sched.stats();
dbprintf("waiting %u timers %u live frames %u", s.waiting, s.timers, s.live_frames);
```

Typical causes: the primitive belongs to another scheduler (`ISIX_EINVARG`), a blocking kernel call
froze the scheduler task (`max_resume_us` is large), a timeout of `0` means "forever", or the waker runs
in an ISR but calls the thread variant. `co_trace` adds a hook with every spawn, suspend, resume and
finish, and `co_shuffle` randomizes the resume order of coroutines that are ready together to expose
order dependencies in tests.
