#!/usr/bin/env bash
# Start QEMU paused with a GDB stub on tcp::1234 (-S -gdb).
set -euo pipefail

IMAGE="${1:-build/tests/libisix/isixtests.binary}"

if [[ ! -f "${IMAGE}" ]]; then
	echo "Missing image: ${IMAGE}"
	echo "Build first: ./configure build qemu && meson compile -C build"
	exit 1
fi

pkill -f 'qemu-system-arm.*olimex-stm32-h405' 2>/dev/null || true
sleep 0.2

echo "QEMU GDB server listening on tcp::1234 (CPU halted; attach GDB to continue)"
exec qemu-system-arm -M olimex-stm32-h405 -semihosting \
	-kernel "${IMAGE}" -nographic -S -gdb tcp::1234
