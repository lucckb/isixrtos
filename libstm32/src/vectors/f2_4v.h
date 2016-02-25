/*
 * =====================================================================================
 *
 *       Filename:  f2_4v.c
 *
 *    Description:  Vectors for stm32F2 and F4
 *
 *        Version:  1.0
 *        Created:  24.02.2016 23:19:12
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
ISR_VECTOR(debug_monitor_isr_vector); 	
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
ISR_VECTOR(dma1_stream0_isr_vector);	
ISR_VECTOR(dma1_stream1_isr_vector);	
ISR_VECTOR(dma1_stream2_isr_vector);	
ISR_VECTOR(dma1_stream3_isr_vector);	
ISR_VECTOR(dma1_stream4_isr_vector);	
ISR_VECTOR(dma1_stream5_isr_vector);	
ISR_VECTOR(dma1_stream6_isr_vector);	
ISR_VECTOR(adc_isr_vector);	
ISR_VECTOR(can1_tx_isr_vector);		
ISR_VECTOR(can1_rx0_isr_vector);  	
ISR_VECTOR(can1_rx1_isr_vector);	
ISR_VECTOR(can1_sce_isr_vector);	
ISR_VECTOR_FORCED(exti9_5_isr_vector);	
ISR_VECTOR(tim1_brk_tim9_isr_vector);	
ISR_VECTOR(tim1_up_tim10_isr_vector);	
ISR_VECTOR(tim1_trg_com_tim11_isr_vector);	
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
ISR_VECTOR(tim8_brk_tim12_isr_vector);						
ISR_VECTOR(tim8_up_tim13_isr_vector);						
ISR_VECTOR(tim8_trg_com_tim14_isr_vector);				
ISR_VECTOR(tim8_cc_isr_vector);						
ISR_VECTOR(dma1_stream7_isr_vector);					
ISR_VECTOR(fsmc_isr_vector);							
ISR_VECTOR(sdio_isr_vector);							
ISR_VECTOR(tim5_isr_vector);							
ISR_VECTOR(spi3_isr_vector);							
ISR_VECTOR(usart4_isr_vector);							
ISR_VECTOR(usart5_isr_vector);							
ISR_VECTOR(tim6_dac_isr_vector);							
ISR_VECTOR(tim7_isr_vector);							
ISR_VECTOR(dma2_stream0_isr_vector);					
ISR_VECTOR(dma2_stream1_isr_vector);					
ISR_VECTOR(dma2_stream2_isr_vector);					
ISR_VECTOR(dma2_stream3_isr_vector); 					
ISR_VECTOR(dma2_stream4_isr_vector);					
ISR_VECTOR(eth_isr_vector);							
ISR_VECTOR(eth_wkup_isr_vector);						
ISR_VECTOR(can2_tx_isr_vector);						
ISR_VECTOR(can2_rx0_isr_vector);						
ISR_VECTOR(can2_rx1_isr_vector);						
ISR_VECTOR(can2_sce_isr_vector);						
ISR_VECTOR(otg_fs_isr_vector);						
ISR_VECTOR(dma2_stream5_isr_vector);					
ISR_VECTOR(dma2_stream6_isr_vector);					
ISR_VECTOR(dma2_stream7_isr_vector);					
ISR_VECTOR(usart6_isr_vector);						
ISR_VECTOR(i2c3_ev_isr_vector);						
ISR_VECTOR(i2c3_er_isr_vector);						
ISR_VECTOR(otg_hs_ep1_out_isr_vector);		    	
ISR_VECTOR(otg_hs_wkup_isr_vector);					
ISR_VECTOR(otg_hs_isr_vector);						
ISR_VECTOR(dcmi_isr_vector);							
ISR_VECTOR(cryp_isr_vector);							
ISR_VECTOR(hash_rng_isr_vector);						
ISR_VECTOR(fpu_isr_vector);							
	
__attribute__ ((section(".isr_vector")))
const vect_fun_t const exceptions_vectors[] =
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
  debug_monitor_isr_vector, 	
  0,                      
  pend_svc_isr_vector, 		
  systick_isr_vector,	
  wwdg_isr_vector,	
  pvd_isr_vector,	
  tamper_isr_vector,	
  rtc_isr_vector,	
  flash_isr_vector,	
  rcc_isr_vector,	
  exti0_isr_vector,	
  exti1_isr_vector,	
  exti2_isr_vector,	
  exti3_isr_vector,	
  exti4_isr_vector,	
  dma1_stream0_isr_vector,	
  dma1_stream1_isr_vector,	
  dma1_stream2_isr_vector,	
  dma1_stream3_isr_vector,	
  dma1_stream4_isr_vector,	
  dma1_stream5_isr_vector,	
  dma1_stream6_isr_vector,	
  adc_isr_vector,	
  can1_tx_isr_vector,		
  can1_rx0_isr_vector,  	
  can1_rx1_isr_vector,	
  can1_sce_isr_vector,	
  exti9_5_isr_vector,	
  tim1_brk_tim9_isr_vector,	
  tim1_up_tim10_isr_vector,	
  tim1_trg_com_tim11_isr_vector,	
  tim1_cc_isr_vector,	
  tim2_isr_vector,	
  tim3_isr_vector,	
  tim4_isr_vector,	
  i2c1_ev_isr_vector,	
  i2c1_er_isr_vector,	
  i2c2_ev_isr_vector,	
  i2c2_er_isr_vector,	
  spi1_isr_vector,	
  spi2_isr_vector,	
  usart1_isr_vector,	
  usart2_isr_vector,	
  usart3_isr_vector,	
  exti15_10_isr_vector,	
  rtc_alarm_isr_vector,	
  usb_wakeup_isr_vector,	
  tim8_brk_tim12_isr_vector,						
  tim8_up_tim13_isr_vector,						
  tim8_trg_com_tim14_isr_vector,				
  tim8_cc_isr_vector,						
  dma1_stream7_isr_vector,					
  fsmc_isr_vector,							
  sdio_isr_vector,							
  tim5_isr_vector,							
  spi3_isr_vector,							
  usart4_isr_vector,							
  usart5_isr_vector,							
  tim6_dac_isr_vector,							
  tim7_isr_vector,							
  dma2_stream0_isr_vector,					
  dma2_stream1_isr_vector,					
  dma2_stream2_isr_vector,					
  dma2_stream3_isr_vector, 					
  dma2_stream4_isr_vector,					
  eth_isr_vector,							
  eth_wkup_isr_vector,						
  can2_tx_isr_vector,						
  can2_rx0_isr_vector,						
  can2_rx1_isr_vector,						
  can2_sce_isr_vector,						
  otg_fs_isr_vector,						
  dma2_stream5_isr_vector,					
  dma2_stream6_isr_vector,					
  dma2_stream7_isr_vector,					
  usart6_isr_vector,						
  i2c3_ev_isr_vector,						
  i2c3_er_isr_vector,						
  otg_hs_ep1_out_isr_vector,		    	
  otg_hs_ep1_out_isr_vector,				
  otg_hs_wkup_isr_vector,					
  otg_hs_isr_vector,						
  dcmi_isr_vector,							
  cryp_isr_vector,							
  hash_rng_isr_vector,						
  fpu_isr_vector,							
};
