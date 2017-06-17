#pragma once

#include <config/conf.h>

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
