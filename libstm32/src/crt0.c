#if defined(STM32MCU_MAJOR_TYPE_F4)
#include <stm32f4x/stm32f4xx.h>
#include <core_cm4.h>
#endif

//Cortex startup file based on STM32 code

/*----------------------------------------------------------*/
extern unsigned long _etext;
extern unsigned long _sidata;		/* start address for the initialization values
                                   of the .data section. defined in linker script */
extern unsigned long _sdata;		/* start address for the .data section. defined
                                   in linker script */
extern unsigned long _edata;		/* end address for the .data section. defined in
                                   linker script */

extern unsigned long _sbss;			/* start address for the .bss section. defined
                                   in linker script */
extern unsigned long _ebss;			/* end address for the .bss section. defined in
                                   linker script */
extern unsigned long _estack;	   /* init value for the stack pointer. defined in linker script */


/*----------------------------------------------------------*/

static void reset_handler(void) __attribute__((__interrupt__,noreturn));

extern int main(void);

/*----------------------------------------------------------*/
static void unused_vector(void) __attribute__((__interrupt__,noreturn));
//Unused vector dummy function
static void unused_vector() { while(1); }

/*----------------------------------------------------------*/
typedef void(*vect_fun_t)(void);

/*----------------------------------------------------------*/
#define DEFINE_INTERRUPT_HANDLER( handler_name ) void handler_name(void) __attribute__ ((interrupt, weak, alias("unused_vector")))
#define DEFINE_REAL_INTERRUPT_HANDLER( handler_name )  void handler_name(void) __attribute__ ((interrupt))
/*----------------------------------------------------------*/
#if defined(COMPILED_UNDER_ISIX) && defined(FUNCTION_MAIN_RETURN)
void  __attribute__((weak,alias("empty_func")))_isixp_finalize();
#endif
/*----------------------------------------------------------*/
DEFINE_INTERRUPT_HANDLER(nmi_exception_vector);
DEFINE_INTERRUPT_HANDLER(hard_fault_exception_vector);
DEFINE_INTERRUPT_HANDLER(mem_manage_exception_vector);
DEFINE_INTERRUPT_HANDLER(bus_fault_exception_vector);
DEFINE_INTERRUPT_HANDLER(usage_fault_exception_vector);
#ifndef COMPILED_UNDER_ISIX
DEFINE_INTERRUPT_HANDLER(svc_isr_vector);
#else
DEFINE_INTERRUPT_HANDLER(svc_isr_vector);
#endif
DEFINE_INTERRUPT_HANDLER(debug_monitor_isr_vector);
#ifndef COMPILED_UNDER_ISIX
DEFINE_INTERRUPT_HANDLER(pend_svc_isr_vector);
DEFINE_INTERRUPT_HANDLER(systick_isr_vector);
#else
DEFINE_INTERRUPT_HANDLER(pend_svc_isr_vector);
DEFINE_INTERRUPT_HANDLER(systick_isr_vector);
#endif
DEFINE_INTERRUPT_HANDLER(wwdg_isr_vector);
DEFINE_INTERRUPT_HANDLER(pvd_isr_vector);
DEFINE_INTERRUPT_HANDLER(tamper_isr_vector);
DEFINE_INTERRUPT_HANDLER(rtc_isr_vector);
DEFINE_INTERRUPT_HANDLER(flash_isr_vector);
DEFINE_INTERRUPT_HANDLER(rcc_isr_vector);
DEFINE_INTERRUPT_HANDLER(exti0_isr_vector);
DEFINE_INTERRUPT_HANDLER(exti1_isr_vector);
DEFINE_INTERRUPT_HANDLER(exti2_isr_vector);
DEFINE_INTERRUPT_HANDLER(exti3_isr_vector);
DEFINE_INTERRUPT_HANDLER(exti4_isr_vector);
DEFINE_INTERRUPT_HANDLER(dma1_channel1_isr_vector);
DEFINE_INTERRUPT_HANDLER(dma1_channel2_isr_vector);
DEFINE_INTERRUPT_HANDLER(dma1_channel3_isr_vector);
DEFINE_INTERRUPT_HANDLER(dma1_channel4_isr_vector);
DEFINE_INTERRUPT_HANDLER(dma1_channel5_isr_vector);
DEFINE_INTERRUPT_HANDLER(dma1_channel6_isr_vector);
DEFINE_INTERRUPT_HANDLER(dma1_channel7_isr_vector);
DEFINE_INTERRUPT_HANDLER(adc_isr_vector);
DEFINE_INTERRUPT_HANDLER(usb_hp_can_tx_isr_vector);
DEFINE_INTERRUPT_HANDLER(usb_lp_can_rx0_isr_vector);
DEFINE_INTERRUPT_HANDLER(can_rx1_isr_vector);
DEFINE_INTERRUPT_HANDLER(can_sce_isr_vector);
DEFINE_REAL_INTERRUPT_HANDLER(exti9_5_isr_vector);
DEFINE_INTERRUPT_HANDLER(tim1_brk_isr_vector);
DEFINE_INTERRUPT_HANDLER(tim1_up_isr_vector);
DEFINE_INTERRUPT_HANDLER(tim1_trg_com_isr_vector);
DEFINE_INTERRUPT_HANDLER(tim1_cc_isr_vector);
DEFINE_INTERRUPT_HANDLER(tim2_isr_vector);
DEFINE_INTERRUPT_HANDLER(tim3_isr_vector);
DEFINE_INTERRUPT_HANDLER(tim4_isr_vector);
DEFINE_INTERRUPT_HANDLER(i2c1_ev_isr_vector);
DEFINE_INTERRUPT_HANDLER(i2c1_er_isr_vector);
DEFINE_INTERRUPT_HANDLER(i2c2_ev_isr_vector);
DEFINE_INTERRUPT_HANDLER(i2c2_er_isr_vector);
DEFINE_INTERRUPT_HANDLER(spi1_isr_vector);
DEFINE_INTERRUPT_HANDLER(spi2_isr_vector);
DEFINE_INTERRUPT_HANDLER(usart1_isr_vector);
DEFINE_INTERRUPT_HANDLER(usart2_isr_vector);
DEFINE_INTERRUPT_HANDLER(usart3_isr_vector);
DEFINE_REAL_INTERRUPT_HANDLER(exti15_10_isr_vector);
DEFINE_INTERRUPT_HANDLER(rtc_alarm_isr_vector);
DEFINE_INTERRUPT_HANDLER(usb_wakeup_isr_vector);
DEFINE_INTERRUPT_HANDLER(tim8_brk_isr_vector);
DEFINE_INTERRUPT_HANDLER(tim8_up_isr_vector);
DEFINE_INTERRUPT_HANDLER(tim8_trg_com_isr_vector);
DEFINE_INTERRUPT_HANDLER(tim8_cc_isr_vector);
DEFINE_INTERRUPT_HANDLER(adc3_isr_vector);
DEFINE_INTERRUPT_HANDLER(fsmc_isr_vector);
DEFINE_INTERRUPT_HANDLER(sdio_isr_vector);
DEFINE_INTERRUPT_HANDLER(tim5_isr_vector);
DEFINE_INTERRUPT_HANDLER(spi3_isr_vector);
DEFINE_INTERRUPT_HANDLER(usart4_isr_vector);
DEFINE_INTERRUPT_HANDLER(usart5_isr_vector);
DEFINE_INTERRUPT_HANDLER(tim6_isr_vector);
DEFINE_INTERRUPT_HANDLER(tim7_isr_vector);
DEFINE_INTERRUPT_HANDLER(dma2_channel1_isr_vector);
DEFINE_INTERRUPT_HANDLER(dma2_channel2_isr_vector);
DEFINE_INTERRUPT_HANDLER(dma2_channel3_isr_vector);
DEFINE_INTERRUPT_HANDLER(dma2_channel4_5_isr_vector);
/* Connectivity line only */
DEFINE_INTERRUPT_HANDLER(dma2_channel5_isr_vector);
DEFINE_INTERRUPT_HANDLER(eth_isr_vector);
DEFINE_INTERRUPT_HANDLER(eth_wkup_isr_vector);
DEFINE_INTERRUPT_HANDLER(can2_tx_isr_vector);
DEFINE_INTERRUPT_HANDLER(can2_rx0_isr_vector);
DEFINE_INTERRUPT_HANDLER(can2_rx1_isr_vector);
DEFINE_INTERRUPT_HANDLER(can2_sce_isr_vector);
DEFINE_INTERRUPT_HANDLER(otg_fs_isr_vector);
DEFINE_INTERRUPT_HANDLER(tim1_brk_tim15_isr_vector);
DEFINE_INTERRUPT_HANDLER(tim1_up_tim16_isr_vector);
DEFINE_INTERRUPT_HANDLER(tim1_trg_com_tim17_isr_vector);
DEFINE_INTERRUPT_HANDLER(cec_isr_vector);
DEFINE_INTERRUPT_HANDLER(tim6_dac_isr_vector);
DEFINE_INTERRUPT_HANDLER(tim12_isr_vector);
DEFINE_INTERRUPT_HANDLER(tim13_isr_vector);
DEFINE_INTERRUPT_HANDLER(tim14_isr_vector);
DEFINE_INTERRUPT_HANDLER(can1_tx_isr_vector);
/* STM32F4 interrupts */
DEFINE_INTERRUPT_HANDLER(can1_rx0_isr_vector);
DEFINE_INTERRUPT_HANDLER(can1_rx1_isr_vector);
DEFINE_INTERRUPT_HANDLER(can1_sce_isr_vector);
DEFINE_INTERRUPT_HANDLER(tim1_brk_tim9_isr_vector);
DEFINE_INTERRUPT_HANDLER(tim1_up_tim10_isr_vector);
DEFINE_INTERRUPT_HANDLER(tim1_trg_com_tim11_isr_vector);
DEFINE_INTERRUPT_HANDLER(tim8_brk_tim12_isr_vector);
DEFINE_INTERRUPT_HANDLER(tim8_up_tim13_isr_vector);
DEFINE_INTERRUPT_HANDLER(tim8_trg_com_tim14_isr_vector);
DEFINE_INTERRUPT_HANDLER(dma1_channel8_isr_vector);
DEFINE_INTERRUPT_HANDLER(dma2_channel4_isr_vector);
DEFINE_INTERRUPT_HANDLER(dma2_channel6_isr_vector);
DEFINE_INTERRUPT_HANDLER(dma2_channel7_isr_vector);
DEFINE_INTERRUPT_HANDLER(dma2_channel8_isr_vector);
DEFINE_INTERRUPT_HANDLER(usart6_isr_vector);
DEFINE_INTERRUPT_HANDLER(i2c3_ev_isr_vector);
DEFINE_INTERRUPT_HANDLER(i2c3_er_isr_vector);
DEFINE_INTERRUPT_HANDLER(otg_hs_ep1_out_isr_vector);
DEFINE_INTERRUPT_HANDLER(otg_hs_wkup_isr_vector);
DEFINE_INTERRUPT_HANDLER(otg_hs_isr_vector);
DEFINE_INTERRUPT_HANDLER(dcmi_isr_vector);
DEFINE_INTERRUPT_HANDLER(cryp_isr_vector);
DEFINE_INTERRUPT_HANDLER(hash_rng_isr_vector);
DEFINE_INTERRUPT_HANDLER(fpu_isr_vector);
//F4 version only
DEFINE_INTERRUPT_HANDLER(dma1_stream0_isr_vector);
DEFINE_INTERRUPT_HANDLER(dma1_stream1_isr_vector);
DEFINE_INTERRUPT_HANDLER(dma1_stream2_isr_vector);
DEFINE_INTERRUPT_HANDLER(dma1_stream3_isr_vector);
DEFINE_INTERRUPT_HANDLER(dma1_stream4_isr_vector);
DEFINE_INTERRUPT_HANDLER(dma1_stream5_isr_vector);
DEFINE_INTERRUPT_HANDLER(dma1_stream6_isr_vector);
DEFINE_INTERRUPT_HANDLER(dma1_stream7_isr_vector);
DEFINE_INTERRUPT_HANDLER(dma2_stream0_isr_vector);
DEFINE_INTERRUPT_HANDLER(dma2_stream1_isr_vector);
DEFINE_INTERRUPT_HANDLER(dma2_stream2_isr_vector);
DEFINE_INTERRUPT_HANDLER(dma2_stream3_isr_vector);
DEFINE_INTERRUPT_HANDLER(dma2_stream4_isr_vector);
DEFINE_INTERRUPT_HANDLER(dma2_stream5_isr_vector);
DEFINE_INTERRUPT_HANDLER(dma2_stream6_isr_vector);
DEFINE_INTERRUPT_HANDLER(dma2_stream7_isr_vector);

/***** EMULATED EXTI HANDLERS demuxed in CRT0 ******/
DEFINE_INTERRUPT_HANDLER(exti5_isr_vector);
DEFINE_INTERRUPT_HANDLER(exti6_isr_vector);
DEFINE_INTERRUPT_HANDLER(exti7_isr_vector);
DEFINE_INTERRUPT_HANDLER(exti8_isr_vector);
DEFINE_INTERRUPT_HANDLER(exti9_isr_vector);
DEFINE_INTERRUPT_HANDLER(exti10_isr_vector);
DEFINE_INTERRUPT_HANDLER(exti11_isr_vector);
DEFINE_INTERRUPT_HANDLER(exti12_isr_vector);
DEFINE_INTERRUPT_HANDLER(exti13_isr_vector);
DEFINE_INTERRUPT_HANDLER(exti14_isr_vector);
DEFINE_INTERRUPT_HANDLER(exti15_isr_vector);

#undef DEFINE_INTERRUPT_HANDLER
#undef DEFINE_REAL_INTERRUPT_HANDLER
/*---------------------------------------------------------*/
#if defined(STM32MCU_MAJOR_TYPE_F4) || defined(STM32MCU_MAJOR_TYPE_F2)
//Interrupt vector table
__attribute__ ((section(".isr_vector")))
const vect_fun_t const exceptions_vectors[] =
{
  (vect_fun_t)(long)&_estack,            // The initial stack pointer
  reset_handler,             // The reset handler
  nmi_exception_vector, //NMIException
  hard_fault_exception_vector, //HardFaultException
  mem_manage_exception_vector, //MemManageException
  bus_fault_exception_vector, //BusFaultException
  usage_fault_exception_vector, //UsageFaultException
  0, 0, 0, 0,            /* Reserved */
  svc_isr_vector, //SVCHandler
  debug_monitor_isr_vector, //DebugMonitor
  0,                      /* Reserved */
  pend_svc_isr_vector, 	//PendSVC
  systick_isr_vector,//SysTickHandler,
  wwdg_isr_vector,//,WWDG_IRQHandler,	P*0
  pvd_isr_vector,//PVD_IRQHandler, P*1
  tamper_isr_vector,//TAMPER_IRQHandler, P*2
  rtc_isr_vector,//RTC_IRQHandler,TIM2->CCR1 P*3
  flash_isr_vector,//FLASH_IRQHandler, P*4
  rcc_isr_vector,//RCC_IRQHandler, P*5
  exti0_isr_vector,//EXTI0_IRQHandler, P*6
  exti1_isr_vector,//EXTI1_IRQHandler, P*7
  exti2_isr_vector,//EXTI2_IRQHandler, P*8
  exti3_isr_vector,//EXTI3_IRQHandler, P*9
  exti4_isr_vector,//EXTI4_IRQHandler, P*10
  dma1_stream0_isr_vector,//DMAChannel1_IRQHandler, P*11
  dma1_stream1_isr_vector,//DMAChannel2_IRQHandler, P*12
  dma1_stream2_isr_vector,//DMAChannel3_IRQHandler, P*13
  dma1_stream3_isr_vector,//DMAChannel4_IRQHandler, P*14
  dma1_stream4_isr_vector,//DMAChannel5_IRQHandler, P*15
  dma1_stream5_isr_vector,//DMAChannel6_IRQHandler, P*16
  dma1_stream6_isr_vector,//DMAChannel7_IRQHandler, P*17
  adc_isr_vector,//ADC_IRQHandler, P*18
  can1_tx_isr_vector,	//CAN11 TX,	P*19
  can1_rx0_isr_vector,  //CAN1  RX0, P*20
  can1_rx1_isr_vector,//CAN1_RX1_IRQHandler, P*21
  can1_sce_isr_vector,//CAN1_SCE_IRQHandler, P*22
  exti9_5_isr_vector,//EXTI9_5_IRQHandler, P*23
  tim1_brk_tim9_isr_vector,//TIM1_BRK_IRQHandler, P*24
  tim1_up_tim10_isr_vector,//TIM1_UP_IRQHandler, P*25
  tim1_trg_com_tim11_isr_vector,//TIM1_TRG_COM_IRQHandler, P*26
  tim1_cc_isr_vector,//TIM1_CC_IRQHandler, P*27
  tim2_isr_vector,//TIM2_IRQHandler, P*28
  tim3_isr_vector,//TIM3_IRQHandler, P*29
  tim4_isr_vector,//TIM4_IRQHandler, P*30
  i2c1_ev_isr_vector,//I2C1_EV_IRQHandler, P*31
  i2c1_er_isr_vector,//I2C1_ER_IRQHandler, P*32
  i2c2_ev_isr_vector,//I2C2_EV_IRQHandler, P*33
  i2c2_er_isr_vector,//I2C2_ER_IRQHandler, P*34
  spi1_isr_vector,//SPI1_IRQHandler,	P*35
  spi2_isr_vector,//SPI2_IRQHandler, P*36
  usart1_isr_vector,//USART1_IRQHandler, P*37
  usart2_isr_vector,//USART2_IRQHandler, P*38
  usart3_isr_vector,//unused_vector,//USART3_IRQHandler, P*39
  exti15_10_isr_vector,//EXTI15_10_IRQHandler, P*40
  rtc_alarm_isr_vector,//RTCAlarm_IRQHandler, P*41
  usb_wakeup_isr_vector,//USBWakeUp_IRQHandler, P*42
  tim8_brk_tim12_isr_vector,					// TIM8 break interrupt P*43
  tim8_up_tim13_isr_vector,					// TIM8 update interrupt P*44
  tim8_trg_com_tim14_isr_vector,			// TIM8 trigger and commutation interrupts P*45
  tim8_cc_isr_vector,					// TIM8 capture compare interrupt P*46
  dma1_stream7_isr_vector,				// DMA1 channel8 P*47
  fsmc_isr_vector,						// FSMC global interrupt P*48
  sdio_isr_vector,						// SDIO global interrupt P*49
  tim5_isr_vector,						// TIM5 global interrupt P*50
  spi3_isr_vector,						// SPI3 global interrupt P*51
  usart4_isr_vector,						// USART4 global interrupt P*52
  usart5_isr_vector,						// USART5 global interrupt P*53
  tim6_dac_isr_vector,						// TIM6 global interrupt P*54
  tim7_isr_vector,						// TIM7 global interrupt P*55
  dma2_stream0_isr_vector,				// DMA2 Channel1 global interrupt P*56
  dma2_stream1_isr_vector,				// DMA2 Channel2 global interrupt P*57
  dma2_stream2_isr_vector,				// DMA2 Channel3 global interrupt P*58
  dma2_stream3_isr_vector, 				// DMA2 Channel4 and DMA2 Channel5 global interrupts P*59
  dma2_stream4_isr_vector,				// Beginning from 60 Connectivity line only P*60
  eth_isr_vector,						// Ethernet isr vector P*61
  eth_wkup_isr_vector,					// Ethernet wakeup vector P*62
  can2_tx_isr_vector,					// Can2 tx P*63
  can2_rx0_isr_vector,					// Can2 rx0 P*64
  can2_rx1_isr_vector,					// Can2 rx1 P*65
  can2_sce_isr_vector,					// Can2 sce P*66
  otg_fs_isr_vector,					// Usb on the go isr P*67
  dma2_stream5_isr_vector,				// Dma channel 6 P*68
  dma2_stream6_isr_vector,				// Dma channel 7 P*69
  dma2_stream7_isr_vector,				// Dma channel 8 P*70
  usart6_isr_vector,					// Usart 6 isr vector P*71
  i2c3_ev_isr_vector,					// I2c4 ev isr vector P*72
  i2c3_er_isr_vector,					// I2c4 er isr vector P*73
  otg_hs_ep1_out_isr_vector,		    // USB OTG HS endpoint out P*74
  otg_hs_ep1_out_isr_vector,			// USB OTG HS endpoint in P*75
  otg_hs_wkup_isr_vector,				//USB OTG  HS wakeup P*76
  otg_hs_isr_vector,					//USB OTG  HS global IRQ P*77
  dcmi_isr_vector,						//DCMI global interrupt P*78
  cryp_isr_vector,						//CRYP global interrupt P*79
  hash_rng_isr_vector,					//hash rng interrupt P*80
  fpu_isr_vector,						//FPU ISR P*81
};
#elif defined (STM32F10X_LD_VL) || defined (STM32F10X_MD_VL) || defined (STM32F10X_HD_VL)
__attribute__ ((section(".isr_vector")))
const vect_fun_t const exceptions_vectors[] =
{
  (vect_fun_t)(long)&_estack,            // The initial stack pointer
  reset_handler,             // The reset handler
  nmi_exception_vector, //NMIException
  hard_fault_exception_vector, //HardFaultException
  mem_manage_exception_vector, //MemManageException
  bus_fault_exception_vector, //BusFaultException
  usage_fault_exception_vector, //UsageFaultException
  0, 0, 0, 0,            /* Reserved */
  svc_isr_vector, //SVCHandler
  debug_monitor_isr_vector, //DebugMonitor
  0,                      /* Reserved */
  pend_svc_isr_vector, 	//PendSVC
  systick_isr_vector,//SysTickHandler,
  wwdg_isr_vector,//,WWDG_IRQHandler,	P*0
  pvd_isr_vector,//PVD_IRQHandler, P*1
  tamper_isr_vector,//TAMPER_IRQHandler, P*2
  rtc_isr_vector,//RTC_IRQHandler,TIM2->CCR1 P*3
  flash_isr_vector,//FLASH_IRQHandler, P*4
  rcc_isr_vector,//RCC_IRQHandler, P*5
  exti0_isr_vector,//EXTI0_IRQHandler, P*6
  exti1_isr_vector,//EXTI1_IRQHandler, P*7
  exti2_isr_vector,//EXTI2_IRQHandler, P*8
  exti3_isr_vector,//EXTI3_IRQHandler, P*9
  exti4_isr_vector,//EXTI4_IRQHandler, P*10
  dma1_channel1_isr_vector,//DMAChannel1_IRQHandler, P*11
  dma1_channel2_isr_vector,//DMAChannel2_IRQHandler, P*12
  dma1_channel3_isr_vector,//DMAChannel3_IRQHandler, P*13
  dma1_channel4_isr_vector,//DMAChannel4_IRQHandler, P*14
  dma1_channel5_isr_vector,//DMAChannel5_IRQHandler, P*15
  dma1_channel6_isr_vector,//DMAChannel6_IRQHandler, P*16
  dma1_channel7_isr_vector,//DMAChannel7_IRQHandler, P*17
  adc_isr_vector,//ADC_IRQHandler, P*18
  0,	//Reserved , P*19
  0,	//Reserved , P*20
  0,	//Reserved , P*21
  0,	//Reserved , P*22
  exti9_5_isr_vector,//EXTI9_5_IRQHandler, P*23
  tim1_brk_tim15_isr_vector,//TIM1_BRK_IRQHandler, P*24
  tim1_up_tim16_isr_vector,//TIM1_UP_IRQHandler, P*25
  tim1_trg_com_tim17_isr_vector,//TIM1_TRG_COM_IRQHandler, P*26
  tim1_cc_isr_vector,//TIM1_CC_IRQHandler, P*27
  tim2_isr_vector,//TIM2_IRQHandler, P*28
  tim3_isr_vector,//TIM3_IRQHandler, P*29
  tim4_isr_vector,//TIM4_IRQHandler, P*30
  i2c1_ev_isr_vector,//I2C1_EV_IRQHandler, P*31
  i2c1_er_isr_vector,//I2C1_ER_IRQHandler, P*32
  i2c2_ev_isr_vector,//I2C2_EV_IRQHandler, P*33
  i2c2_er_isr_vector,//I2C2_ER_IRQHandler, P*34
  spi1_isr_vector,//SPI1_IRQHandler,	P*35
  spi2_isr_vector,//SPI2_IRQHandler, P*36
  usart1_isr_vector,//USART1_IRQHandler, P*37
  usart2_isr_vector,//USART2_IRQHandler, P*38
  usart3_isr_vector,//unused_vector,//USART3_IRQHandler, P*39
  exti15_10_isr_vector,//EXTI15_10_IRQHandler, P*40
  rtc_alarm_isr_vector,//RTCAlarm_IRQHandler, P*41
  cec_isr_vector,//CEC, P*42
  tim12_isr_vector,					// TI12 interrupt P*43
  tim13_isr_vector,					// TIM13 interrupt P*44
  tim14_isr_vector,					// TIM14 interrupts P*45
  0,								// Reserved P*46
  0,								// Reserved P*47
  fsmc_isr_vector,					// FSMC global interrupt P*48
  0,								// Reserved P*49
  tim5_isr_vector,						// TIM5 global interrupt P*50
  spi3_isr_vector,						// SPI3 global interrupt P*51
  usart4_isr_vector,						// USART4 global interrupt P*52
  usart5_isr_vector,						// USART5 global interrupt P*53
  tim6_dac_isr_vector,						// TIM6 global interrupt P*54
  tim7_isr_vector,						// TIM7 global interrupt P*55
  dma2_channel1_isr_vector,				// DMA2 Channel1 global interrupt P*56
  dma2_channel2_isr_vector,				// DMA2 Channel2 global interrupt P*57
  dma2_channel3_isr_vector,				// DMA2 Channel3 global interrupt P*58
  dma2_channel4_5_isr_vector, 			// DMA2 Channel4 and DMA2 Channel5 global interrupts P*59
  dma2_channel5_isr_vector,				// Beginning from 60 Connectivity line only P*60
};
#else /*  !(defined (STM32F10X_LD_VL) || defined (STM32F10X_MD_VL) || (defined STM32F10X_HD_VL)) */
//Interrupt vector table
  __attribute__ ((section(".isr_vector")))
  const vect_fun_t const exceptions_vectors[] =
  {
    (vect_fun_t)(long)&_estack,            // The initial stack pointer
    reset_handler,             // The reset handler
    nmi_exception_vector, //NMIException
    hard_fault_exception_vector, //HardFaultException
    mem_manage_exception_vector, //MemManageException
    bus_fault_exception_vector, //BusFaultException
    usage_fault_exception_vector, //UsageFaultException
    0, 0, 0, 0,            /* Reserved */
    svc_isr_vector, //SVCHandler
    debug_monitor_isr_vector, //DebugMonitor
    0,                      /* Reserved */
    pend_svc_isr_vector, 	//PendSVC
    systick_isr_vector,//SysTickHandler,
    wwdg_isr_vector,//,WWDG_IRQHandler,	P*0
    pvd_isr_vector,//PVD_IRQHandler, P*1
    tamper_isr_vector,//TAMPER_IRQHandler, P*2
    rtc_isr_vector,//RTC_IRQHandler,TIM2->CCR1 P*3
    flash_isr_vector,//FLASH_IRQHandler, P*4
    rcc_isr_vector,//RCC_IRQHandler, P*5
    exti0_isr_vector,//EXTI0_IRQHandler, P*6
    exti1_isr_vector,//EXTI1_IRQHandler, P*7
    exti2_isr_vector,//EXTI2_IRQHandler, P*8
    exti3_isr_vector,//EXTI3_IRQHandler, P*9
    exti4_isr_vector,//EXTI4_IRQHandler, P*10
    dma1_channel1_isr_vector,//DMAChannel1_IRQHandler, P*11
    dma1_channel2_isr_vector,//DMAChannel2_IRQHandler, P*12
    dma1_channel3_isr_vector,//DMAChannel3_IRQHandler, P*13
    dma1_channel4_isr_vector,//DMAChannel4_IRQHandler, P*14
    dma1_channel5_isr_vector,//DMAChannel5_IRQHandler, P*15
    dma1_channel6_isr_vector,//DMAChannel6_IRQHandler, P*16
    dma1_channel7_isr_vector,//DMAChannel7_IRQHandler, P*17
    adc_isr_vector,//ADC_IRQHandler, P*18
    usb_hp_can_tx_isr_vector,//USB_HP_CAN_TX_IRQHandler,	P*19
    usb_lp_can_rx0_isr_vector,//USB_LP_CAN_RX0_IRQHandler, P*20
    can_rx1_isr_vector,//CAN_RX1_IRQHandler, P*21
    can_sce_isr_vector,//CAN_SCE_IRQHandler, P*22
    exti9_5_isr_vector,//EXTI9_5_IRQHandler, P*23
    tim1_brk_isr_vector,//TIM1_BRK_IRQHandler, P*24
    tim1_up_isr_vector,//TIM1_UP_IRQHandler, P*25
    tim1_trg_com_isr_vector,//TIM1_TRG_COM_IRQHandler, P*26
    tim1_cc_isr_vector,//TIM1_CC_IRQHandler, P*27
    tim2_isr_vector,//TIM2_IRQHandler, P*28
    tim3_isr_vector,//TIM3_IRQHandler, P*29
    tim4_isr_vector,//TIM4_IRQHandler, P*30
    i2c1_ev_isr_vector,//I2C1_EV_IRQHandler, P*31
    i2c1_er_isr_vector,//I2C1_ER_IRQHandler, P*32
    i2c2_ev_isr_vector,//I2C2_EV_IRQHandler, P*33
    i2c2_er_isr_vector,//I2C2_ER_IRQHandler, P*34
    spi1_isr_vector,//SPI1_IRQHandler,	P*35
    spi2_isr_vector,//SPI2_IRQHandler, P*36
    usart1_isr_vector,//USART1_IRQHandler, P*37
    usart2_isr_vector,//USART2_IRQHandler, P*38
    usart3_isr_vector,//unused_vector,//USART3_IRQHandler, P*39
    exti15_10_isr_vector,//EXTI15_10_IRQHandler, P*40
    rtc_alarm_isr_vector,//RTCAlarm_IRQHandler, P*41
    usb_wakeup_isr_vector,//USBWakeUp_IRQHandler, P*42
    tim8_brk_isr_vector,					// TIM8 break interrupt P*43
    tim8_up_isr_vector,					// TIM8 update interrupt P*44
    tim8_trg_com_isr_vector,				// TIM8 trigger and commutation interrupts P*45
    tim8_cc_isr_vector,					// TIM8 capture compare interrupt P*46
    adc3_isr_vector,						// ADC3 global interrupt P*47
    fsmc_isr_vector,						// FSMC global interrupt P*48
    sdio_isr_vector,						// SDIO global interrupt P*49
    tim5_isr_vector,						// TIM5 global interrupt P*50
    spi3_isr_vector,						// SPI3 global interrupt P*51
    usart4_isr_vector,						// USART4 global interrupt P*52
    usart5_isr_vector,						// USART5 global interrupt P*53
    tim6_isr_vector,						// TIM6 global interrupt P*54
    tim7_isr_vector,						// TIM7 global interrupt P*55
    dma2_channel1_isr_vector,				// DMA2 Channel1 global interrupt P*56
    dma2_channel2_isr_vector,				// DMA2 Channel2 global interrupt P*57
    dma2_channel3_isr_vector,				// DMA2 Channel3 global interrupt P*58
    dma2_channel4_5_isr_vector, 			// DMA2 Channel4 and DMA2 Channel5 global interrupts P*59
    dma2_channel5_isr_vector,				// Beginning from 60 Connectivity line only P*60
    eth_isr_vector,						// Ethernet isr vector P*61
    eth_wkup_isr_vector,					// Ethernet wakeup vector P*62
    can2_tx_isr_vector,					// Can2 tx P*63
    can2_rx0_isr_vector,					// Can2 rx0 P*64
    can2_rx1_isr_vector,					// Can2 rx1 P*65
    can2_sce_isr_vector,					// Can2 sce P*66
    otg_fs_isr_vector						// Usb on the go isr P*67
  };

#endif /* defined (STM32F10X_LD_VL) || defined (STM32F10X_MD_VL) || (defined STM32F10X_HD_VL) */
/*----------------------------------------------------------*/
#ifdef CPP_STARTUP_CODE	/* Include cpp startup code */

/*----------------------------------------------------------*/
#if !defined(__ARM_EABI__)

typedef void(ctors_t)(void);
extern ctors_t *__ctors_start__;		/* Constructor start */
extern ctors_t *__ctors_end__;		/* Constructor end */
extern ctors_t *__dtors_start__;		/* Constructor start */
extern ctors_t *__dtors_end__;		/* Constructor end */

/*----------------------------------------------------------*/
static void crt0_sys_construction(void)
{
    //Call constructors
      for (ctors_t ** cons = &__ctors_start__; cons < &__ctors_end__; cons++)
        if(*(cons))
            (**(cons))();
}
#endif
/*----------------------------------------------------------*/
#if defined(FUNCTION_MAIN_RETURN) && !defined(__ARM_EABI__)
static void crt0_sys_destruction(void)
{
  ctors_t **cons;
  cons = &__dtors_end__;
  while(cons > &__dtors_start__)
    if(*(--cons))
        (**(cons))();
}
#endif
/*----------------------------------------------------------*/
/*only the address of this symbol is taken by gcc*/
void *__dso_handle = (void*) &__dso_handle;

/*----------------------------------------------------------*/
#ifdef FUNCTION_MAIN_RETURN
struct cxa_object
{
        void (*func)(void*);
        void *arg;
};

#ifndef MAX_CXA_ATEXIT_OBJECTS
#define MAX_CXA_ATEXIT_OBJECTS 16
#endif

static struct cxa_object object[MAX_CXA_ATEXIT_OBJECTS];

static unsigned short iObject = 0;

int __cxa_atexit(void (*func) (void *), void * arg, void * dso_handle)
{
        (void)dso_handle;
		if (iObject >= MAX_CXA_ATEXIT_OBJECTS) return -1;
        object[iObject].func = func;
        object[iObject].arg = arg;
        ++iObject;
        return 0;
}

/* This currently destroys all objects */
void __cxa_finalize(void *d)
{
        (void)d;
		unsigned int i = iObject;
        for (; i > 0; --i)
        {
                --iObject;
                object[iObject].func( object[iObject].arg );
        }
}

#else /* FUNCTION_MAIN_RETURN */
int __cxa_atexit(void (*func)(void *), void * arg, void * dso_handle);
int __cxa_atexit(void (*func)(void *), void * arg, void * dso_handle)
{
	(void)func;
	(void)arg;
	(void)dso_handle;
	return -1;
}

#endif /* FUNCTION_MAIN_RETURN */

#ifdef __ARM_EABI__
/* Predefined symbols for EABI */
extern void __libc_init_array(void);
extern void __libc_fini_array(void);
void _init(void) {  }
void _fini(void) {}
#endif /* __ARM_EABI__ */

static void empty_func(void) {}
void _external_startup(void) __attribute__ ((weak, alias("empty_func")));
void _external_exit(void) __attribute__ ((weak, alias("empty_func")));
#endif /* CPP_STARTUP_CODE */


/*----------------------------------------------------------*/
//Default reset handler
void reset_handler(void)
{
	unsigned long *pulSrc, *pulDest;
	//Enable FPU if present
#if defined(STM32MCU_MAJOR_TYPE_F4)	&& (__FPU_PRESENT == 1) && (__FPU_USED == 1)
	SCB->CPACR |= ((3UL << 10*2)|(3UL << 11*2));	// set CP10 and CP11 Full Access
#endif
	//! Trap divide by zero
#	define SCB_CCR *((unsigned long*)0xE000ED14)
#	define    CCR_DIV0_TRP  (1UL<<4);
	SCB_CCR |= CCR_DIV0_TRP;
#	undef SCB_CCR
#	undef CCR_DIV0_TRP
    ///
    // Copy the data segment initializers from flash to SRAM.
    //
    pulSrc = &_sidata;
    for(pulDest = &_sdata; pulDest < &_edata; )
    {
        *(pulDest++) = *(pulSrc++);
    }

    //
    // Zero fill the bss segment.
    //
    for(pulDest = &_sbss; pulDest < &_ebss; )
    {
        *(pulDest++) = 0;
    }
    //Startup code before construtors call
    _external_startup();
#if defined(CPP_STARTUP_CODE)
#if !defined(__ARM_EABI__)
    //Call constructor
    crt0_sys_construction();
#else	/*__ARM_EABI__*/
    __libc_init_array();
#endif /*__ARM_EABI__*/
#endif /* CPP_STARTUP_CODE */

    // Call the application's entry point.
    //
    main();
    
#if defined(FUNCTION_MAIN_RETURN) && defined(CPP_STARTUP_CODE)
#if !defined(__ARM_EABI__)
    //Call destructor
    crt0_sys_destruction();
#else /* __ARM_EABI__*/
    __libc_fini_array();
#endif /* __ARM_EABI__*/
    __cxa_finalize(0);
#if defined(COMPILED_UNDER_ISIX) && defined(FUNCTION_MAIN_RETURN)
    _isixp_finalize();
#endif
    _external_exit();
#endif /*defined(FUNCTION_MAIN_RETURN) && defined(CPP_STARTUP_CODE)*/
    while(1);
}

/*----------------------------------------------------------*/
//EXTI pending register for route interrupt
#if defined(STM32MCU_MAJOR_TYPE_F4) || defined(STM32MCU_MAJOR_TYPE_F2)
#define EXTI_PR (*((volatile unsigned long*)(0x40000000 + 0x00010000 + 0x3C00 + 0x14)))
#else
#define EXTI_PR (*((volatile unsigned long*)(0x40010400+0x14)))
#endif
#define EXTI_PEND( flags, inp ) ((flags) & (1<<(inp)))

/************* EMULATED EXTI HANDLERS ***********************/
void exti9_5_isr_vector(void)
{
	unsigned long flags = EXTI_PR;
	if( EXTI_PEND(flags, 5) )
	{
		exti5_isr_vector();
	}
	if( EXTI_PEND(flags, 6) )
	{
		exti6_isr_vector();
	}
	if( EXTI_PEND(flags, 7) )
	{
		exti7_isr_vector();
	}
	if( EXTI_PEND(flags, 8) )
	{
		exti8_isr_vector();
	}
	if( EXTI_PEND(flags, 9) )
	{
		exti9_isr_vector();
	}
}

void exti15_10_isr_vector(void)
{
	unsigned long flags = EXTI_PR;
	if( EXTI_PEND(flags, 10) )
	{
		exti10_isr_vector();
	}
	if( EXTI_PEND(flags, 11) )
	{
		exti11_isr_vector();
	}
	if( EXTI_PEND(flags, 12) )
	{
		exti12_isr_vector();
	}
	if( EXTI_PEND(flags, 13) )
	{
		exti13_isr_vector();
	}
	if( EXTI_PEND(flags, 14) )
	{
		exti14_isr_vector();
	}
	if( EXTI_PEND(flags, 15) )
	{
		exti15_isr_vector();
	}
}
#undef EXTI_PR
#undef EXTI_PEND
/*----------------------------------------------------------*/
