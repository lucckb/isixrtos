# Unit tests 

## Preface

Unit tests can be run on a startup board such as STM32F411E-DISCO or on QEMU (emulated uC but more convenient development).


## Unit tests on STM32F411E-DISCO dev board

First, compile the test application with the `-Dtest=true`flag. 
```bash
meson setup --cross-file arm.ini --cross-file cortex/m4.ini --cross-file stm32/f411vet6.ini \
	--buildtype=debug -Doptimization=s -Db_lto=true -Dcrystal_hz=8000000 -Dtest=true builddisco
meson compile -C builddisco
```
In the ***builddisco/test/libisix/*** directory ***isixunittests*** file will be created. The ELF file can be used to program target board.
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
After compilation, which may take a while, we will find qemu-system-arm in the build directory, which we can use in this directory or copy to another location such as /usr/local/bin.

### Compile tests
Compilation of tests on QEMU is similar to the [STM32F411E-DISCO board](#unit-tests-on-stm32f411e-disco-dev-board) but with a different microcontroller cross-file. A separate microcontroller type for the QEMU was intentionally created because the simulated microcontroller does not contain an RCC block and is configured by default to 168MHz.

```bash
meson setup --cross-file arm.ini --cross-file cortex/m4.ini --cross-file stm32/f405rg_qemu.ini \
	--buildtype=debug -Doptimization=g -Db_lto=true -Dcrystal_hz=8000000 -Dtest=true buildqemu
meson compile -C buildqemu
```
Once compiled, a binary file ***isixunittests.binary*** will be created, which we can use to run the tests on QEMU.

### Running tests

The tests can be run continuously, or we can also run them with an additional gdb session to which we can connect and debug the tests. To run tests in continuous mode, issue the following command:
```bash 
qemu-system-arm -M olimex-stm32-h405  -kernel build/tests/libisix/isixunittests.binary -nographic
```
To run tests while waiting for a gdb session, use the following command:
```bash
qemu-system-arm -M olimex-stm32-h405  -kernel build/tests/libisix/isixunittests.binary -nographic -S -s
```
In this mode, QEMU will wait for a gdb session to start on port 1234 before running the test.
