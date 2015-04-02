#include <isix/config.h>
#include <isix/printk.h>
#include <isix/types.h>
/* Unix utilities */
#include <ucontext.h>
#include <sys/types.h>
#include <sys/time.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#define _ISIX_KERNEL_CORE_
#include <isix/prv/scheduler.h>
/*-----------------------------------------------------------------------*/
//On the pc stack size is alwas assumed as 256k
#define STACK_SIZE (256*1024)

/*-----------------------------------------------------------------------*/
static ucontext_t signal_context;  /* The interrupt context */
static void *signal_stack;
static sigset_t sigblock_mask;
/*-----------------------------------------------------------------------*/
//Create of stack context
unsigned long* _isixp_task_init_stack(unsigned long *sp, task_func_ptr_t pfun, void *param)
{
    ucontext_t* uc = calloc(1,sizeof(ucontext_t));
    if( uc == NULL )
    {
        perror("malloc");
        exit(1);
    }
    void *stack = malloc(STACK_SIZE);
    if (stack == NULL)
    {
        perror("malloc");
        exit(1);
    }
    getcontext( uc );
    /*  and a sigmask */
    uc->uc_stack.ss_sp = stack;
    uc->uc_stack.ss_size = STACK_SIZE;
    uc->uc_stack.ss_flags = 0;
    sigemptyset(&uc->uc_sigmask);
    /* setup the function we're going to, and n-1 arguments. */
    makecontext(uc, (void (*)(void))pfun, 1, param );
    printf("Init stack %p\n",uc);
    return (unsigned long*)uc;
}

/* ----------------------------------------------------------------------*/
static void unused_func(void ) {}
void isix_systime_handler(void) __attribute__ ((weak, alias("unused_func")));

/*-----------------------------------------------------------------------*/
//Cyclic schedule time interrupt

static void schedule_time(void)
{
       //Increment system ticks
	   _isixp_enter_critical();
       //Call isix system time handler if used
       isix_systime_handler();
       _isixp_schedule_time();
       //Clear interrupt mask
	   _isixp_exit_critical();
       //Yeld
       port_yield();
}

/*-----------------------------------------------------------------------*/
//Set interrupt mask
void port_set_interrupt_mask(void)
{
    if(sigprocmask(SIG_BLOCK,&sigblock_mask, NULL)==-1)
    {
	    perror("Sigblock"); exit(1);
    }
}

/*-----------------------------------------------------------------------*/
//Clear interrupt mask
void port_clear_interrupt_mask(void)
{
    if(sigprocmask(SIG_UNBLOCK,&sigblock_mask, NULL)==-1)
    {
	    perror("Sigblock"); exit(1);
    }
}

/*-----------------------------------------------------------------------*/
//Yield to another task
void port_yield( void )
{
    _isixp_schedule();
    setcontext( (ucontext_t*)currp->top_stack );
}

/*-----------------------------------------------------------------------*/

/*
 *   Timer interrupt handler.
 *   Creates a new context to run the scheduler in, masks signals, then swaps
 *   contexts saving the previously executing thread and jumping to the
 *   scheduler.
 *         */
static void timer_interrupt(int j, siginfo_t *si, void *old_context)
{
        //printf("Context switch\n");
        /* Create new scheduler context */
        getcontext(&signal_context);
        signal_context.uc_stack.ss_sp = signal_stack;
        signal_context.uc_stack.ss_size = STACK_SIZE;
        signal_context.uc_stack.ss_flags = 0;
        sigemptyset(&signal_context.uc_sigmask);
        makecontext(&signal_context, schedule_time, 0);

#ifdef ISIX_CONFIG_USE_PREEMPTION
    /* Set a PendSV to request a context switch. */
     if(schrun)
     {
        /* save running thread, jump to scheduler */
        swapcontext((ucontext_t*)currp->top_stack ,&signal_context);
     }
#endif
}
/*-----------------------------------------------------------------------*/

/* Set up SIGALRM signal handler */
static void setup_signals(void)
{
    struct sigaction act;
    act.sa_sigaction = timer_interrupt;
    sigemptyset(&act.sa_mask);
    act.sa_flags = SA_RESTART | SA_SIGINFO;
    if(sigaction(SIGALRM, &act, NULL) != 0) 
    {
        perror("Signal handler");
    }
}

/*-----------------------------------------------------------------------*/
//Start first task by svc call
void port_start_first_task( void )
{
    //Setup sigblock_mask;
    if(sigemptyset(&sigblock_mask) == -1 || sigaddset(&sigblock_mask,SIGALRM) == -1 )
    {
	perror("Sigblock");
    }
    struct itimerval it;
    signal_stack = malloc(STACK_SIZE);
    if (signal_stack == NULL)
    {
      perror("malloc");
      exit(1);
    }
    //Setup signals
    setup_signals();
    it.it_interval.tv_sec = 0;
    it.it_interval.tv_usec = 1000;
    it.it_value = it.it_interval;
    if( setitimer(ITIMER_REAL, &it, NULL) )
    {
        perror("itimer");
    }
    _isixp_schedule();
    printf("first task uccontext %p\n",currp->top_stack);
    setcontext( (ucontext_t*)currp->top_stack  );
}

/*-----------------------------------------------------------------------*/
//Cleanup task dealocate the memory
void port_cleanup_task( void *sp )
{
    ucontext_t *uc = (ucontext_t*)(sp);
    free( signal_context.uc_stack.ss_sp );
    free(uc);
}  
/*-----------------------------------------------------------------------*/
