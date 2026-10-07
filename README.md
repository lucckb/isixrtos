# ISIX-RTOS v3 mini operating system for Cortex-M0 / M3 / M4 / M7 – functional description and system characteristics

More information at [blog post](https://www.emsyslabs.com/isix-rtos-v3-mini-operating-system-for-cortex-m0-m3-m4-m7-functional-description-and-system-characteristics/)

## Running unit tests in QEMU

From the repository root, the usual way is **`configure`**, which wraps the right cross files and Meson options:

```bash
./configure build qemu
meson compile -C build
```

**Alternative** (same result without `configure` — full `meson setup`):

```bash
meson setup --cross-file arm.ini --cross-file cortex/m4.ini \
  --cross-file stm32/f405rg_qemu.ini \
  --buildtype=debug -Doptimization=g -Db_lto=true \
  -Dcrystal_hz=8000000 -Dtest=true build
meson compile -C build
```

Run on **`olimex-stm32-h405`**. The default is to pass **`-semihosting`**: the test firmware uses ARM semihosting `SYS_EXIT` on shutdown, so QEMU **exits** when tests finish. Omitting `-semihosting` leaves emulation running with no clean exit.

```bash
# default (headless, semihosting on)
qemu-system-arm -M olimex-stm32-h405 -semihosting \
  -kernel build/tests/libisix/isixtests.binary -nographic
```

```bash
# same, wait for GDB on :1234 (keep -semihosting for SYS_EXIT / QEMU exit)
qemu-system-arm -M olimex-stm32-h405 -semihosting \
  -kernel build/tests/libisix/isixtests.binary -nographic -S -s
```

Further detail (patching QEMU for STM32 timers, DISCO build, and so on): [extras/doc/unit_test_qemu.md](extras/doc/unit_test_qemu.md).

## Coroutines

ISIX is, to our knowledge, the first classic preemptive RTOS for Cortex-M that ships C++20 coroutines as a first-class, kernel-aware feature: coroutines wake from threads and interrupts without polling, allocate nothing on the hot path, run with no heap, cancel cooperatively and integrate with the scheduler's tickless idle. Many coroutines share one RTOS task, with timeouts, `when_any`/`when_all`, cancellation, interrupt and stream awaitables, and a bridge to blocking code.

Documentation, cookbook and examples: [extras/doc/coroutines](extras/doc/coroutines/README.md).
