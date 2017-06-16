/*
 * =====================================================================================
 *
 *       Filename:  f3xx.h
 *
 *    Description:  STM32f3 ISR vectors 
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
ISR_VECTOR(exti2_isr_vector);
ISR_VECTOR(exti3_isr_vector);
ISR_VECTOR(exti4_isr_vector);
ISR_VECTOR(dma1_channel1_isr_vector);
ISR_VECTOR(dma1_channel2_isr_vector);
ISR_VECTOR(dma1_channel3_isr_vector);
ISR_VECTOR(dma1_channel4_isr_vector);
ISR_VECTOR(dma1_channel5_isr_vector);
ISR_VECTOR(dma1_channel6_isr_vector);
ISR_VECTOR(dma1_channel7_isr_vector);
ISR_VECTOR(adc_isr_vector);
ISR_VECTOR(usb_hp_can_tx_isr_vector);
ISR_VECTOR(usb_lp_can_rx0_isr_vector);
ISR_VECTOR(can_rx1_isr_vector);
ISR_VECTOR(can_sce_isr_vector);
ISR_VECTOR_FORCED(exti9_5_isr_vector);
ISR_VECTOR(tim1_brk_isr_vector);
ISR_VECTOR(tim1_up_isr_vector);
ISR_VECTOR(tim1_trg_com_isr_vector);
ISR_VECTOR(tim1_cc_isr_vector);
ISR_VECTOR(tim2_isr_vector);
ISR_VECTOR(tim3_isr_vector);
ISR_VECTOR(tim4_isr_vector);
ISR_VECTOR(i2c1_ev_isr_vector);
ISR_VECTOR(i2c1_er_isr_vector);
ISR_VECTOR(i2c2_ev_isr_vector);
ISR_VECTOR(i2c2_er_isr_vector);
ISR_VECTOR(spi1_isr_vector);
ISR_VECTOR(spi2_isr_vector);
ISR_VECTOR(usart1_isr_vector);
ISR_VECTOR(usart2_isr_vector);
ISR_VECTOR(usart3_isr_vector);
ISR_VECTOR_FORCED(exti15_10_isr_vector);
ISR_VECTOR(rtc_alarm_isr_vector);
ISR_VECTOR(usb_wakeup_isr_vector);
ISR_VECTOR(tim8_brk_isr_vector);
ISR_VECTOR(tim8_up_isr_vector);
ISR_VECTOR(tim8_trg_com_isr_vector);
ISR_VECTOR(tim8_cc_isr_vector);
ISR_VECTOR(adc3_isr_vector);
ISR_VECTOR(fsmc_isr_vector);
ISR_VECTOR(spi3_isr_vector);
ISR_VECTOR(usart4_isr_vector);
ISR_VECTOR(usart5_isr_vector);
ISR_VECTOR(tim6_isr_vector);
ISR_VECTOR(tim7_isr_vector);
ISR_VECTOR(dma2_channel1_isr_vector);
ISR_VECTOR(dma2_channel2_isr_vector);
ISR_VECTOR(dma2_channel3_isr_vector);
ISR_VECTOR(dma2_channel4_isr_vector);
ISR_VECTOR(dma2_channel5_isr_vector);
ISR_VECTOR(adc4_isr_vector);
ISR_VECTOR(comp1_2_3_isr_vector);
ISR_VECTOR(comp4_5_6_isr_vector);
ISR_VECTOR(comp7_isr_vector);
ISR_VECTOR(i2c3_ev_isr_vector);
ISR_VECTOR(i2c3_er_isr_vector);
ISR_VECTOR(usb_hp_isr_vector);
ISR_VECTOR(usb_lp_isr_vector);
ISR_VECTOR(usb_wakeup_rmp_isr_vector);
ISR_VECTOR(tim20_brk_isr_vector);
ISR_VECTOR(tim20_up_isr_vector);
ISR_VECTOR(tim20_trg_com_isr_vector);
ISR_VECTOR(tim20_cc_isr_vector);
ISR_VECTOR(fpu_isr_vector);
ISR_VECTOR(spi4_isr_vector);

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
    exti2_isr_vector,
    exti3_isr_vector,
    exti4_isr_vector,						//10
    dma1_channel1_isr_vector,
    dma1_channel2_isr_vector,
    dma1_channel3_isr_vector,
    dma1_channel4_isr_vector,
    dma1_channel5_isr_vector,
    dma1_channel6_isr_vector,
    dma1_channel7_isr_vector,
    adc_isr_vector,
    usb_hp_can_tx_isr_vector,
    usb_lp_can_rx0_isr_vector, 				//20
    can_rx1_isr_vector,
    can_sce_isr_vector,
    exti9_5_isr_vector,
    tim1_brk_isr_vector,
    tim1_up_isr_vector,
    tim1_trg_com_isr_vector,
    tim1_cc_isr_vector,
    tim2_isr_vector,
    tim3_isr_vector,
    tim4_isr_vector,						//30
    i2c1_ev_isr_vector,
    i2c1_er_isr_vector,
    i2c2_ev_isr_vector,
    i2c2_er_isr_vector,
    spi1_isr_vector,
    spi2_isr_vector,
    usart1_isr_vector,
    usart2_isr_vector,
    usart3_isr_vector,
    exti15_10_isr_vector,					//40
    rtc_alarm_isr_vector,
    usb_wakeup_isr_vector,
    tim8_brk_isr_vector,
    tim8_up_isr_vector,
    tim8_trg_com_isr_vector,
    tim8_cc_isr_vector,
    adc3_isr_vector,
    fsmc_isr_vector,
    0,
    0,										//50
    spi3_isr_vector,
    usart4_isr_vector,
    usart5_isr_vector,
    tim6_isr_vector,
    tim7_isr_vector,
    dma2_channel1_isr_vector,
    dma2_channel2_isr_vector,
    dma2_channel3_isr_vector,
    dma2_channel4_isr_vector,
    dma2_channel5_isr_vector,				//60
    adc4_isr_vector,
	0,
	0,
    comp1_2_3_isr_vector,
    comp4_5_6_isr_vector,
    comp7_isr_vector,
	0,
	0,
	0,
	0,										//70
	0,
    i2c3_ev_isr_vector,
    i2c3_er_isr_vector,
	usb_hp_isr_vector,
	usb_lp_isr_vector,
	usb_wakeup_rmp_isr_vector,
	tim20_brk_isr_vector,
	tim20_up_isr_vector,
	tim20_trg_com_isr_vector,
	tim20_cc_isr_vector,					//80
	fpu_isr_vector,
	0,
	0,
	spi4_isr_vector
  };

