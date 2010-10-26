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
extern void _estack;				/* init value for the stack pointer. defined in linker script */


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
void nmi_exception_vector(void) __attribute__ ((interrupt, weak, alias("unused_vector")));
void hard_fault_exception_vector(void) __attribute__ ((interrupt, weak, alias("unused_vector")));
void mem_manage_exception_vector(void) __attribute__ ((interrupt, weak, alias("unused_vector")));
void bus_fault_exception_vector(void) __attribute__ ((interrupt, weak, alias("unused_vector")));
void usage_fault_exception_vector(void) __attribute__ ((interrupt, weak, alias("unused_vector")));
#ifndef __UNDER_ISIX__
void svc_isr_vector(void) __attribute__ ((interrupt, weak, alias("unused_vector")));
#else
void svc_isr_vector(void) __attribute__ ((interrupt));
#endif
void debug_monitor_isr_vector(void) __attribute__ ((interrupt, weak, alias("unused_vector")));
#ifndef __UNDER_ISIX__
void pend_svc_isr_vector(void) __attribute__ ((interrupt, weak, alias("unused_vector")));
void systick_isr_vector(void) __attribute__ ((interrupt, weak, alias("unused_vector")));
#else
void pend_svc_isr_vector(void) __attribute__ ((interrupt));
void systick_isr_vector(void) __attribute__ ((interrupt));
#endif
void wwdg_isr_vector(void) __attribute__ ((interrupt, weak, alias("unused_vector")));
void pvd_isr_vector(void) __attribute__ ((interrupt, weak, alias("unused_vector")));
void tamper_isr_vector(void) __attribute__ ((interrupt, weak, alias("unused_vector")));
void rtc_isr_vector(void) __attribute__ ((interrupt, weak, alias("unused_vector")));
void flash_isr_vector(void) __attribute__ ((interrupt, weak, alias("unused_vector")));
void rcc_isr_vector(void) __attribute__ ((interrupt, weak, alias("unused_vector")));
void exti0_isr_vector(void) __attribute__ ((interrupt, weak, alias("unused_vector")));
void exti1_isr_vector(void) __attribute__ ((interrupt, weak, alias("unused_vector")));
void exti2_isr_vector(void) __attribute__ ((interrupt, weak, alias("unused_vector")));
void exti3_isr_vector(void) __attribute__ ((interrupt, weak, alias("unused_vector")));
void exti4_isr_vector(void) __attribute__ ((interrupt, weak, alias("unused_vector")));
void dma1_channel1_isr_vector(void) __attribute__ ((interrupt, weak, alias("unused_vector")));
void dma1_channel2_isr_vector(void) __attribute__ ((interrupt, weak, alias("unused_vector")));
void dma1_channel3_isr_vector(void) __attribute__ ((interrupt, weak, alias("unused_vector")));
void dma1_channel4_isr_vector(void) __attribute__ ((interrupt, weak, alias("unused_vector")));
void dma1_channel5_isr_vector(void) __attribute__ ((interrupt, weak, alias("unused_vector")));
void dma1_channel6_isr_vector(void) __attribute__ ((interrupt, weak, alias("unused_vector")));
void dma1_channel7_isr_vector(void) __attribute__ ((interrupt, weak, alias("unused_vector")));
void adc_isr_vector(void) __attribute__ ((interrupt, weak, alias("unused_vector")));
void usb_hp_can_tx_isr_vector(void) __attribute__ ((interrupt, weak, alias("unused_vector")));
void usb_lp_can_rx0_isr_vector(void) __attribute__ ((interrupt, weak, alias("unused_vector")));
void can_rx1_isr_vector(void) __attribute__ ((interrupt, weak, alias("unused_vector")));
void can_sce_isr_vector(void) __attribute__ ((interrupt, weak, alias("unused_vector")));
void exti9_5_isr_vector(void) __attribute__ ((interrupt, weak, alias("unused_vector")));
void tim1_brk_isr_vector(void) __attribute__ ((interrupt, weak, alias("unused_vector")));
void tim1_up_isr_vector(void) __attribute__ ((interrupt, weak, alias("unused_vector")));
void tim1_trg_com_isr_vector(void) __attribute__ ((interrupt, weak, alias("unused_vector")));
void tim1_cc_isr_vector(void) __attribute__ ((interrupt, weak, alias("unused_vector")));
void tim2_isr_vector(void) __attribute__ ((interrupt, weak, alias("unused_vector")));
void tim3_isr_vector(void) __attribute__ ((interrupt, weak, alias("unused_vector")));
void tim4_isr_vector(void) __attribute__ ((interrupt, weak, alias("unused_vector")));
void i2c1_ev_isr_vector(void) __attribute__ ((interrupt, weak, alias("unused_vector")));
void i2c1_er_isr_vector(void) __attribute__ ((interrupt, weak, alias("unused_vector")));
void i2c2_ev_isr_vector(void) __attribute__ ((interrupt, weak, alias("unused_vector")));
void i2c2_er_isr_vector(void) __attribute__ ((interrupt, weak, alias("unused_vector")));
void spi1_isr_vector(void) __attribute__ ((interrupt, weak, alias("unused_vector")));
void spi2_isr_vector(void) __attribute__ ((interrupt, weak, alias("unused_vector")));
void usart1_isr_vector(void) __attribute__ ((interrupt, weak, alias("unused_vector")));
void usart2_isr_vector(void) __attribute__ ((interrupt, weak, alias("unused_vector")));
void usart3_isr_vector(void) __attribute__ ((interrupt, weak, alias("unused_vector")));
void exti15_10_isr_vector(void) __attribute__ ((interrupt, weak, alias("unused_vector")));
void rtc_alarm_isr_vector(void) __attribute__ ((interrupt, weak, alias("unused_vector")));
void usb_wakeup_isr_vector(void) __attribute__ ((interrupt, weak, alias("unused_vector")));
void tim8_brk_isr_vector(void) __attribute__ ((interrupt, weak, alias("unused_vector")));
void tim8_up_isr_vector(void) __attribute__ ((interrupt, weak, alias("unused_vector")));
void tim8_trg_com_isr_vector(void) __attribute__ ((interrupt, weak, alias("unused_vector")));
void tim8_cc_isr_vector(void) __attribute__ ((interrupt, weak, alias("unused_vector")));
void adc3_isr_vector(void) __attribute__ ((interrupt, weak, alias("unused_vector")));
void fsmc_isr_vector(void) __attribute__ ((interrupt, weak, alias("unused_vector")));
void sdio_isr_vector(void) __attribute__ ((interrupt, weak, alias("unused_vector")));
void tim5_isr_vector(void) __attribute__ ((interrupt, weak, alias("unused_vector")));
void spi3_isr_vector(void) __attribute__ ((interrupt, weak, alias("unused_vector")));
void usart4_isr_vector(void) __attribute__ ((interrupt, weak, alias("unused_vector")));
void usart5_isr_vector(void) __attribute__ ((interrupt, weak, alias("unused_vector")));
void tim6_isr_vector(void) __attribute__ ((interrupt, weak, alias("unused_vector")));
void tim7_isr_vector(void) __attribute__ ((interrupt, weak, alias("unused_vector")));
void dma2_channel1_isr_vector(void) __attribute__ ((interrupt, weak, alias("unused_vector")));
void dma2_channel2_isr_vector(void) __attribute__ ((interrupt, weak, alias("unused_vector")));
void dma2_channel3_isr_vector(void) __attribute__ ((interrupt, weak, alias("unused_vector")));
void dma2_channel4_5_isr_vector(void) __attribute__ ((interrupt, weak, alias("unused_vector")));
/* Connectivity line only */
void dma2_channel5_isr_vector(void) __attribute__ ((interrupt, weak, alias("unused_vector")));
void eth_isr_vector(void) __attribute__ ((interrupt, weak, alias("unused_vector")));
void eth_wkup_isr_vector(void) __attribute__ ((interrupt, weak, alias("unused_vector")));
void can2_tx_isr_vector(void) __attribute__ ((interrupt, weak, alias("unused_vector")));
void can2_rx0_isr_vector(void) __attribute__ ((interrupt, weak, alias("unused_vector")));
void can2_rx1_isr_vector(void) __attribute__ ((interrupt, weak, alias("unused_vector")));
void can2_sce_isr_vector(void) __attribute__ ((interrupt, weak, alias("unused_vector")));
void otg_fs_isr_vector(void) __attribute__ ((interrupt, weak, alias("unused_vector")));

/*----------------------------------------------------------*/
//Interrupt vector table
__attribute__ ((section(".isr_vector")))
const vect_fun_t const exceptions_vectors[] =
{
  (vect_fun_t)&_estack,            // The initial stack pointer
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
void *__dso_handle; /*only the address of this symbol is taken by gcc*/

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
        if (iObject >= MAX_CXA_ATEXIT_OBJECTS) return -1;
        object[iObject].func = func;
        object[iObject].arg = arg;
        ++iObject;
        return 0;
}

/* This currently destroys all objects */
void __cxa_finalize(void *d)
{
        unsigned int i = iObject;
        for (; i > 0; --i)
        {
                --iObject;
                object[iObject].func( object[iObject].arg );
        }
}

#else /* FUNCTION_MAIN_RETURN */
int __cxa_atexit(void (*func) (void *), void * arg, void * dso_handle)
{
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
void __external_startup(void) __attribute__ ((weak, alias("empty_func")));

#endif /* CPP_STARTUP_CODE */


/*----------------------------------------------------------*/
//Default reset handler
void reset_handler(void)
{
	unsigned long *pulSrc, *pulDest;

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

#if defined(CPP_STARTUP_CODE)
    //Startup code before construtors call
    __external_startup();
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

#endif /*defined(FUNCTION_MAIN_RETURN) && defined(CPP_STARTUP_CODE)*/
    while(1);
}

/*----------------------------------------------------------*/
#ifndef __UNDER_ISIX__
/* thread safe constructed objects  */
void __cxa_guard_acquire ()
{
  return;
}

void __cxa_guard_release ()
{
  return;
}


//Pure virtual call error handler
void __cxa_pure_virtual()
{
	while (1);
}

#endif

