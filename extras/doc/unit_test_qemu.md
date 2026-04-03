# Unit tests 

## Preface

Unit tests can be run on a startup board such as STM32F411E-DISCO or on QEMU (emulated uC but more convenient development).

Builds are documented first with the repository **`configure`** helper; each section also lists an **Alternative** with plain **`meson setup`**, if you prefer not to use that script.


## Unit tests on STM32F411E-DISCO dev board

From the repository root (Meson build directory is `build`; see `./configure --help`). Prefer the helper script; if you do not want to use it, use the **Alternative** block (pure Meson).

```bash
./configure build disco
meson compile -C build
```

**Alternative** — same layout without the repository `configure` script:

```bash
meson setup --cross-file arm.ini --cross-file cortex/m4.ini \
	--cross-file stm32/f411vet6.ini \
	--buildtype=debug -Doptimization=s -Db_lto=true \
	-Dcrystal_hz=8000000 -Dtest=true build
meson compile -C build
```

The ***isixtests*** ELF is produced under ***build/tests/libisix/***. Use it to program the target board.
The serial console is configured using USART1_TX pin (PA9), with serial baudrate 115200.

## Unit tests on the QEMU

The current version of QEMU allows emulation of the *olimex-stm32-h405* board. Unfortunately, the implementation of the *STM32F405* processor contains bugs in the simulation of the T2-T5 timers making it necessary in the original version (as of October 2023) to make a patch containing fixes for these timers and then build the qemu from sources.

### Compile QEMU

To compile QEMU first download the patch from the following location: http://bryndza.boff.pl/downloads/prv/qemu-v10.1.2-STM32-fix-raise-interrupt-time.patch and then clone the QEMU sources and apply the patch and compile the application.

```bash
git clone -b v10.1.2 --recurse-submodules https://gitlab.com/qemu-project/qemu.git
cd qemu
patch -p1 -d . < qemu-v10.1.2-STM32-fix-raise-interrupt-time.patch
./configure --enable-debug --disable-xen --disable-werror --target-list="arm-softmmu"
make
```
(Here `./configure` is **upstream QEMU’s** build configure, not the Meson helper `configure` in the ISIX repository root.)

After compilation, which may take a while, we will find qemu-system-arm in the build directory, which we can use in this directory or copy to another location such as /usr/local/bin.

### Compile tests
Compilation of tests on QEMU uses the QEMU-specific cross file (`stm32/f405rg_qemu.ini`). From the repository root, either run `./configure build qemu` or the **Alternative** `meson setup` below (no repository helper).

```bash
./configure build qemu
meson compile -C build
```

**Alternative** — without the repository `configure` script:

```bash
meson setup --cross-file arm.ini --cross-file cortex/m4.ini \
	--cross-file stm32/f405rg_qemu.ini \
	--buildtype=debug -Doptimization=g -Db_lto=true \
	-Dcrystal_hz=8000000 -Dtest=true build
meson compile -C build
```

The firmware is built with `QEMU_NO_RCC_PERIPH` (QEMU lacks a real RCC as on silicon). Artifact: ***isixtests.binary*** under ***build/tests/libisix/***.

### Semihosting and exiting QEMU

The test image uses **ARM semihosting** for a clean **process exit** when the run completes: `_external_exit()` in `tests/libisix/utils/board_boot.cpp` issues **SYS_EXIT** (`bkpt #0xAB` with the usual registers). QEMU must be started with **`-semihosting`**; otherwise the CPU may fault or sit in a loop instead of shutting down.

Without semihosting, QEMU would keep emulating after tests finish; with `-semihosting`, QEMU exits when the guest performs that semihost call.

### Running tests

The tests can be run continuously, or we can also run them with an additional gdb session to which we can connect and debug the tests. To run tests in continuous mode:

```bash
qemu-system-arm -M olimex-stm32-h405 -semihosting \
	-kernel build/tests/libisix/isixtests.binary -nographic
```

To run tests while waiting for a gdb session:

```bash
qemu-system-arm -M olimex-stm32-h405 -semihosting \
	-kernel build/tests/libisix/isixtests.binary -nographic -S -s
```

In this mode, QEMU will wait for a gdb session to start on port 1234 before running the test.
