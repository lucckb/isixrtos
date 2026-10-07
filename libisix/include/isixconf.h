#pragma once

#include <config/conf.h>

/* Optional: #define CONFIG_ISIX_CPP_COROUTINES 1 before including <isix.h> (or pass -D) to
 * include C++20 coroutine helpers (isix/cpp/coroutine.hpp). */

/** Ticks per seconds */
#ifndef CONFIG_ISIX_HZ
#define CONFIG_ISIX_HZ 1000
#endif

/* Maximum interrupt priority used with the ISIX context */
#ifndef ISIX_MAX_SYSCALL_INTERRUPT_PRIORITY
#define ISIX_MAX_SYSCALL_INTERRUPT_PRIORITY 0x10
#endif

/** Isix numer of config priorities */
#ifndef CONFIG_ISIX_NUMBER_OF_PRIORITIES
#define CONFIG_ISIX_NUMBER_OF_PRIORITIES 16
#endif

/** Enable disable global debug */
#ifndef CONFIG_ISIX_LOGLEVEL
#define CONFIG_ISIX_LOGLEVEL ISIXLOG_OFF
#endif
/** Enable or disable loglevel for modules separately */

//#define CONFIG_ISIX_LOGLEVEL_SCHEDULER
//#define CONFIG_ISIX_LOGLEVEL_FIFO
//#define CONFIG_ISIX_LOGLEVEL_TASK
//#define CONFIG_ISIX_LOGLEVEL_FIFO
//#define CONFIG_ISIX_LOGLEVEL_VTIMERS
//#define CONFIG_ISIX_LOGLEVEL_MEMORY
//#define CONFIG_ISIX_LOGLEVEL_SEMAPHORE
//#define CONFIG_ISIX_LOGLEVEL_EVENTS
//
//
/** Define memory protection layout */
//#define CONFIG_ISIX_MEMORY_PROTECTION_MODEL
// ISIX_MPROT_NONE ISIX_MPROT_LITE ISIX_MPROT_FULL
#ifndef CONFIG_ISIX_MEMORY_PROTECTION_MODEL
#define CONFIG_ISIX_MEMORY_PROTECTION_MODEL ISIX_MPROT_NONE
#endif

//! CPU load API
//#define CONFIG_ISIX_CPU_USAGE_API 1

/** Tickless idle: disable periodic SysTick when idle (set via meson tickless or tests) */
#ifndef CONFIG_ISIX_TICKLESS
#define CONFIG_ISIX_TICKLESS 0
#endif
#ifndef CONFIG_ISIX_TICKLESS_MIN_SLEEP_TICKS
#define CONFIG_ISIX_TICKLESS_MIN_SLEEP_TICKS 2
#endif
#if CONFIG_ISIX_TICKLESS && CONFIG_ISIX_TICKLESS_MIN_SLEEP_TICKS < 2
#error "CONFIG_ISIX_TICKLESS_MIN_SLEEP_TICKS must be at least 2"
#endif
//! The long timer period is shortened by this number of ticks, the end of the sleep is exact so 0 is enough
#ifndef CONFIG_ISIX_TICKLESS_TIMER_COMPENSATION_TICKS
#define CONFIG_ISIX_TICKLESS_TIMER_COMPENSATION_TICKS 0
#endif
