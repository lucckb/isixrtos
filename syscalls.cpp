/* ------------------------------------------------------------ */
/*
 *    tiny_alloc.cpp
 *
 *     Created on: 2009-08-08
 *     Author: lucck
 *
 * ------------------------------------------------------------ */
#ifndef COMPILED_UNDER_ISIX

#include "tiny_alloc.h"

#else /*COMPILED_UNDER_ISIX*/

#include <isix.h>
#endif
/* -------------------------------------------------------------- */
#include <cstring>
/* -------------------------------------------------------------- */

#ifndef COMPILED_UNDER_ISIX

//It can be redefined
#define foundation_alloc fnd::tiny_alloc
#define foundation_free fnd::tiny_free
#define terminate_process() while(1)

#else /*COMPILED_UNDER_ISIX*/


#define foundation_alloc isix::isix_alloc
#define foundation_free isix::isix_free
#define terminate_process() isix::isix_bug()

#endif /*COMPILED_UNDER_ISIX*/

/* -------------------------------------------------------------- */
#ifndef  CONFIG_ENABLE_EXCEPTIONS
void* operator new( size_t n ) throw()
{
    if(n==0) n++;
    void *ptr = foundation_alloc(n);
    if(!ptr) terminate_process();
    return ptr;
}

void operator delete( void* p) throw()
{
    if(p)
    	foundation_free(p);
}

void* operator new[]( size_t n) throw()
{
    if(n==0) n++;
    void *ptr = foundation_alloc(n);
    if(!ptr) terminate_process();
    return ptr;
}

void operator delete[]( void* p) throw()
{
    if(p)
    	foundation_free(p);
}
#endif /* CONFIG_ENABLE_EXCEPTIONS  */

/* -------------------------------------------------------------- */
// Syscalls definitions
extern "C"
{
	void *_malloc_r(struct _reent */*r*/, size_t size);
	void _free_r(struct _reent */*r*/, void *ptr);
	void* malloc(size_t size);
	void free(void *ptr);
	void *calloc(size_t nmemb, size_t size);
	void *realloc(void */*ptr*/, size_t /*size*/);
	void abort(void);
	int __cxa_guard_acquire(void);
	void __cxa_guard_release (void);
	void __cxa_pure_virtual();
	int * __errno(void);
	void __cxa_guard_abort(void *);
}
/* -------------------------------------------------------------- */
#ifdef COMPILED_UNDER_ISIX
static isix::sem_t *ctors_sem;
#endif
/* -------------------------------------------------------------- */
#ifdef CPP_STARTUP_CODE

/* thread safe constructed objects  */
int __cxa_guard_acquire(void)
{
#ifdef COMPILED_UNDER_ISIX
  if(ctors_sem==NULL)
  {
	  ctors_sem = isix::isix_sem_create(NULL,1);
	  if(ctors_sem==NULL)
	  {
		  //Remove task if can;t create semaphore
		  terminate_process();
	  }
  }
  if(ctors_sem && isix::isix_is_scheduler_active())
  {
	  isix::isix_sem_wait(ctors_sem,isix::ISIX_TIME_INFINITE);
  }
  return 1;
#else
  return 1;
#endif
}

void __cxa_guard_release(void)
{
#ifdef COMPILED_UNDER_ISIX
	if(ctors_sem && isix::isix_is_scheduler_active())
	{
		isix::isix_sem_signal(ctors_sem);
	}
#endif
}

//Pure virtual call error handler
void __cxa_pure_virtual()
{
	terminate_process();
}

void __cxa_guard_abort(void *)
{
	terminate_process();
}

#endif /*CPP_STARTUP_CODE*/

/* ------------------------------------------------------------------ */


void abort(void)
{
	terminate_process();
	for(;;);	//Prevent warning
}

void* malloc(size_t size)
{
	return foundation_alloc(size);
}

void free(void *ptr)
{
	foundation_free(ptr);
}

void *calloc(size_t nmemb, size_t size)
{
	size_t ns = nmemb * size;
	void *ptr = malloc(ns);
	if(ptr)
	{
		std::memset(ptr,0,ns);
	}
	return ptr;
}

void *realloc(void */*ptr*/, size_t /*size*/)
{
	//TODO: Not implemented
	return NULL;
}


void *_malloc_r(struct _reent */*r*/, size_t size)
{
	return foundation_alloc(size);
}

void _free_r(struct _reent */*r*/, void *ptr)
{
	foundation_free(ptr);
}
/* -------------------------------------------------------------- */
//Cpp internals
namespace __gnu_cxx
{

/**
 * \internal
 * Replaces the default verbose terminate handler.
 * Avoids the inclusion of code to demangle C++ names, which saves code size
 * when using exceptions.
 */
void __verbose_terminate_handler()
{
	terminate_process();
}

}//namespace __gnu_cxx

/* -------------------------------------------------------------- */

