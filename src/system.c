/* STM32 tiny system library (c) Lucjan Bryndza
 * Lucjan Bryndza (c) 2009
 */


#include "stm32f10x_lib.h"
#include "system.h"

/*----------------------------------------------------------*/
static uint32_t calc_priority(uint32_t preempt_prio, uint32_t sub_prio)
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

//! Setup priority group in NVIC controler
#define AIRCR_VECTKEY_MASK    ((u32)0x05FA0000)

/**  Setup NVIC priority group
 * @param[in] group Priority group to set
 */
void nvic_priority_group(uint32_t group)
{
	/* Set the PRIGROUP[10:8] bits according to NVIC_PriorityGroup value */
	  SCB->AIRCR = AIRCR_VECTKEY_MASK | group;

}
/*----------------------------------------------------------*/
/** Setup NVIC priority in CORTEX-M3 core
 * @param[in] irq_n Setup selected channel
 * @param[in] priority Assigned IRQ preemtion priority
 * @param[in] subpriority  Assigned supbriority
 */
void nvic_set_priority(IRQn_Type irq_num,uint32_t priority,uint32_t subpriority)
{
	uint32_t prio = calc_priority(priority,subpriority);
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
/** Irq mask interrupt priority in CORTEX-M3 core
 * @param[in] priority Assigned IRQ preemtion priority
 * @param[in] subpriority Assigned supbriority
 */
void irq_mask(uint32_t priority,uint32_t subpriority)
{

	/* Compute the Corresponding IRQ Priority */
	uint32_t tmppriority = calc_priority(priority,subpriority);

	asm volatile
	(
			"msr BASEPRI,%0\n"
			::"r"(tmppriority)
	);

}

/*----------------------------------------------------------*/
/** Enable or disable selected interrupt in CORTEX-M3 NVIC core
 * @param[in] channel IRQ channel number
 * @param[in] enable Enable or disable selected channel
 */
void nvic_irq_enable(IRQn_Type irq_num, bool enable)
{
	if(enable)
	{
		/* Enable the Selected IRQ Channels */
		NVIC->ISER[(uint32_t)irq_num >> 0x05] = (u32)0x01 << ((int32_t)irq_num & (u8)0x1F);
	}
	else
	{
		/* Disable the Selected IRQ Channels */
		NVIC->ICER[(uint32_t)irq_num >> 0x05] = (u32)0x01 << ((uint32_t)irq_num & (u8)0x1F);
	}
}

/*----------------------------------------------------------*/
/** Clear pending IRQ interrupt in CORTEX-M3 core
 * @param[in] channel IRQ channel number
 */
void nvic_irq_pend_clear(IRQn_Type irq_num)
{
	//Clear pending bit
	NVIC->ICPR[((uint32_t)irq_num >> 0x05)] = (u32)0x01 << ((uint32_t)irq_num & (u32)0x1F);
}

/*----------------------------------------------------------*/
/** Watchdog KR register bit mask */
#define KR_KEY_Reload    ((u16)0xAAAA)
#define KR_KEY_Enable    ((u16)0xCCCC)
#define IWDG_WriteAccess_Enable     ((uint16_t)0x5555)
#define IWDG_WriteAccess_Disable    ((uint16_t)0x0000)


/** Configure watchdog with timeout
 * @param[in] prescaler Prescaler value
 * @param[in] reload  Reload timeout value
 */
void iwdt_setup(uint8_t prescaler,uint16_t reload)
{
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
