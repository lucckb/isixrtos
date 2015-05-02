#pragma once
#ifdef _HAVE_CONFIG_H
#include "config.h"
#endif


/** Ticks per seconds */
#ifndef ISIX_CONFIG_HZ
#define ISIX_CONFIG_HZ 1000
#endif

/* Maximum interrupt priority used with the ISIX context */
#ifndef ISIX_MAX_SYSCALL_INTERRUPT_PRIORITY
#define ISIX_MAX_SYSCALL_INTERRUPT_PRIORITY 0x10
#endif

/** Configure alignment mask */
#ifndef ISIX_CONFIG_BYTE_ALIGNMENT_SIZE
#define ISIX_CONFIG_BYTE_ALIGNMENT_SIZE 8
#endif

/** Define isix config log level */


/** Enable disable global debug */
#ifndef ISIX_CONFIG_LOGLEVEL
#define ISIX_CONFIG_LOGLEVEL ISIXLOG_OFF
#endif
/** Enable or disable loglevel for modules separately */

//#define ISIX_LOGLEVEL_SCHEDULER
//#define ISIX_LOGLEVEL_FIFO
//#define ISIX_LOGLEVEL_TASK
//#define ISIX_LOGLEVEL_FIFO
//#define ISIX_LOGLEVEL_VTIMERS
//#define ISIX_LOGLEVEL_MEMORY
//#define ISIX_LOGLEVEL_SEMAPHORE
//#define ISIX_LOGLEVEL_EVENTS


