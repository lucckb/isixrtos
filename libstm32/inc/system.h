#ifndef SYSTEM_H_
#define SYSTEM_H_

/*----------------------------------------------------------*/

#include <stdint.h>
#include <stdbool.h>
#include "stm32f10x_lib.h"
/*----------------------------------------------------------*/
#ifdef __cplusplus
namespace stm32 {
#endif
/*----------------------------------------------------------*/
//Internal namespace
#ifdef __cplusplus
namespace _internal {
namespace system {
#endif
/*----------------------------------------------------------*/
//! Setup priority group in NVIC controler
static const uint32_t AIRCR_VECTKEY_MASK  = 0x05FA0000;
//! Reset the system
static const uint32_t AICR_SYSRESET_REQ = 4;

/*----------------------------------------------------------*/
static inline uint32_t _system_calc_priority(uint32_t preempt_prio, uint32_t sub_prio )
{
    uint32_t tmpsub = 0x0F;
    uint32_t tmppriority = (0x700 - ((SCB->AIRCR) & (uint32_t)0x700))>> 0x08;
    uint32_t tmppre = (0x4 - tmppriority);
    tmpsub = tmpsub >> tmppriority;

    tmppriority =  preempt_prio << tmppre;
    tmppriority |=  sub_prio & tmpsub;
    tmppriority = tmppriority << 0x04;
    return tmppriority;
}

/*----------------------------------------------------------*/
#ifdef __cplusplus
}}	//Internal namespace
#endif

/*----------------------------------------------------------*/
/**  Setup NVIC priority group
 * @param[in] group Priority group to set
 */
static inline void nvic_priority_group(uint32_t group )
{
#ifdef __cplusplus
	using namespace _internal::system;
#endif
	/* Set the PRIGROUP[10:8] bits according to NVIC_PriorityGroup value */
	SCB->AIRCR = AIRCR_VECTKEY_MASK | group;
}

/*----------------------------------------------------------*/
/** Setup NVIC priority in CORTEX-M3 core
 * @param[in] irq_n Setup selected channel
 * @param[in] priority Assigned IRQ preemtion priority
 * @param[in] subpriority  Assigned supbriority
 */
static inline void nvic_set_priority(IRQn_Type irq_num,uint32_t priority,uint32_t subpriority )
{
#ifdef __cplusplus
	using namespace _internal::system;
#endif
	uint32_t prio =  _system_calc_priority(priority,subpriority);
	if(irq_num>=0)
	{
	    NVIC->IP[(uint32_t)irq_num] = prio;
	}
	else
	{
	    SCB->SHP[((uint32_t)(irq_num) & 0xF)-4] = prio;
	}
}

/*----------------------------------------------------------*/
/** Enable or disable selected interrupt in CORTEX-M3 NVIC core
 * @param[in] channel IRQ channel number
 * @param[in] enable Enable or disable selected channel
 */
static inline void nvic_irq_enable(IRQn_Type irq_num, bool enable )
{
	if(enable)
	{
		/* Enable the Selected IRQ Channels */
		NVIC->ISER[(uint32_t)irq_num >> 0x05] = (u32)0x01 << ((uint32_t)irq_num & (u8)0x1F);
	}
	else
	{
		/* Disable the Selected IRQ Channels */
		NVIC->ICER[(uint32_t)irq_num >> 0x05] = (u32)0x01 << ((uint32_t)irq_num & (u8)0x1F);
	}
}

/*----------------------------------------------------------*/
/** Irq mask interrupt priority in CORTEX-M3 core
 * @param[in] priority Assigned IRQ preemtion priority
 * @param[in] subpriority Assigned supbriority
 */
static inline void irq_mask(uint32_t priority,uint32_t subpriority)
{
#ifdef __cplusplus
	using namespace _internal::system;
#endif
	/* Compute the Corresponding IRQ Priority */
	uint32_t tmppriority =  _system_calc_priority(priority,subpriority);

	asm volatile
	(
			"msr BASEPRI,%0\n"
			::"r"(tmppriority)
	);

}

/*----------------------------------------------------------*/
//! Disable IRQ masking in CORTEX-M3 core
static inline void irq_umask(void)
{
	asm volatile
	(
		"msr BASEPRI,%0\n"
		::"r"(0):"cc"
	);
}
/*----------------------------------------------------------*/
//!Disable IRQ in the core
static inline void irq_disable(void)
{
	asm volatile("cpsid i\t\n");
}
/*----------------------------------------------------------*/
//!Enable IRQ in the core
static inline void irq_enable(void)
{
	asm volatile("cpsie i\t\n");
}

/*----------------------------------------------------------*/

/** Clear pending IRQ interrupt in CORTEX-M3 core
 * @param[in] channel IRQ channel number
 */
static inline void nvic_irq_pend_clear(IRQn_Type irq_num)
{
	//Clear pending bit
	NVIC->ICPR[((uint32_t)irq_num >> 0x05)] = (u32)0x01 << ((uint32_t)irq_num & (u32)0x1F);
}


/*----------------------------------------------------------*/
//! Memory access in bit band region
enum {
	RAM_BASE =      0x20000000,
	RAM_BB_BASE =    0x22000000
};

static inline void resetBit_BB(void *VarAddr, unsigned BitNumber)
{
     (*(vu32 *) (RAM_BB_BASE | ((((u32)VarAddr) - RAM_BASE) << 5) | ((BitNumber) << 2)) = 0);
}

static inline void setBit_BB(void *VarAddr, unsigned BitNumber)
{
    (*(vu32 *) (RAM_BB_BASE | ((((u32)VarAddr) - RAM_BASE) << 5) | ((BitNumber) << 2)) = 1);
}

static inline bool getBit_BB(void *VarAddr, unsigned BitNumber)
{
     return (*(vu32 *) (RAM_BB_BASE | ((((u32)VarAddr) - RAM_BASE) << 5) | ((BitNumber) << 2)));
}

/*----------------------------------------------------------*/
//! Sleep mode wait for interrupt macros
#ifndef PDEBUG
static inline void wfi( void ) { asm volatile("wfi"); }
#else
static inline void wfi( void ) { asm volatile("nop");  }
#endif

/*----------------------------------------------------------*/
//! NOP command definition
static inline void nop(void) { asm volatile("nop"); }

/*----------------------------------------------------------*/

/** KR register bit mask */
enum {
	KR_KEY_Reload  = 0xAAAA,
	KR_KEY_Enable  =  0xCCCC
};

/*----------------------------------------------------------*/
/** Configure watchdog with timeout
 * @param[in] prescaler Prescaler value
 * @param[in] reload  Reload timeout value
 */
static inline void iwdt_setup(uint8_t prescaler,uint16_t reload)
{
#ifdef __cplusplus
	using namespace _internal::system;
#endif
	//Enable write access to wdt
	IWDG->KR = IWDG_WriteAccess_Enable;
	//Program prescaler
	IWDG->PR = prescaler;
	//Set reload value
	IWDG->RLR = reload;
	//Reload register
	IWDG->KR = KR_KEY_Reload;
	//Watchdog enable
	IWDG->KR = KR_KEY_Enable;
}

/*----------------------------------------------------------*/
//! Reset Watchdog MACRO
static inline void iwdt_reset(void)
{
    IWDG->KR = KR_KEY_Reload;
}

/*----------------------------------------------------------*/
/** Try write atomic into specified location
 * This function should be called from interrupt context
 * @param[out] addr Atomic location to write
 * @param[in] val Value to write into selected addr
 * @return 1 if unable to access memory
 */
static inline long atomic_try_write_byte( volatile uint8_t *addr, uint8_t val )
{
	long lock;
	asm volatile
	(
		"ldrexb %[lock],[%[addr]]\n"
		"strexb %[lock],%[val],[%[addr]]\n"
		: [lock] "=&r"(lock)
		: [addr] "r"(addr), [val] "r"(val)
		: "cc", "memory"
	);
	return lock;
}

/*----------------------------------------------------------*/
/** Atomic exchange byte
 * Write and read to selected address. Function
 * should be called from main context
 * @param[out] addr Address with location to write
 * @param[in] val Value to write
 * @return Value read from specified location
 */
static inline uint8_t atomic_xchg_byte( volatile uint8_t *addr, uint8_t val )
{
	uint8_t ret;
	unsigned long lock;
	asm volatile
	(
	"1:	ldrexb %[ret],[%[addr]]\n"
	   "strexb %[lock],%[val],[%[addr]]\n"
	   "teq %[lock],#0\n"
	   "bne 1b\n"
		: [ret]"=&r"(ret), [lock]"=&r"(lock)
		: [addr]"r"(addr), [val]"r"(val)
		: "cc", "memory"
	);
	return ret;
}
/*----------------------------------------------------------*/
/** Atomic read byte
 * Write and read to selected address. Function
 * should be called from main context
 * @param[out] addr Address with location to write
 * @return Value read from specified location
 */
static inline uint8_t atomic_read_byte( const volatile uint8_t *addr )
{
	uint8_t ret = 0;
	unsigned long lock;
	asm volatile
	(
	"1:	ldrexb %[ret],[%[addr]]\n"
	   "strexb %[lock],%[ret],[%[addr]]\n"
	   "teq %[lock],#0\n"
	   "bne 1b\n"
		: [ret]"+&r"(ret), [lock]"=&r"(lock)
		: [addr]"r"(addr)
		: "cc", "memory"
	);
	return ret;
}
/*----------------------------------------------------------*/
/** Try write atomic into specified location
 * This function should be called from interrupt context
 * @param[out] addr Atomic location to write
 * @param[in] val Value to write into selected addr
 * @return 1 if unable to access memory
 */
static inline long atomic_try_write_word( volatile uint32_t *addr, uint32_t val )
{
	long lock;
	asm volatile
	(
		"ldrex %[lock],[%[addr]]\n"
		"strex %[lock],%[val],[%[addr]]\n"
		: [lock] "=&r"(lock)
		: [addr] "r"(addr), [val] "r"(val)
		: "cc", "memory"
	);
	return lock;
}

/*----------------------------------------------------------*/
/** Atomic exchange byte
 * Write and read to selected address. Function
 * should be called from main context
 * @param[out] addr Address with location to write
 * @param[in] val Value to write
 * @return Value read from specified location
 */
static inline uint32_t atomic_xchg_word( volatile uint32_t *addr, uint32_t val )
{
	uint32_t ret;
	unsigned long lock;
	asm volatile
	(
	"1:	ldrex %[ret],[%[addr]]\n"
	   "strex %[lock],%[val],[%[addr]]\n"
	   "teq %[lock],#0\n"
	   "bne 1b\n"
		: [ret]"=&r"(ret), [lock]"=&r"(lock)
		: [addr]"r"(addr), [val]"r"(val)
		: "cc", "memory"
	);
	return ret;
}
/*----------------------------------------------------------*/
/** Atomic read byte
 * Write and read to selected address. Function
 * should be called from main context
 * @param[out] addr Address with location to write
 * @return Value read from specified location
 */
static inline uint32_t atomic_read_word( const volatile uint32_t *addr )
{
	uint32_t ret = 0;
	unsigned long lock;
	asm volatile
	(
	"1:	ldrex %[ret],[%[addr]]\n"
	   "strex %[lock],%[ret],[%[addr]]\n"
	   "teq %[lock],#0\n"
	   "bne 1b\n"
		: [ret]"+&r"(ret), [lock]"=&r"(lock)
		: [addr]"r"(addr)
		: "cc", "memory"
	);
	return ret;
}
/*----------------------------------------------------------*/
/** GPIO bits macros */
enum
{
	//! GPIO mode input
	GPIO_MODE_INPUT = 0,
	//! GPIO mode out 10MHZ
	GPIO_MODE_10MHZ = 1,
	//! GPIO mode out 2MHZ
	GPIO_MODE_2MHZ = 2,
	//! GPIO mode out 50MHZ
	GPIO_MODE_50MHZ = 3
};

enum
{
	//! GPIO mode PUSH-PULL
	GPIO_CNF_GPIO_PP = 0,
	//! GPIO mode Open Drain
	GPIO_CNF_GPIO_OD = 1,
	//! Alternate function PUSH-PULL
	GPIO_CNF_ALT_PP = 2,
	//! Alternate function Open Drain
	GPIO_CNF_ALT_OD = 3
};

enum
{
	//! Configuration IN analog mode
	GPIO_CNF_IN_ANALOG = 0,
	//! Configuration IN float mode
	GPIO_CNF_IN_FLOAT = 1,
	//! Configuration IN pullup mode
	GPIO_CNF_IN_PULLUP = 2
};
/*----------------------------------------------------------*/
//! Set GPIO bit macro
static inline void io_set(GPIO_TypeDef* port , unsigned bit)
{
	port->BSRR = 1<<bit;
}
/*----------------------------------------------------------*/
//! Clear GPIO bit macro
static inline void io_clr(GPIO_TypeDef* port , unsigned bit)
{
	port->BRR = 1<<bit;
}
/*----------------------------------------------------------*/
//! Set by the mask
static inline void io_set_mask(GPIO_TypeDef* port , uint16_t bitmask)
{
	port->BSRR = bitmask;
}
/*----------------------------------------------------------*/
//! Clear GPIO bit mask
static inline void io_clr_mask(GPIO_TypeDef* port , uint16_t bitmask)
{
	port->BRR = bitmask;
}
/*----------------------------------------------------------*/
//! set clr in one op
static inline void io_set_clr_mask(GPIO_TypeDef* port , uint16_t enflags, uint16_t mask)
{
	port->BSRR = (uint32_t)(enflags & mask) | ((uint32_t)( ~enflags & mask)<<16);
}
/*----------------------------------------------------------*/
//! Get GPIO bit macro
//#define io_get(PORT,BIT) (((PORT)->IDR & (1<<(BIT)))?1:0)
static inline bool io_get(GPIO_TypeDef* port , unsigned bit)
{
	return (port->IDR >> (bit))&1;
}
/*----------------------------------------------------------*/
//! Get GPIO bit mask
static inline uint16_t io_get_mask(GPIO_TypeDef* port , uint16_t bitmask)
{
	return port->IDR & bitmask;
}
/*----------------------------------------------------------*/
/**
 * 	Configure GPIO line into selected mode
 *	@param[in] port Port to configure
 *	@param[in] bit  Pin bit number
 *	@param[in] mode New port mode
 *	@param[in] config New port configuration
 */
static inline void io_config(GPIO_TypeDef* port,uint8_t bit,uint32_t mode,uint32_t config)
{
	if(bit>7)
	{
		port->CRH &= ~(0xf<<(4*(bit-8)));
		port->CRH |= (mode|(config<<2)) << (4*(bit-8));
	}
	else
	{
		port->CRL &= ~(0xf<<(4*bit));
		port->CRL |= (mode|(config<<2)) << (4*bit);
	}
}
/*----------------------------------------------------------*/
//Reset the MCU system
static inline void nvic_system_reset(void)
{
#ifdef __cplusplus
	using namespace _internal::system;
#endif
	SCB->AIRCR = AIRCR_VECTKEY_MASK | AICR_SYSRESET_REQ;
	asm volatile("dsb\n");
	for(;;);	//Wait for system reset
}

/*----------------------------------------------------------*/
/**
 * 	Configure GPIO lines into selected mode
 *	@param[in] port Port to configure
 *	@param[in] bitmask Pin bitmap to configure
 *	@param[in] mode New port mode
 *	@param[in] config New port configuration
 */
static inline void io_config_ext(GPIO_TypeDef* port, uint16_t bit, uint32_t mode, uint32_t config)
{
	for(unsigned i=0; i<16; i++)
	{
		if(bit & (1<<i))
		{
			io_config(port,i,mode,config);
		}
	}
}
/*----------------------------------------------------------*/
/** Put function in the RAM section */
#define STM32_FASTRUN __attribute__ ((long_call, section (".ram_func")))

/*----------------------------------------------------------*/
/** Return FLASH size in the CPU
 */
static inline int flash_mem_size(void)
{
	return (*(vu16 *)(0x1FFFF7E0));
}
/*----------------------------------------------------------*/
/** Return CPUID
 * @param[in] word pos number
 * @return value
 */
static inline unsigned get_cpuid(unsigned pos)
{
	if(pos<3)
		return ((vu32 *)( 0x1FFFF7E8))[pos];
	else
		return 0;
}
/*----------------------------------------------------------*/
/**
 * @brief  Initialize and start the SysTick counter and its interrupt.
 *
 * @param   ticks   number of ticks between two interrupts
 * @return  1 = failed, 0 = successful
 *
 * Initialise the system tick timer and its interrupt and start the
 * system tick timer / counter in free running mode to generate
 * periodical interrupts.
 */

static inline int systick_config(uint32_t ticks)
{
  if (ticks > SysTick_LOAD_RELOAD_Msk)  return (-1);            /* Reload value impossible */

  SysTick->LOAD  = (ticks & SysTick_LOAD_RELOAD_Msk) - 1;      /* set reload register */
  SysTick->VAL   = 0;                                          /* Load the SysTick Counter Value */
  SysTick->CTRL  = /*SysTick_CTRL_CLKSOURCE_Msk | */
                   SysTick_CTRL_TICKINT_Msk   |
                   SysTick_CTRL_ENABLE_Msk;                    /* Enable SysTick IRQ and SysTick Timer */
  return (0);                                                  /* Function successful */
}

/*----------------------------------------------------------*/
/**
 * @brief Enable or disable selected perhipheral
 * @param clock_type Clock type depends on the bus
 * @param periph  Perhipherals bitmask
 * @param enable Enable or disable
 */
enum rcc_clock_type
{
	rcc_clk_ahb,
	rcc_clk_apb2,
	rcc_clk_apb1
};

static inline void periph_clock_cmd( enum rcc_clock_type clock_type, uint32_t periph, bool enable)
{
	switch( clock_type )
	{
	case rcc_clk_ahb:
		if( enable ) RCC->AHBENR |= periph;
		else RCC->AHBENR &= ~periph;
		break;
	case rcc_clk_apb2:
		if( enable ) RCC->APB2ENR |= periph;
		else RCC->APB2ENR &= ~periph;
		break;
	case rcc_clk_apb1:
		if( enable ) RCC->APB1ENR |= periph;
		else RCC->APB1ENR &= ~periph;
		break;
	}
}
/*----------------------------------------------------------*/
#ifdef __cplusplus
}
#endif

/*----------------------------------------------------------*/
#endif /*SYSTEM_H_*/

