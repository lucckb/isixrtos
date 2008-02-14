/* Use LPC 2000 sofware */
#define CONFIG_ARM7LPC2000 1

/* Perhiperal CLOCK */
#define CONFIG_PCLK 60000000

/* Main sustrm clock */
#define CONFIG_MCLK 60000000

/* Stack from high to low address */
#define CONFIG_STACK_GROWTH 1

/* Use preemtion */
#define CONFIG_USE_PREEMPTION 1

/* Ticks per sec */
#define CONFIG_HZ 100

/* Enable debug scheduler */
#define DEBUG_SCHEDULER DBG_OFF
#define DEBUG_FIFO DBG_OFF
#define DEBUG_MEMORY DBG_OFF
#define DEBUG_SEMAPHORE DBG_OFF
#define DEBUG_TASK DBG_OFF


