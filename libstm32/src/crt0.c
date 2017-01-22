


extern unsigned long _etext;
extern unsigned long _sidata;
extern unsigned long _sdata;
extern unsigned long _edata;
extern unsigned long _sbss;
extern unsigned long _ebss;

void  __attribute__((weak,alias("empty_func")))_isixp_finalize();
extern int main(void);


#ifdef CPP_STARTUP_CODE	/* Include cpp startup code */

#if !defined(__ARM_EABI__)

typedef void(ctors_t)(void);
extern ctors_t *__ctors_start__;		/* Constructor start */
extern ctors_t *__ctors_end__;		/* Constructor end */
extern ctors_t *__dtors_start__;		/* Constructor start */
extern ctors_t *__dtors_end__;		/* Constructor end */

static void crt0_sys_construction(void)
{
    //Call constructors
      for (ctors_t ** cons = &__ctors_start__; cons < &__ctors_end__; cons++)
        if(*(cons))
            (**(cons))();
}
#endif
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
/*only the address of this symbol is taken by gcc*/
void *__dso_handle = (void*) &__dso_handle;

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

#endif /* CPP_STARTUP_CODE */
static void empty_func(void) {}
void _external_startup(void) __attribute__ ((weak, alias("empty_func")));
void _external_exit(void) __attribute__ ((weak, alias("empty_func")));


//Default reset handler
void _mcu_reset_handler_(void)
{
	unsigned long *pul_src, *pul_dest;
	//Enable FPU if present
#if defined(__ARM_FP) &&  (__ARM_FP>0)
#define SCB_CPACR  *((volatile unsigned long*)0xE000ED88)
	SCB_CPACR |= ((3UL << 10*2)|(3UL << 11*2));	// set CP10 and CP11 Full Access
#undef SCB_CPACR
#endif
	//! Trap divide by zero
#	define SCB_CCR *((volatile unsigned long*)0xE000ED14)
#	define    CCR_DIV0_TRP  (1UL<<4);
	SCB_CCR |= CCR_DIV0_TRP;
#	undef SCB_CCR
#	undef CCR_DIV0_TRP
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

