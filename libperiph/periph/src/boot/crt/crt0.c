


extern unsigned long _etext;
extern unsigned long _sidata;
extern unsigned long _sdata;
extern unsigned long _edata;
extern unsigned long _sbss;
extern unsigned long _ebss;
extern unsigned char __heap_start;
extern unsigned char __heap_end;


void  __attribute__((weak,alias("empty_func")))_isixp_finalize();
extern int main(void);
extern void __libc_init_array(void);
extern void __libc_fini_array(void);
void _init(void) {  }
void _fini(void) {}
static void empty_func(void) {}
void _external_startup(void) __attribute__ ((weak, alias("empty_func")));
void _external_exit(void) __attribute__ ((weak, alias("empty_func")));



/*only the address of this symbol is taken by gcc*/
void *__dso_handle = (void*) &__dso_handle;

struct cxa_object
{
        void (*func)(void*);
        void *arg;
};

#ifndef CONFIG_ISIX_MAX_CXA_ATEXIT_OBJECTS
#define CONFIG_ISIX_MAX_CXA_ATEXIT_OBJECTS 16
#endif

static struct cxa_object object[CONFIG_ISIX_MAX_CXA_ATEXIT_OBJECTS];

static unsigned short iObject = 0;

int __cxa_atexit(void (*func) (void *), void * arg, void * dso_handle)
{
        (void)dso_handle;
		if (iObject >= CONFIG_ISIX_MAX_CXA_ATEXIT_OBJECTS) return -1;
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


// Early cpu config
static void early_cpu_setup(void)
{

	//Enable FPU if present
#	if defined(__ARM_FP) &&  (__ARM_FP>0)
#	define SCB_CPACR  *((volatile unsigned long*)0xE000ED88)
	SCB_CPACR |= ((3UL << 10*2)|(3UL << 11*2));	// set CP10 and CP11 Full Access
#	undef SCB_CPACR
#	endif
	//! Trap divide by zero
#	define SCB_CCR *((volatile unsigned long*)0xE000ED14)
#	define    CCR_DIV0_TRP  (1UL<<4)
#	define	  CCR_STKALIGN  (1UL<<9)
	SCB_CCR |= CCR_DIV0_TRP|CCR_STKALIGN;
#	undef SCB_CCR
#	undef CCR_DIV0_TRP
#	undef CCR_STKALIGN
}


//Default reset handler
__attribute__((noreturn))
void _mcu_reset_handler_(void)
{
	unsigned long *pul_src, *pul_dest;
	early_cpu_setup();
    ///
    // Copy the data segment initializers from flash to SRAM.
    //
    pul_src = &_sidata;
    for(pul_dest = &_sdata; pul_dest < &_edata; )
    {
        *(pul_dest++) = *(pul_src++);
    }

    //
    // Zero fill the bss segment.
    //
    for(pul_dest = &_sbss; pul_dest < &_ebss; )
    {
        *(pul_dest++) = 0;
    }
	// Clear the heap
	for(pul_dest = (unsigned long*)&__heap_start; pul_dest < (unsigned long*)&__heap_end; )
	{
		*(pul_dest++) = 0;
	}
   
    //Machine specific code called just before constructor
    _external_startup();
    //Call constructors
    __libc_init_array();
    // Call the application's entry point.
    //
    main();
    //Call destructors
    __libc_fini_array();
    __cxa_finalize(0);
	// Called final rtos destructor
    _isixp_finalize();
    _external_exit();
    while(1);
}

