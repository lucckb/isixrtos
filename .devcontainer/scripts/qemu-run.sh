#!/usr/bin/env bash
#
# qemu-run.sh - Run QEMU with the built image
#
# This script is used to run the built image in QEMU.
#
# Usage:
#   ./devcontainer/scripts/qemu-run.sh [image]
#
# The image is the path to the built image. If not provided, it defaults to build/tests/libisix/isixtests.binary.
set -euo pipefail

IMAGE="${1:-build/tests/libisix/isixtests.binary}"

if [[ ! -f "${IMAGE}" ]]; then
	echo "Missing image: ${IMAGE}"
	echo "Build first with:"
	echo "  ./configure build qemu"
	echo "  meson compile -C build"
	exit 1
fi

exec qemu-system-arm -M olimex-stm32-h405 -semihosting -kernel "${IMAGE}" -nographic
