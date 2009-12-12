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
void svc_isr_vector(void) __attribute__ ((interrupt, weak, alias("unused_vector")));
void debug_monitor_isr_vector(void) __attribute__ ((interrupt, weak, alias("unused_vector")));
void pend_svc_isr_vector(void) __attribute__ ((interrupt, weak, alias("unused_vector")));
void systick_isr_vector(void) __attribute__ ((interrupt, weak, alias("unused_vector")));
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
  wwdg_isr_vector,//,WWDG_IRQHandler,
  pvd_isr_vector,//PVD_IRQHandler,
  tamper_isr_vector,//TAMPER_IRQHandler,
  rtc_isr_vector,//RTC_IRQHandler,TIM2->CCR1
  flash_isr_vector,//FLASH_IRQHandler,
  rcc_isr_vector,//RCC_IRQHandler,
  exti0_isr_vector,//EXTI0_IRQHandler,
  exti1_isr_vector,//EXTI1_IRQHandler,
  exti2_isr_vector,//EXTI2_IRQHandler,
  exti3_isr_vector,//EXTI3_IRQHandler,
  exti4_isr_vector,//EXTI4_IRQHandler,
  dma1_channel1_isr_vector,//DMAChannel1_IRQHandler,
  dma1_channel2_isr_vector,//DMAChannel2_IRQHandler,
  dma1_channel3_isr_vector,//DMAChannel3_IRQHandler,
  dma1_channel4_isr_vector,//DMAChannel4_IRQHandler,
  dma1_channel5_isr_vector,//DMAChannel5_IRQHandler,
  dma1_channel6_isr_vector,//DMAChannel6_IRQHandler,
  dma1_channel7_isr_vector,//DMAChannel7_IRQHandler,
  adc_isr_vector,//ADC_IRQHandler,
  usb_hp_can_tx_isr_vector,//USB_HP_CAN_TX_IRQHandler,
  usb_lp_can_rx0_isr_vector,//USB_LP_CAN_RX0_IRQHandler,
  can_rx1_isr_vector,//CAN_RX1_IRQHandler,
  can_sce_isr_vector,//CAN_SCE_IRQHandler,
  exti9_5_isr_vector,//EXTI9_5_IRQHandler,
  tim1_brk_isr_vector,//TIM1_BRK_IRQHandler,
  tim1_up_isr_vector,//TIM1_UP_IRQHandler,
  tim1_trg_com_isr_vector,//TIM1_TRG_COM_IRQHandler,
  tim1_cc_isr_vector,//TIM1_CC_IRQHandler,
  tim2_isr_vector,//TIM2_IRQHandler,
  tim3_isr_vector,//TIM3_IRQHandler,
  tim4_isr_vector,//TIM4_IRQHandler,
  i2c1_ev_isr_vector,//I2C1_EV_IRQHandler,
  i2c1_er_isr_vector,//I2C1_ER_IRQHandler,
  i2c2_ev_isr_vector,//I2C2_EV_IRQHandler,
  i2c2_er_isr_vector,//I2C2_ER_IRQHandler,
  spi1_isr_vector,//SPI1_IRQHandler,
  spi2_isr_vector,//SPI2_IRQHandler,
  usart1_isr_vector,//USART1_IRQHandler,
  usart2_isr_vector,//USART2_IRQHandler,
  usart3_isr_vector,//unused_vector,//USART3_IRQHandler,
  exti15_10_isr_vector,//EXTI15_10_IRQHandler,
  rtc_alarm_isr_vector,//RTCAlarm_IRQHandler,
  usb_wakeup_isr_vector,//USBWakeUp_IRQHandler,
  tim8_brk_isr_vector,					// TIM8 break interrupt
  tim8_up_isr_vector,					// TIM8 update interrupt
  tim8_trg_com_isr_vector,				// TIM8 trigger and commutation interrupts
  tim8_cc_isr_vector,					// TIM8 capture compare interrupt
  adc3_isr_vector,						// ADC3 global interrupt
  fsmc_isr_vector,						// FSMC global interrupt
  sdio_isr_vector,						// SDIO global interrupt
  tim5_isr_vector,						// TIM5 global interrupt
  spi3_isr_vector,						// SPI3 global interrupt
  usart4_isr_vector,						// USART4 global interrupt
  usart5_isr_vector,						// USART5 global interrupt
  tim6_isr_vector,						// TIM6 global interrupt
  tim7_isr_vector,						// TIM7 global interrupt
  dma2_channel1_isr_vector,				// DMA2 Channel1 global interrupt
  dma2_channel2_isr_vector,				// DMA2 Channel2 global interrupt
  dma2_channel3_isr_vector,				// DMA2 Channel3 global interrupt
  dma2_channel4_5_isr_vector 			// DMA2 Channel4 and DMA2 Channel5 global interrupts
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
#ifdef __UNDER_ISIX__
/* thread safe constructed objects  */
void __cxa_guard_acquire ()
{
  return;
}

void __cxa_guard_release ()
{
  return;
}
#endif

/*----------------------------------------------------------*/
//Pure virtual call error handler
void __cxa_pure_virtual()
{
	while (1);
}
