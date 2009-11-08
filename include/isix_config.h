

/* Stack from high to low address */
#define CONFIG_STACK_GROWTH 1

/* Use preemtion */
#define CONFIG_USE_PREEMPTION 1

/* Ticks per sec */
#define CONFIG_HZ 1000

/* Enable debug scheduler */
#define ISIX_DEBUG_SCHEDULER ISIX_DBG_OFF
#define ISIX_DEBUG_FIFO ISIX_DBG_OFF
#define ISIX_DEBUG_MEMORY ISIX_DBG_OFF
#define ISIX_DEBUG_SEMAPHORE ISIX_DBG_OFF
#define ISIX_DEBUG_TASK ISIX_DBG_OFF

#define configMAX_SYSCALL_INTERRUPT_PRIORITY 0x80

