/*
 * Copyright (c) 2025 Lucjan Bryndza
 *
 * Helpers for the system timer ports with a free running up counter and a compare register
 * (RISC-V mtime, a 16, 24, 32 or 64 bit hardware timer). They keep the arithmetic of the
 * counter wrap and of the cycles that do not divide into ticks in one place.
 */
#pragma once

#include <isix/types.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Counter cycles elapsed since the reference value, correct also after the counter wrapped
 * @param[in] now Current counter value
 * @param[in] last Counter value at the reference point
 * @param[in] mask Counter range, all bits of the counter width set
 * @return Elapsed cycles, valid when less than one counter range elapsed
 */
static inline uint64_t _isixp_upcounter_elapsed( uint64_t now, uint64_t last, uint64_t mask )
{
	return (now - last) & mask;
}

/** Convert the elapsed cycles into whole ticks, the cycles of an incomplete tick are not lost
 * @param[in] elapsed Elapsed cycles, below 2^64 / hz
 * @param[in] freq Counter frequency in Hz
 * @param[in] hz Tick frequency in Hz
 * @param[in,out] carry Remainder of the previous conversions, it is below freq
 * @return Number of whole ticks
 */
static inline ostick_t _isixp_upcounter_ticks( uint64_t elapsed, uint32_t freq, uint32_t hz, uint64_t* carry )
{
	const uint64_t total = elapsed * hz + *carry;
	*carry = total % freq;
	return (ostick_t)(total / freq);
}

/** Longest timeout in ticks that can be programmed so that the counter does not wrap unnoticed
 * Half of the counter range is the margin for the interrupt latency.
 * @param[in] mask Counter range, all bits of the counter width set
 * @param[in] freq Counter frequency in Hz
 * @param[in] hz Tick frequency in Hz
 * @return Number of ticks, at most half of the jiffies range
 */
static inline ostick_t _isixp_upcounter_max_ticks( uint64_t mask, uint32_t freq, uint32_t hz )
{
	uint64_t half = mask / 2U;
	// The elapsed time times the tick frequency must fit in 64 bits
	if( half > UINT64_MAX / hz ) {
		half = UINT64_MAX / hz;
	}
	const uint64_t ticks = (half / freq) * hz + ((half % freq) * hz) / freq;
	const uint64_t limit = (uint64_t)UINT32_MAX / 2U;
	return (ostick_t)(ticks > limit ? limit : ticks);
}

#ifdef __cplusplus
}
#endif
