#ifndef SYSTEM_H_
#define SYSTEM_H_

/*----------------------------------------------------------*/

#include <stdint.h>
#include <stdbool.h>
#include "stm32f10x_lib.h"


/*----------------------------------------------------------*/
/**  Setup NVIC priority group
 * @param[in] group Priority group to set
 */
void nvic_priority_group(uint32_t group);

/*----------------------------------------------------------*/
/** Setup NVIC priority in CORTEX-M3 core
 * @param[in] channel Setup selected channel
 * @param[in] priority Assigned IRQ preemtion priority
 * @param[in] subpriority  Assigned supbriority
 */
void nvic_set_priority(IRQn_Type irq_num,uint32_t priority,uint32_t subpriority);

/*----------------------------------------------------------*/
/** Enable or disable selected interrupt in CORTEX-M3 NVIC core
 * @param[in] channel IRQ channel number
 * @param[in] enable Enable or disable selected channel
 */
void nvic_irq_enable(IRQn_Type irq_num, bool enable);


/*----------------------------------------------------------*/
/** Irq mask interrupt priority in CORTEX-M3 core
 * @param[in] priority Assigned IRQ preemtion priority
 * @param[in] subpriority Assigned supbriority
 */
void irq_mask(uint32_t priority,uint32_t subpriority);

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
/** Clear pending IRQ interrupt in CORTEX-M3 core
 * @param[in] channel IRQ channel number
 */
void nvic_irq_pend_clear(IRQn_Type irq_num);

/*----------------------------------------------------------*/
//! Memory access in bit band region

#define RAM_BASE       0x20000000
#define RAM_BB_BASE    0x22000000


#define  resetBit_BB(VarAddr, BitNumber)    \
          (*(vu32 *) (RAM_BB_BASE | ((((u32)VarAddr) - RAM_BASE) << 5) | ((BitNumber) << 2)) = 0)

#define  setBit_BB(VarAddr, BitNumber)       \
          (*(vu32 *) (RAM_BB_BASE | ((((u32)VarAddr) - RAM_BASE) << 5) | ((BitNumber) << 2)) = 1)

#define getBit_BB(VarAddr, BitNumber)       \
          (*(vu32 *) (RAM_BB_BASE | ((((u32)VarAddr) - RAM_BASE) << 5) | ((BitNumber) << 2)))

/*----------------------------------------------------------*/
//! Sleep mode wait for interrupt macros
#ifndef PDEBUG
#define wfi() asm volatile("wfi")
#else
#define wfi()
#endif

/*----------------------------------------------------------*/
//! NOP command definition
#define nop() asm volatile("nop")

/*----------------------------------------------------------*/
/** Configure watchdog with timeout
 * @param[in] prescaler Prescaler value
 * @param[in] reload  Reload timeout value
 */
void iwdt_setup(uint8_t prescaler,uint16_t reload);

/*----------------------------------------------------------*/

/** KR register bit mask */
#define KR_KEY_Reload    ((u16)0xAAAA)

//! Reset Watchdog MACRO
#define iwdt_reset() IWDG->KR = KR_KEY_Reload

/*----------------------------------------------------------*/
/** Try write atomic into specified location
 * This function should be called from interrupt context
 * @param[out] addr Atomic location to write
 * @param[in] val Value to write into selected addr
 */
static inline long atomic_try_writeb(volatile uint8_t *addr,uint8_t val)
{
	long lock;
	asm volatile
	(
		"ldrexb %0,[%1]\n"
		"strexb %0,%2,[%1]\n"
		: "=&r"(lock)
		: "r"(addr),"r"(val)
		: "cc"
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
static inline uint8_t atomic_xchg_byte(volatile uint8_t *addr,uint8_t val)
{
	uint8_t ret;
	unsigned long tmp;
	asm volatile
	(
	"1:	ldrexb %0,[%2]\n"
	   "strexb %1,%3,[%2]\n"
	   "teq %1,#0\n"
	   "bne 1b\n"
		: "=&r"(ret),"=&r"(tmp)
		: "r"(addr),"r"(val)
		: "cc"
	);
	return ret;
}

/*----------------------------------------------------------*/
/** GPIO bits macros */

//! GPIO mode input
#define GPIO_MODE_INPUT 0
//! GPIO mode out 10MHZ
#define GPIO_MODE_10MHZ 1
//! GPIO mode out 2MHZ
#define GPIO_MODE_2MHZ  2
//! GPIO mode out 50MHZ
#define GPIO_MODE_50MHZ 3

//! GPIO mode PUSH-PULL
#define GPIO_CNF_GPIO_PP 0
//! GPIO mode Open Drain
#define GPIO_CNF_GPIO_OD 1
//! Alternate function PUSH-PULL
#define GPIO_CNF_ALT_PP  2
//! Alternate function Open Drain
#define GPIO_CNF_ALT_OD  3
//! Configuration IN analog mode
#define GPIO_CNF_IN_ANALOG 0
//! Configuration IN float mode
#define GPIO_CNF_IN_FLOAT 1
//! Configuration IN pullup mode
#define GPIO_CNF_IN_PULLUP 2

//! Set GPIO bit macro
#define io_set(PORT,BIT) (PORT)->BSRR = 1<<(BIT)

//! Clear GPIO bit macro
#define io_clr(PORT,BIT) (PORT)->BRR = 1<<(BIT)

//! Get GPIO bit macro
#define io_get(PORT,BIT) (((PORT)->IDR & (1<<(BIT)))?1:0)

//! Enable apb2 perhiperal macro
#define io_apb2set(M)  RCC->APB2ENR |= (M)

/*----------------------------------------------------------*/
/**
 * 	Configure GPIO line into selected mode
 *	@param[in] port Port to configure
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
#define FASTRUN __attribute__ ((long_call, section (".ram_func")))

/*----------------------------------------------------------*/
#endif /*SYSTEM_H_*/
