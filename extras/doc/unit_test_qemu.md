# Unit tests 

## Preface

Unit tests can be run on a startup board such as STM32F411E-DISCO or on a QEMU. Due to more convenient development, it is recommended to use QEMU.


## Unit tests on STM32F411E-DISCO dev board

In the first step, go to the directory 'libisix/tests/libisix and compile the application. 
```bash
cd tests/libisix/
waf configure --cpu=stm32f411vet6 --debug --crystal-hz=8000000
waf
```
In the ***build/test/libisix/*** directory ***isixunittests*** file will be created. The elf file can be used to program target board.
The serial console is configured using USART1_TX pin (PA9), with serial baudrate 115200.

## Unit tests on the QEMU

The current version of QEMU allows emulation of the *olimex-stm32-h405* board. Unfortunately, the implementation of the *STM32F405* processor contains bugs in the simulation of the T2-T5 timers making it necessary in the original version (as of October 2023) to make a patch containing fixes for these timers and then build the qemu from sources.

### Compile QEMU

To compile QEMU first download the patch from the following location: http://bryndza.boff.pl/downloads/prv/0001-STM32-fix-raise-interrupt-time.patch and then clone the QEMU sources and apply the patch and compile the application.

```bash
git clone --recurse-submodules https://gitlab.com/qemu-project/qemu.git
cd qemu
patch -p1 -d < 0001-STM32-fix-raise-interrupt-time.patch
./configure --enable-debug --disable-xen --disable-werror --target-list="arm-softmmu"
make
```
After compilation, which may take a while, we will find qemu-system-arm in the build directory, which we can use in this directory or copy to another location such as /usr/local/bin.

### Compile tests
The compilation of tests is similar to the previous case. Only we have to choose a different microcontroller. A separate microcontroller type for the QEMU was intentionally created, because the simulated microcontroller does not contain an RCC block and is configured to default to 168MHz.

```bash
cd tests/libisix/
waf configure --cpu=stm32f405rg_qemu --debug --crystal-hz=8000000
waf
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
