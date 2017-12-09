/*
 * =====================================================================================
 *
 *       Filename:  f3xx.h
 *
 *    Description:  STM32f33x ISR vectors
 *
 *        Version:  1.0
 *        Created:  29.02.2016 20:56:22
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Lucjan Bryndza (LB), lucck(at)boff(dot)pl
 *   Organization:  BoFF
 *
 * =====================================================================================
 */

ISR_VECTOR(nmi_exception_vector);
ISR_VECTOR(hard_fault_exception_vector);
ISR_VECTOR(mem_manage_exception_vector);
ISR_VECTOR(bus_fault_exception_vector);
ISR_VECTOR(usage_fault_exception_vector);
ISR_VECTOR(svc_isr_vector);
ISR_VECTOR(pend_svc_isr_vector);
ISR_VECTOR(systick_isr_vector);
ISR_VECTOR(wwdg_isr_vector);


ISR_VECTOR(pvd_isr_vector);
ISR_VECTOR(tamper_isr_vector);
ISR_VECTOR(rtc_isr_vector);
ISR_VECTOR(flash_isr_vector);
ISR_VECTOR(rcc_isr_vector);
ISR_VECTOR(exti0_isr_vector);
ISR_VECTOR(exti1_isr_vector);
ISR_VECTOR(exti2_ts_isr_vector);
ISR_VECTOR(exti3_isr_vector);
ISR_VECTOR(exti4_isr_vector);
ISR_VECTOR(dma1_channel1_isr_vector);
ISR_VECTOR(dma1_channel2_isr_vector);
ISR_VECTOR(dma1_channel3_isr_vector);
ISR_VECTOR(dma1_channel4_isr_vector);
ISR_VECTOR(dma1_channel5_isr_vector);
ISR_VECTOR(dma1_channel6_isr_vector);
ISR_VECTOR(dma1_channel7_isr_vector);
ISR_VECTOR(adc12_isr_vector);
ISR_VECTOR(can_tx_isr_vector);
ISR_VECTOR(can_rx0_isr_vector);
ISR_VECTOR(can_rx1_isr_vector);
ISR_VECTOR(can_sce_isr_vector);
ISR_VECTOR_FORCED(exti9_5_isr_vector);
ISR_VECTOR(tim1_brk_tim15_isr_vector);
ISR_VECTOR(tim1_up_tim16_isr_vector);
ISR_VECTOR(tim1_trg_com_tim17_isr_vector);
ISR_VECTOR(tim1_cc_isr_vector);
ISR_VECTOR(tim2_isr_vector);
ISR_VECTOR(tim3_isr_vector);
ISR_VECTOR(i2c1_ev_isr_vector);
ISR_VECTOR(i2c1_er_isr_vector);
ISR_VECTOR(spi1_isr_vector);
ISR_VECTOR(usart1_isr_vector);
ISR_VECTOR(usart2_isr_vector);
ISR_VECTOR(usart3_isr_vector);
ISR_VECTOR_FORCED(exti15_10_isr_vector);
ISR_VECTOR(rtc_alarm_isr_vector);
ISR_VECTOR(tim6_dac1_isr_vector);
ISR_VECTOR(tim7_dac2_isr_vector);
ISR_VECTOR(comp2_isr_vector);
ISR_VECTOR(comp4_6_isr_vector);
ISR_VECTOR(hrtim_master_isr_vector);
ISR_VECTOR(hrtim_tim_a_isr_vector);
ISR_VECTOR(hrtim_tim_b_isr_vector);
ISR_VECTOR(hrtim_tim_c_isr_vevtor);
ISR_VECTOR(hrtim_tim_d_isr_vector);
ISR_VECTOR(hrtim_tim_e_isr_vector);
ISR_VECTOR(hrtim_tim_flt_isr_vector);
ISR_VECTOR(fpu_isr_vector);



  __attribute__ ((section(".isr_vector")))
  const vect_fun_t _exceptions_vectors[] =
  {
    (vect_fun_t)(long)&_estack,
    _mcu_reset_handler_,
    nmi_exception_vector,
    hard_fault_exception_vector,
    mem_manage_exception_vector,
    bus_fault_exception_vector,
    usage_fault_exception_vector,
    0, 0, 0, 0,
    svc_isr_vector,
    0, 0,
    pend_svc_isr_vector,
    systick_isr_vector,
    wwdg_isr_vector,						//0
    pvd_isr_vector,
    tamper_isr_vector,
    rtc_isr_vector,
    flash_isr_vector,
    rcc_isr_vector,
    exti0_isr_vector,
    exti1_isr_vector,
    exti2_ts_isr_vector,
    exti3_isr_vector,
    exti4_isr_vector,						//10
    dma1_channel1_isr_vector,
    dma1_channel2_isr_vector,
    dma1_channel3_isr_vector,
    dma1_channel4_isr_vector,
    dma1_channel5_isr_vector,
    dma1_channel6_isr_vector,
    dma1_channel7_isr_vector,
    adc12_isr_vector,
    can_tx_isr_vector,
    can_rx0_isr_vector,						//20
    can_rx1_isr_vector,
    can_sce_isr_vector,
    exti9_5_isr_vector,
    tim1_brk_tim15_isr_vector,
    tim1_up_tim16_isr_vector,
    tim1_trg_com_tim17_isr_vector,
    tim1_cc_isr_vector,
    tim2_isr_vector,
    tim3_isr_vector,
    0,										//30
    i2c1_ev_isr_vector,
    i2c1_er_isr_vector,
    0,
    0,
    spi1_isr_vector,
    0,
    usart1_isr_vector,
    usart2_isr_vector,
    usart3_isr_vector,
    exti15_10_isr_vector,					//40
    rtc_alarm_isr_vector,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,										//50
    0,
    0,
    0,
    tim6_dac1_isr_vector,
    tim7_dac2_isr_vector,
    0,
    0,
    0,
    0,
    0,										//60
    0,
	0,
	0,
    comp2_isr_vector,
    comp4_6_isr_vector,
    0,
	hrtim_master_isr_vector,
	hrtim_tim_a_isr_vector,
	hrtim_tim_b_isr_vector,
	hrtim_tim_c_isr_vevtor,				   //70
	hrtim_tim_d_isr_vector,
    hrtim_tim_e_isr_vector,
    hrtim_tim_flt_isr_vector,
	0,
	0,
	0,
	0,
	0,
	0,
	0,								//80
	fpu_isr_vector
  };

