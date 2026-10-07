#!/usr/bin/env bash
# Print coroutine frame sizes found in an ARM ELF: the size argument of the
# operator new (or frame allocator) call in each coroutine ramp function.
# The runtime allocation is this size plus the frame header (8 bytes).
#
# Usage: coro_frames.sh [-f regex] [-n objdump] ELF
#   -f  show only functions whose demangled name matches the regex
#   -n  objdump tool name (default arm-none-eabi-objdump)
#
# Author: Lucjan Bryndza

set -euo pipefail

filter='.'
objdump_bin='arm-none-eabi-objdump'

while getopts "f:n:h" opt; do
	case "$opt" in
		f) filter="$OPTARG" ;;
		n) objdump_bin="$OPTARG" ;;
		*) sed -n '2,8p' "$0"; exit 2 ;;
	esac
done
shift $((OPTIND - 1))

elf="${1:-}"
[[ -f "$elf" ]] || { echo "coro_frames: ELF file required" >&2; exit 2; }

"$objdump_bin" -d -C "$elf" | perl -e '
	my $filter = shift @ARGV;
	my (%word, @fn);
	my ($name, @body);
	my $flush = sub {
		return unless defined $name;
		push @fn, [$name, [@body]];
	};
	while (<STDIN>) {
		if (/^[0-9a-f]+ <(.*)>:$/) {
			$flush->();
			($name, @body) = ($1);
			next;
		}
		if (/^\s*([0-9a-f]+):\s+(?:[0-9a-f]{2,4} )+\s*\.word\s+(0x[0-9a-f]+)/) {
			$word{hex($1)} = hex($2);
		}
		push @body, $_ if defined $name;
	}
	$flush->();
	my @rows;
	for my $f (@fn) {
		my ($n, $b) = @$f;
		next if $n =~ /^operator new|^isix::co::detail::frame|std::_|^_?_?cxa/ || $n !~ /$filter/;
		my $size;
		for my $i (0 .. $#$b) {
			next unless $b->[$i] =~ /\bbl\s+[0-9a-f]+ <(?:operator new|isix::co::detail::frame_acquire)\(/;
			for (my $j = $i - 1; $j >= 0 && $j >= $i - 4; --$j) {
				my $l = $b->[$j];
				if ($l =~ /\b(?:movs|mov\.w|movw|mov)\s+r0,\s*#(\d+)/) { $size = $1; last }
				if ($l =~ /\bldr\s+r0,\s*\[pc,.*\(([0-9a-f]+)/ && exists $word{hex($1)}) {
					$size = $word{hex($1)}; last
				}
				last if $l =~ /\br0\b/;
			}
			last;
		}
		push @rows, [$size, $n] if defined $size;
	}
	printf "%6s  %s\n", "bytes", "function";
	for my $r (sort { $a->[0] <=> $b->[0] || $a->[1] cmp $b->[1] } @rows) {
		my $s = length($r->[1]) > 110 ? substr($r->[1], 0, 107) . "..." : $r->[1];
		printf "%6d  %s\n", $r->[0], $s;
	}
' "$filter"
