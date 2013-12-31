#ifdef _HAVE_CONFIG_H
#include "config.h"
#endif

/** Stack from high to low address */
#ifndef ISIX_CONFIG_STACK_GROWTH
#define ISIX_CONFIG_STACK_GROWTH 1
#endif
/** Use preemption */
#ifndef ISIX_CONFIG_USE_PREEMPTION
#define ISIX_CONFIG_USE_PREEMPTION 1
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

/** Enable disable global debug */
#define ISIX_DEBUG_ENABLE ISIX_DBG_OFF
/** Configure the thread debugging */
#define ISIX_CONFIG_TASK_STACK_CHECK ISIX_ON
/** Enable debugging options */
#define ISIX_DEBUG_SCHEDULER ISIX_DBG_OFF
#define ISIX_DEBUG_FIFO ISIX_DBG_OFF
#define ISIX_DEBUG_MEMORY ISIX_DBG_OFF
#define ISIX_DEBUG_SEMAPHORE ISIX_DBG_OFF
#define ISIX_DEBUG_TASK ISIX_DBG_OFF
#define ISIX_DEBUG_MULTIOBJECTS ISIX_DBG_OFF
