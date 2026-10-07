# Coroutines internals

This document describes how the layer works for people who change it or write new awaitable
types. Application code only needs the [guide](guide.md).

## Objects

| Object | Lives in | Role |
|---|---|---|
| `scheduler` | the owner | one isix event, one lazily created vtimer, the ready, waiting, timer, inbox, posted and dead lists |
| `promise_common` | the coroutine frame | start entry of the coroutine, completion and cancellation flags, the awaited child, the continuation |
| `wait_node` | the awaiter, so in the frame | one suspension: three intrusive links (primitive waiters, scheduler timers, scheduler waiting/ready), the coroutine handle, deadline, result code and an optional `on_abort` |
| `signal_link` | a primitive | entry of the scheduler inbox |

All links are intrusive and doubly linked, so removing a node is O(1) and needs no allocation. A node
removes itself from every list in its destructor, which makes destroying a suspended coroutine safe.
A `wait_node` is 44 bytes.

## Wake-up protocol

Primitives keep their state (a counter, a ring, a latched flag) behind a critical section. A thread or
an interrupt that changes the state calls `scheduler::signal(link)` or `signal_isr(link)`:

```
critical section:  update state;  first = !link.queued;  if (first) { link.queued = true; inbox.push_back(link) }
if (first)         set the inbox bit in the scheduler event
```

`step()` takes the inbox bit, moves the whole inbox list to a local list in one critical section, and for
each link clears `queued` **before** calling the primitive's dispatch function, which reads the
state in a critical section and wakes waiters.

No wake-up is lost: a signal that arrives before `queued` is cleared does not push the link again, but
the dispatch that follows sees the new state; a signal after that pushes the link again and sets the
bit, which stays latched in the kernel event until the next wait. A dispatch without work is harmless.
Interrupts only ever touch the primitive, never a node in a coroutine frame, so a frame can be
destroyed while an interrupt is running.

A signal from the thread that drives the scheduler skips the inbox: `event::set()` checks that it runs in
that thread (never in an ISR, where the interrupted thread could be the driver) and that the event has no
signal queued. It then wakes the waiters directly, or latches the signal without queueing it when
nobody waits. A queued signal keeps its place, so the order of signals is preserved. The
waiter lists are only touched by the driving thread, so the direct path needs no critical section
when it wakes waiters.

`scheduler::signal()` also skips setting the inbox bit while the scheduler resumes coroutines
(`m_resuming`). This is safe because the scheduler looks at the inbox before every blocking wait and
the flag clear is followed by a compiler barrier, so that read cannot move above it.

The kernel-driven adapters (`fifo_reader`, `sem_adapter`) cannot use the inbox because the kernel can
only set a bit of the event: they own one of the eight adapter bits.

## Resume rules

`run_forever()` runs up to four extra rounds of ready coroutines per step without polling the kernel
event, so a ping-pong between two coroutines does not pay for a poll on every hand-off. Kernel
bits (adapters, timer, `post`, `stop`) are therefore seen up to four rounds later; `step()` itself always
runs one round.

Only `scheduler::step()` resumes coroutines, only in the scheduler thread, never from a primitive's
dispatch and never from an interrupt. A node is unlinked before its coroutine is resumed, and each
step resumes the entries that were ready when it started, so a coroutine that wakes another one
during a round does not run it until the next round. `yield()` relies on this.

## Frame lifetime

- The frame belongs to its `task<T>`, or after `spawn`/`post` to an `async_scope` or the scheduler.
  Destroying a task destroys a suspended coroutine; its awaiters unlink their nodes.
- A detached coroutine reports its end to the scope from its final suspension, which puts it on the
  scheduler's dead list. The scheduler destroys the frame after the resume loop, never inside the
  coroutine's own final suspension.
- A child task's continuation is the awaiter's node in the parent's frame. `when_all`, `when_any`
  and `with_timeout` replace it with a join node and a finish callback; their awaiter unhooks
  children that are still running if the awaiting coroutine is destroyed.
- Frames are allocated by `operator new` of the promise. The 8 byte header in front of a frame holds
  the function that releases it, so a frame always returns to the source it came from, even after
  `set_frame_allocator` changed the global one.

## Cancellation

`task::cancel()` follows the `child` pointers to the innermost awaited coroutine and marks every
coroutine on the way. The innermost one is found in the scheduler's waiting list by its coroutine
handle; its node is woken with `ISIX_ECANCELED`, or, when the node has an `on_abort` callback, the
callback is called instead of the wake-up. `on_abort` is also what a timeout calls. The callback must
cause exactly one later wake-up; this is how `with_timeout` and `when_any` cancel children and how
`worker_thread::run` defers the end of the wait until the job finished. A coroutine that is already on the
ready queue is not woken again; its next suspension returns `ISIX_ECANCELED` because every
`await_suspend` starts by checking the flag.

## Timers

Timeouts are nodes on a deadline-ordered list. The scheduler arms its one vtimer for the nearest
deadline only when that deadline changes. The timer callback only sets a bit of the scheduler event;
the nodes are processed in the scheduler thread. Destroying a scheduler starts a fence command on the
timer worker and waits for it, so no command that refers to the scheduler is pending afterwards.

## Limits and toolchain notes

- At most 8 adapters per scheduler; everything else is unlimited.
- `std::coroutine_handle::done()` is not used because it is unreliable on this toolchain; completion is
  a flag in the promise set in the final suspension.
- Symmetric transfer is not used.
- GCC does not elide the frame allocation of these tasks and does not share frame slots between
  consecutive awaiters, so a frame grows with the number of `co_await`s in one function.
- Exceptions and RTTI are off in the project. With exceptions enabled the promise stores the exception.
- A frame cannot be allocated in an interrupt: starting a coroutine function is an allocation.
