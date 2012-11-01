/* ------------------------------------------------------------------ */
/* System architecture lwip for isix operating system
 * Copyright (c) Lucjan Bryndza 2012
 * All Rights reserverd
 */
/* ------------------------------------------------------------------ */
#include "lwip/debug.h"
#include "arch/sys_arch.h"
#include "lwip/debug.h"
#include "lwip/def.h"
#include "lwip/sys.h"
#include "lwip/mem.h"
#include "lwip/timers.h"
//#include <dbglog.h>
#undef dbprintf
#define dbprintf(...) do {} while(0)
/* ------------------------------------------------------------------ */

//Default task structure
static struct sys_thread *threads = NULL;

//Semaphore for locking the mempool
static sem_t* arch_protect_sem;

/* ------------------------------------------------------------------ */
struct sys_thread
{
  struct sys_thread *next;
  struct sys_timeo timeouts;
  task_t *task;
};

/* ------------------------------------------------------------------ */
//Create basic thread structure for LWIP
static struct sys_thread* introduce_thread(task_t *task)
{
	struct sys_thread *thread;
    thread = isix_alloc(sizeof(struct sys_thread));
    if(thread)
    {
        thread->next = NULL;
        thread->timeouts.next = NULL;
        thread->task = task;
    }
    if(threads==NULL)
    {
        threads = thread;
    }
    else
    {
        struct sys_thread *st;
        for(st=threads;st->next!=NULL;st=st->next);
        st->next = thread;
    }
    return thread;
}

/* ------------------------------------------------------------------ */
/* Return current thread with per process timeouts */
static struct sys_thread * current_thread(void)
{
    struct sys_thread *st;
    task_t *t;
    t = isix_task_self();
    for(st=threads;st!=NULL;st=st->next)
    {
        if(st->task==t) return st;
    }
    LWIP_ASSERT(st,"st==NULL");
    return st;
} 

/* ------------------------------------------------------------------ */
/** The only thread function:
 * Creates a new thread
 * @param name human-readable name for the thread (used for debugging purposes)
 * @param thread thread-function
 * @param arg parameter passed to 'thread'
 * @param stacksize stack size in bytes for the new thread (may be ignored by ports)
 * @param prio priority of the new thread (may be ignored by ports) */
sys_thread_t sys_thread_new(const char *name, lwip_thread_fn thread, void *arg, int stacksize, int prio)
{
    (void)name;
	struct sys_thread *st = NULL;
    task_t* task = isix_task_create( thread, arg, stacksize, prio);
    if(task != NULL )
    {
        st = introduce_thread(task);
    }
    return st;
}


/* ------------------------------------------------------------------ */
/** Create a new semaphore
 * @param sem pointer to the semaphore to create
 * @param count initial count of the semaphore
 * @return ERR_OK if successful, another err_t otherwise */
err_t sys_sem_new(sys_sem_t *sem, u8_t count)
{
    *sem = isix_sem_create(NULL,count);
    return (*sem)?(ERR_OK):(ERR_MEM);
}


/* ------------------------------------------------------------------ */
/** Delete a semaphore
 * @param sem semaphore to delete */
void sys_sem_free(sys_sem_t *sem)
{
    isix_sem_destroy(*sem);
}

/* ------------------------------------------------------------------ */
/** Signals a semaphore
 * @param sem the semaphore to signal */
void sys_sem_signal(sys_sem_t *sem)
{
    isix_sem_signal(*sem);
}

/* ------------------------------------------------------------------ */
/** Wait for a semaphore for the specified timeout
 * @param sem the semaphore to wait for
 * @param timeout timeout in milliseconds to wait (0 = wait forever)
 * @return time (in milliseconds) waited for the semaphore
 *         or SYS_ARCH_TIMEOUT on timeout */
u32_t sys_arch_sem_wait(sys_sem_t *sem, u32_t timeout)
{
    timeout = (timeout*ISIX_HZ)/1000;
    tick_t t = isix_get_jiffies();
    int reason = isix_sem_wait(*sem,timeout);
    t =  isix_get_jiffies() - t;
    t = (t*1000)/ISIX_HZ;
    if(reason == ISIX_ETIMEOUT) return SYS_ARCH_TIMEOUT;
    return t;
}


/* ------------------------------------------------------------------ */
/*
  Returns a pointer to the per-thread sys_timeouts structure. In lwIP,
  each thread has a list of timeouts which is represented as a linked
  list of sys_timeout structures. The sys_timeouts structure holds a
  pointer to a linked list of timeouts. This function is called by
  the lwIP timeout scheduler and must not return a NULL value.

  In a single threaded sys_arch implementation, this function will
  simply return a pointer to a global sys_timeouts variable stored in
  the sys_arch module.
*/
struct sys_timeo* sys_arch_timeouts(void)
{
    struct sys_thread *thread;
    thread = current_thread();
    if(thread==NULL) return NULL;
    return &thread->timeouts;
}
/* ------------------------------------------------------------------ */
/*
Blocks the thread until a message arrives in the mailbox, but does
not block the thread longer than "timeout" milliseconds (similar to
the sys_arch_sem_wait() function). The "msg" argument is a result
parameter that is set by the function (i.e., by doing "*msg =
ptr"). The "msg" parameter maybe NULL to indicate that the message
should be dropped.

The return values are the same as for the sys_arch_sem_wait() function:
Number of milliseconds spent waiting or SYS_ARCH_TIMEOUT if there was a
timeout.

Note that a function with a similar name, sys_mbox_fetch(), is
implemented by lwIP.
*/
typedef void* message_t;

u32_t sys_arch_mbox_fetch(sys_mbox_t *mbox, message_t *msg, u32_t timeout)
{
    message_t m;
	timeout = (timeout*ISIX_HZ)/1000;
    tick_t t = isix_get_jiffies();
    int reason = isix_fifo_read( *mbox, &m, timeout);
    if(reason == ISIX_ETIMEOUT)
    	return SYS_ARCH_TIMEOUT;
    else
    {
    	dbprintf("F s=%08x v=%08x", *mbox, m);
    	if( msg ) *msg = m;
    	t = isix_get_jiffies() - t;
    	t = (t*1000)/ISIX_HZ;
    	return t;
    }
}

/* ------------------------------------------------------------------ */
/*
  Deallocates a mailbox. If there are messages still present in the
  mailbox when the mailbox is deallocated, it is an indication of a
  programming error in lwIP and the developer should be notified.
*/
void sys_mbox_free(sys_mbox_t *mbox)
{
    isix_fifo_destroy(*mbox);
}

/* ------------------------------------------------------------------ */
/** Create a new mbox of specified size
 * @param mbox pointer to the mbox to create
 * @param size (miminum) number of messages in this mbox
 * @return ERR_OK if successful, another err_t otherwise */
err_t sys_mbox_new(sys_mbox_t *mbox, int size)
{
   *mbox = isix_fifo_create(size,sizeof(void*));
   return (*mbox)?(ERR_OK):(ERR_MEM);
}
/* ------------------------------------------------------------------ */
/* Post the message to the mailbox */
void sys_mbox_post(sys_mbox_t *mbox,  message_t msg)
{
	dbprintf("P s=%08x v=%08x",*mbox,msg);
	isix_fifo_write( *mbox, &msg, ISIX_TIME_INFINITE );
}
/* ------------------------------------------------------------------ */
/** Wait for a new message to arrive in the mbox
 * @param mbox mbox to get a message from
 * @param msg pointer where the message is stored
 * @param timeout maximum time (in milliseconds) to wait for a message
 * @return 0 (milliseconds) if a message has been received
 *         or SYS_MBOX_EMPTY if the mailbox is empty */
u32_t sys_arch_mbox_tryfetch(sys_mbox_t *mbox, message_t *msg)
{
	message_t m;
	int reason = isix_fifo_read( *mbox, &m, 1 );
	if( msg && reason==ISIX_EOK) *msg = m;
	dbprintf("TF s=%08x v=%08x",*mbox,msg);
	return ( reason==ISIX_EOK )?( 0 ):( SYS_MBOX_EMPTY );
}

/* ------------------------------------------------------------------ */
/** Try to post a message to an mbox - may fail if full or ISR
 * @param mbox mbox to posts the message
 * @param msg message to post (ATTENTION: can be NULL) */
err_t sys_mbox_trypost(sys_mbox_t *mbox, message_t msg)
{
	dbprintf("TP s=%08x v=%08x", *mbox, msg);
	int reason = isix_fifo_write( *mbox, &msg, 1 );
	return ( reason==ISIX_EOK )?( 0 ):( SYS_MBOX_EMPTY );
}

/* ------------------------------------------------------------------ */
/*
 * Perform a "fast" protect. This could be implemented by
 * disabling interrupts for an embedded system or by using a semaphore or
 * mutex. The implementation should allow calling SYS_ARCH_PROTECT when
 * already protected. The old protection level is returned in the variable
 * "lev". This macro will default to calling the sys_arch_protect() function
 * which should be implemented in sys_arch.c. If a particular port needs a
 * different implementation, then this macro may be defined in sys_arch.h
*/
/* ------------------------------------------------------------------ */
sys_prot_t sys_arch_protect(void)
{
	LWIP_ASSERT("Invalid TCPIP lock semaphore", arch_protect_sem );
	isix_sem_wait( arch_protect_sem, ISIX_TIME_INFINITE );
	return 1;
}
/*-----------------------------------------------------------------------------------*/
/** void sys_arch_unprotect(sys_prot_t pval)

This optional function does a "fast" set of critical region protection to the
value specified by pval. See the documentation for sys_arch_protect() for
more information. This function is only required if your port is supporting
an operating system.
*/
void sys_arch_unprotect(sys_prot_t pval)
{
	(void)pval;
	LWIP_ASSERT("Invalid TCPIP lock semaphore", arch_protect_sem );
	isix_sem_signal( arch_protect_sem );
}
/* ------------------------------------------------------------------ */
/* Add system initialize stuff before tcpip init */
void sys_init(void)
{
	arch_protect_sem = isix_sem_create( NULL, 1 );
	LWIP_ASSERT("Unable to create lock semaphore", arch_protect_sem );
}

/* ------------------------------------------------------------------ */


