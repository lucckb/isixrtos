#!/usr/bin/env bash
# Verify that the PendSV context switch handler in an ARM ELF is built safely.
#
# The handler is a naked function, so the compiler must not place any code
# of its own in it. The script checks that:
#   R1  no instruction before the callee-saved register save writes r4-r11/sp
#   R2  the only call in the handler is the one to _isixp_schedule
#   R3  the handler is small (no scheduler code inlined into it)
#
# Usage: check_pendsv.sh ELF [STAMP]
#   STAMP  file touched when all checks pass
#   OBJDUMP, NM  override the binutils tool names
#
# Author: Lucjan Bryndza

set -euo pipefail

objdump_bin="${OBJDUMP:-arm-none-eabi-objdump}"
nm_bin="${NM:-arm-none-eabi-nm}"
sym="pend_svc_isr_vector"
max_size=$((0x80))

if [[ $# -lt 1 || $# -gt 2 ]]; then
	sed -n '2,15p' "$0"
	exit 2
fi
elf="$1"
stamp="${2:-}"

if [[ ! -f "$elf" ]]; then
	echo "check_pendsv: no such file: $elf" >&2
	exit 2
fi

dump="$("$objdump_bin" -d "$elf")"
code="$(awk -v s="<${sym}>:" '
	$2 == s { on = 1; next }
	on && NF == 0 { exit }
	on { print }
' <<<"$dump")"

if [[ -z "$code" ]]; then
	echo "check_pendsv: $sym not found in $elf" >&2
	exit 2
fi

fail=0

# R1: nothing may clobber r4-r11 or sp before they are stored
r1_bad="$(awk -F'\t' '
	{
		mnem = $3; ops = $4
		if (mnem ~ /^stmdb/ && ops ~ /^r0!, \{r4/) exit
		if (mnem ~ /^(str|cmp|tst|b|bl|bx|it|vstm|push|stm|msr|dmb|dsb|isb|clrex|nop)/) next
		split(ops, a, ",")
		if (a[1] ~ /^(r[4-9]|r10|r11|sl|fp|sp)$/) print $0
	}
' <<<"$code")"
if [[ -n "$r1_bad" ]]; then
	echo "check_pendsv: R1 failed, code before the register save clobbers a callee-saved register:" >&2
	echo "$r1_bad" >&2
	fail=1
fi

# R2: exactly one call, to the scheduler
calls="$(awk -F'\t' '$3 ~ /^(bl|blx)(\.w)?$/ { print $4 }' <<<"$code")"
ncalls="$(grep -c . <<<"$calls" || true)"
if [[ "$ncalls" -ne 1 ]] || ! grep -Eq '<_isixp_schedule(\.[A-Za-z0-9_.]+)?>' <<<"$calls"; then
	echo "check_pendsv: R2 failed, expected a single call to _isixp_schedule, found $ncalls: $(tr '\n' ' ' <<<"$calls")" >&2
	fail=1
fi

# R3: size
size_hex="$("$nm_bin" -S "$elf" | awk -v s="$sym" '$4 == s { print $2 }')"
size=$((16#${size_hex:-0}))
if [[ "$size" -eq 0 || "$size" -gt "$max_size" ]]; then
	echo "check_pendsv: R3 failed, handler size is $size bytes (limit $max_size)" >&2
	fail=1
fi

if [[ "$fail" -ne 0 ]]; then
	exit 1
fi

if [[ -n "$stamp" ]]; then
	touch "$stamp"
fi
echo "check_pendsv: $elf OK ($size bytes)"
