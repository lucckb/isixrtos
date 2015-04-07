#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <isix/error.h>
#include <isix/config.h>

#ifndef __cplusplus
#define ISIX_TIME_INFINITE 0
#define ISIX_SEM_ULIMITED 0
#else
namespace {
	constexpr unsigned ISIX_TIME_INFINITE = 0U;
	constexpr unsigned ISIX_SEM_ULIMITED = 0U;
}
#endif

#ifdef __cplusplus
static constexpr unsigned ISIX_HZ = ISIX_CONFIG_HZ;
#else
//! HZ value used as system ticks
#define ISIX_HZ ISIX_CONFIG_HZ
#endif

//! Define tick type
typedef unsigned int ostick_t;
//! Define a micro tick
typedef unsigned long osutick_t;
//! Priority type
typedef uint8_t osprio_t;
// Natural bitset type
typedef uint32_t osbitset_t;
