#!/usr/bin/env bash
# Verify that no PendSV can run between isix_start_scheduler() and the
# first task entry (the stack pointer of the task context is not valid yet).
#
# The test image is started in QEMU under GDB. At start_first_task_svc the
# script pends SysTick and PendSV by hand, then lets the image run. The
# check passes when the Unity summary is printed without failures and no
# panic or fault is reported. QEMU and GDB are always killed on exit.
#
# Usage: check_start_window.sh [-t seconds] BINARY ELF
#   BINARY  isixtests.binary to load into QEMU
#   ELF     matching ELF with symbols
#   ISIX_QEMU, GDB  override the tool paths
#
# Run it by hand after changes to the port start code or the PendSV
# handler. It needs QEMU and GDB, so it is not part of the build.
#
# Author: Lucjan Bryndza

set -uo pipefail

timeout_s=60
while getopts "t:h" opt; do
	case "$opt" in
		t) timeout_s="$OPTARG" ;;
		*) sed -n '2,17p' "$0"; exit 2 ;;
	esac
done
shift $((OPTIND - 1))
if [[ $# -ne 2 ]]; then
	sed -n '2,17p' "$0"
	exit 2
fi
bin="$1"
elf="$2"

qemu="${ISIX_QEMU:-$HOME/temporary/qemu-stm32/opt/homebrew/bin/qemu-system-arm}"
[[ -x "$qemu" ]] || qemu="$(command -v qemu-system-arm || true)"
gdb_bin="${GDB:-arm-none-eabi-gdb}"
if [[ -z "$qemu" || ! -x "$qemu" ]]; then
	echo "check_start_window: qemu-system-arm not found (set ISIX_QEMU)" >&2
	exit 2
fi
for f in "$bin" "$elf"; do
	[[ -f "$f" ]] || { echo "check_start_window: no such file: $f" >&2; exit 2; }
done

tmp="$(mktemp -d)"
qpid=""
gpid=""
cleanup() {
	[[ -n "$gpid" ]] && kill -KILL "$gpid" 2>/dev/null
	[[ -n "$qpid" ]] && kill -KILL "$qpid" 2>/dev/null
	rm -rf "$tmp"
}
trap cleanup EXIT

port=$((20000 + RANDOM % 20000))
"$qemu" -M olimex-stm32-h405 -kernel "$bin" -nographic -semihosting \
	-S -gdb "tcp::$port" </dev/null >"$tmp/qemu.log" 2>&1 &
qpid=$!

# ICSR: PENDSTSET | PENDSVSET
cat >"$tmp/cmd.gdb" <<G
set pagination off
set confirm off
target remote :$port
hbreak start_first_task_svc
continue
set {unsigned int}0xE000ED04 = 0x14000000
delete
continue
G

"$gdb_bin" -batch -nx -x "$tmp/cmd.gdb" "$elf" >"$tmp/gdb.log" 2>&1 </dev/null &
gpid=$!

elapsed=0
verdict=""
while (( elapsed < timeout_s )); do
	if grep -Eq "ISIX panic|HardFault|Hard fault|BusFault" "$tmp/qemu.log"; then
		verdict="fault"
		break
	fi
	if grep -Eq "[0-9]+ Tests [0-9]+ Failures" "$tmp/qemu.log"; then
		verdict="done"
		break
	fi
	if ! kill -0 "$qpid" 2>/dev/null; then
		verdict="exit"
		break
	fi
	sleep 1
	(( elapsed++ ))
done

summary="$(grep -E "[0-9]+ Tests [0-9]+ Failures" "$tmp/qemu.log" | tail -n 1)"
if [[ "$verdict" == "done" || "$verdict" == "exit" ]] && [[ "$summary" =~ ([0-9]+)\ Tests\ 0\ Failures ]]; then
	echo "check_start_window: OK ($summary)"
	exit 0
fi

echo "check_start_window: FAILED (${verdict:-timeout} after ${elapsed}s)" >&2
tail -n 15 "$tmp/qemu.log" >&2
tail -n 5 "$tmp/gdb.log" >&2
exit 1
