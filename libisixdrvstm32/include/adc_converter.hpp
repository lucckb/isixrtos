 
/*
 * adc_converter.hpp
 *
 *  Created on: 08-03-2011
 *      Author: lucck
 */

#ifndef ADC_CONVERTER_HPP_
#define ADC_CONVERTER_HPP_
 
#include <stm32system.h>
#include <stm32lib.h>
#include <isix.h>
 
namespace stm32 {
namespace dev {
 
extern "C"
{
    void dma1_channel1_isr_vector() __attribute__((interrupt));
    void dma2_stream0_isr_vector() __attribute__((interrupt));
}
 
class adc_converter {
    friend void dma1_channel1_isr_vector();
    friend void dma2_stream0_isr_vector();
public:
	//ADC channel table
	enum ch
	{
		CH_0 = 1, CH_1 = 1<<1, CH_2 = 1<<2, CH_3 = 1<<3,
		CH_4 = 1<<4, CH_5 = 1<<5, CH_6 = 1<<6, CH_7 = 1<<7,
		CH_8 = 1<<8, CH_9= 1<<9, CH_10 = 1<<10, CH_11 = 1<<11,
		CH_12 = 1<<12, CH_13 = 1<<13, CH_14 = 1<<14, CH_15 = 1<<15,
		CH_TEMP = 1<<16, CH_VREFINT = 1<<17
	};
    enum sample_time
    {
        sample1C5, sample7C5, sample13C5, sample28C5, sample41C5, sample55C5, sample71C5, sample239C5
    };
	//Default constructor
	adc_converter(ADC_TypeDef * const _ADC, unsigned _ch_mask, sample_time sh_time=sample71C5,
			int irq_prio=1, int irq_sub=7);
	//Default destructor
	~adc_converter();
	//Start the ADC conversion on selected chns sleep current process for conv
	int get_adc_values(volatile unsigned short *regs);
private:
	static const int adc_channels = 18;
    void start_conv();
    void isr();
private:
	ADC_TypeDef * const m_ADC;
    isix::semaphore lock_sem;
    isix::semaphore complete_sem;
    short num_active_chns;
private: //nonocopyable
	adc_converter(adc_converter &);
	adc_converter& operator=(const adc_converter&);
};

 

}
}
 
#endif /* ADC_CONVERTER_HPP_ */
 
