#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <isix/error.h>
#include <isix/config.h>
#include <sys/types.h>

#ifndef __cplusplus
#define ISIX_TIME_INFINITE 0
#define ISIX_SEM_ULIMITED 0
#define ISIX_TIME_DONTWAIT 0xFFFFFFFFU
#define ISIX_TIME_MAX_TICK 0xFFFFFFFFU
#else
namespace {
	constexpr unsigned ISIX_TIME_INFINITE = 0U;
	constexpr unsigned ISIX_TIME_DONTWAIT = 0xFFFFFFFFU;
	constexpr unsigned ISIX_SEM_ULIMITED = 0U;
}
#endif

#ifdef __cplusplus
static constexpr unsigned ISIX_HZ = CONFIG_ISIX_HZ;
#else
//! HZ value used as system ticks
#define ISIX_HZ CONFIG_ISIX_HZ
#endif


//! Reference type
typedef uint8_t osprio_t;
#ifdef __cplusplus
static constexpr auto OSREF_T_MAX = UINT16_MAX;
#else
#define OSREF_T_MAX UINT16_MAX
#endif


//! Define tick type
typedef unsigned int ostick_t;
//! Define a micro tick
typedef unsigned long osutick_t;
//! Priority type
typedef uint16_t osref_t;
//! Natural bitset type
typedef uint32_t osbitset_t;
typedef int32_t osbitset_ret_t;
