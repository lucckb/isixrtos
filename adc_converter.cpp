/* ------------------------------------------------------------------ */
/*
 * adc_converter.cpp
 *
 *  Created on: 08-03-2011
 *      Author: lucck
 */
/* ------------------------------------------------------------------ */
#include "adc_converter.hpp"
#include <isix.h>
#include <stm32dma.h>
/* ------------------------------------------------------------------ */
namespace stm32 {
namespace dev {

/* ------------------------------------------------------------------ */
namespace
{
	//GPIO port assignement
	GPIO_TypeDef* const adc_ports[] =
	{
		GPIOA, GPIOA, GPIOA, GPIOA, GPIOA, GPIOA, GPIOA, GPIOA,
		GPIOB, GPIOB, GPIOC, GPIOC, GPIOC, GPIOC, GPIOC, GPIOC,
		0, 0
	};
	const char adc_pins[] =
	{
		0, 1, 2, 3, 4, 5, 6, 7,
		0, 1, 0, 1, 2, 3, 4, 5,
		0, 0
	};
	/* SQR1 register Mask */
	const unsigned SQR1_CLEAR_Mask =  0xFF0FFFFF;
	/* CR2 register Mask */
	const unsigned CR2_CLEAR_Mask =   0xFFF1F7FD;
	/* CR1 register Mask */
	const unsigned CR1_CLEAR_Mask =  0xFFF0FEFF;
	/* SQR Channel mask */
	const unsigned SQR_CH_Mask = 0x1f;
	/* ADC ADON mask */
	const unsigned CR2_ADON_Set =  0x00000001;
	const unsigned CR2_ADON_Reset = 0xFFFFFFFE;
	/* ADC RSTCAL mask */
	const unsigned CR2_RSTCAL_Set = 0x00000008;
	/* ADC CAL mask */
	const unsigned CR2_CAL_Set =  0x00000004;
	/* ADC Software start mask */
	const unsigned CR2_EXTTRIG_SWSTART_Set =   0x00500000;
	const unsigned CR2_EXTTRIG_SWSTART_Reset = 0xFFAFFFFF;
	const unsigned CR1_ModeScan = 1<<8;
	const unsigned CR1_EOCInt = 1<<5;
	const unsigned CR2_TempEn = 1<<23;
	const unsigned CR2_SwStart = 1<<22;	//Start conv
	const unsigned CR2_EXTTRIG_Sw = 7<<17;
    const unsigned SR_EOC = 1<<1;
    const unsigned adc_conversion_timeout = 1000;
    //Clear flashs
    const unsigned SR_CLR_Flags = 0x7;
    //DMA enable CR2
    const unsigned CR2_DMAEn = 1<<8;
    //const unsigned CR2_Cont = 1<<1;
	inline unsigned SQR1_L( unsigned num )
	{
		return ((num-1) & 0x0F) << 20;
	}
    //Global object for interrupt handling
    adc_converter * adc1_object;
}
/* ------------------------------------------------------------------ */
adc_converter::adc_converter(ADC_TypeDef * const _ADC, unsigned _ch_mask,
		sample_time sh_time, int irq_prio, int irq_sub)
	: ADC(_ADC),lock_sem(1, 1), complete_sem(0,1), num_active_chns(0)
{
	if(ADC==ADC1)   RCC->APB2ENR |= RCC_APB2Periph_ADC1;
	if( ADC==ADC1 )
	{
		for(int ch=0; ch<adc_channels; ch++)
		{
			if( _ch_mask & (1 << ch))
			{
				if( adc_ports[ch] )
					stm32::io_config( adc_ports[ch],adc_pins[ch],stm32::GPIO_MODE_INPUT, stm32::GPIO_CNF_IN_ANALOG);
				num_active_chns++;
			}
		}
	}
	//Initialize ADC conversion for normal continous scan mode after
	//After conversion values will be avaiable at the channel
	// Clear DUALMOD and SCAN bits
	ADC->CR1 &= CR1_CLEAR_Mask;
	ADC->CR1 |= CR1_ModeScan;
	//CR2 Configuration settings
	ADC->CR2 &= CR2_CLEAR_Mask;
	ADC->CR2 |= CR2_TempEn | CR2_EXTTRIG_Sw | CR2_DMAEn;
	//SQR1 register
	ADC->SQR1 = SQR1_L(num_active_chns);
	ADC->SQR2 = 0; ADC->SQR3 = 0;
	//Setup continous chns
	for(int ch=0, act=0; ch<adc_channels; ch++)
	{
		if( _ch_mask & (1 << ch) )
		{
			if(act<6) ADC->SQR3 |= ch << (5*act);
			else if(act>=6 && act<12)  ADC->SQR2 |= ch << (5*(act-6));
			else if(act>=12 && act<16) ADC->SQR1 |= ch << (5*(act-12));
			act++;
		}
	}
    //Setup the sample timings for all channels
    //TODO: Add separate config for each channel separately
    ADC->SMPR2 = 0; ADC->SMPR1 = 0;
    for(int ch=0; ch<adc_channels; ch++)
    {
        if(ch<10) ADC->SMPR2 |= sh_time << (ch*3);
        else ADC->SMPR1 |= sh_time << ((ch-10)*3);
    }
    //Enable the ADC converter
    ADC->CR2 |= CR2_ADON_Set;
    //Wait for initialization
    ADC->CR2 |= CR2_RSTCAL_Set;
    for(int i=0;i<100;i++)
    {
        if((ADC->CR2 & CR2_RSTCAL_Set) == 0) break;
        isix::isix_wait_ms(10);
    }
    ADC->CR2 |= CR2_CAL_Set;
    //Calibrate
    for(int i=0;i<100;i++)
    {
        if((ADC->CR2 & CR2_CAL_Set) == 0) break;
        isix::isix_wait_ms(10);
    }
    if( ADC==ADC1 ) adc1_object = this;
    //Enable adc interrupt in nvic
    if( ADC==ADC1 )
    {
    	stm32::nvic_set_priority( DMA1_Channel1_IRQn, irq_prio, irq_sub );
    	stm32::nvic_irq_enable( DMA1_Channel1_IRQn, true );
    }
    stm32::dma_enable(stm32::DMACNTR_1);

}
/* ------------------------------------------------------------------ */
//Destructor
adc_converter::~adc_converter()
{
	//Disable interrupts
	stm32::nvic_irq_enable( DMA1_Channel1_IRQn, false );
	//Disable the ADC converter
	 ADC->CR2 &= ~CR2_ADON_Set;
	//Shutdown the converter
    if(ADC==ADC1)   RCC->APB2ENR &= ~RCC_APB2Periph_ADC1;
}
/* ------------------------------------------------------------------ */
void adc_converter::start_conv()
{
    ADC->CR2 |= CR2_EXTTRIG_SWSTART_Set;
}
/* ------------------------------------------------------------------ */

int adc_converter::get_adc_values(volatile unsigned short *regs)
{
	int res = lock_sem.wait ( isix::ISIX_TIME_INFINITE );
    if(res < 0) return res;
    stm32::dma_channel_enable(DMA1_Channel1,
        DMA_DIR_PeripheralSRC | DMA_PeripheralInc_Disable |
        DMA_MemoryInc_Enable | DMA_PeripheralDataSize_HalfWord |
        DMA_MemoryDataSize_HalfWord | DMA_Mode_Normal | DMA_Priority_High | DMA_M2M_Disable,
        regs, &ADC->DR, num_active_chns);
    stm32::dma_channel_enable(DMA1_Channel1);
    stm32::dma_irq_enable(DMA1_Channel1,DMA_IT_TC);
    start_conv();
    res = complete_sem.wait( adc_conversion_timeout );
    stm32::dma_channel_disable(DMA1_Channel1);
    stm32::dma_irq_disable(DMA1_Channel1,DMA_IT_TC);
    if(res<0)
    {
    	lock_sem.signal();
    	return res;
    }
    return lock_sem.signal();
}
/* ------------------------------------------------------------------ */
//Interrupt service routine
void adc_converter::isr()
{
   complete_sem.signal_isr();
}

/* ------------------------------------------------------------------ */
//External interrupt triggering
extern "C"
{
    void dma1_channel1_isr_vector(void) __attribute__((interrupt));
    void dma1_channel1_isr_vector(void)
    {
    	if(adc1_object)
            adc1_object->isr();
    	stm32::dma_clear_flag( DMA1_FLAG_TC1 );
    }
}

/* ------------------------------------------------------------------ */
}}

/* ------------------------------------------------------------------ */
