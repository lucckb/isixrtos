

/** Stack from high to low address */
#define ISIX_CONFIG_STACK_GROWTH 1

/** Use preemption */
#define ISIX_CONFIG_USE_PREEMPTION 1

/** Ticks per seconds */
#define ISIX_CONFIG_HZ 1000

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
/** Maximum interrupt priority used with the ISIX context */
#define ISIX_MAX_SYSCALL_INTERRUPT_PRIORITY 0x10

