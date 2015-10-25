/* ------------------------------------------------------------ */
/*
 *    tiny_alloc.cpp
 *
 *     Created on: 2009-08-08
 *     Author: lucck
 *
 * ------------------------------------------------------------ */
#ifndef COMPILED_UNDER_ISIX
#include "foundation/tiny_alloc.h"

#else /*COMPILED_UNDER_ISIX*/
#include <isix.h>
#endif
/* -------------------------------------------------------------- */
#include <cstring>
#include <cstdarg>
#include <cstdint>
#include "foundation/tiny_vaprintf.h"
/* -------------------------------------------------------------- */

#ifndef COMPILED_UNDER_ISIX

#ifndef CONFIG_FOUNDATION_NO_DYNAMIC_ALLOCATION
//It can be redefined
#define foundation_alloc fnd::tiny_alloc
#define foundation_free fnd::tiny_free
#else /*CONFIG_FOUNDATION_NO_DYNAMIC_ALLOCATION */
#define foundation_alloc(x) ((x)?NULL:NULL)
#define foundation_free(x) do { (void)(x); } while(0)
#endif /*CONFIG_FOUNDATION_NO_DYNAMIC_ALLOCATION */

#define terminate_process() while(1)

#else /*COMPILED_UNDER_ISIX*/

#define foundation_alloc isix_alloc
#define foundation_free isix_free
#define terminate_process() isix_bug(__PRETTY_FUNCTION__)
#endif /*COMPILED_UNDER_ISIX*/

/* -------------------------------------------------------------- */
#ifndef  __EXCEPTIONS
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

typedef unsigned long __guard;

/* -------------------------------------------------------------- */
// Syscalls definitions
extern "C"
{
	void *_malloc_r(struct _reent */*r*/, size_t size);
	void _free_r(struct _reent */*r*/, void *ptr);
	void* malloc(size_t size) __attribute__((used));
	void free(void *ptr) __attribute__((used));
	void *calloc(size_t nmemb, size_t size) __attribute__((used));
	void *realloc(void */*ptr*/, size_t /*size*/);
	void abort(void);
	void __cxa_pure_virtual();
	void __cxa_deleted_virtual (void);
	int * __errno(void);
	int __cxa_guard_acquire(__guard *);
	void __cxa_guard_release (__guard *);
	void __cxa_guard_abort(__guard *);
}
/* -------------------------------------------------------------- */

static bool initializerHasRun(__guard *);
static void setInitializerHasRun(__guard *);
static void setInUse(__guard *);
static void setNotInUse(__guard *);

/* -------------------------------------------------------------- */
#ifdef COMPILED_UNDER_ISIX
static ossem_t ctors_sem;
#endif
/* -------------------------------------------------------------- */
#ifdef CPP_STARTUP_CODE

/* thread safe constructed objects  */
int __cxa_guard_acquire(__guard *guard_object)
{
#ifdef COMPILED_UNDER_ISIX

	// check that the initializer has not already been run
	if (initializerHasRun(guard_object))
		return 0;

	//  create semaphore if not existing
	if(ctors_sem == NULL)
	{
		ctors_sem = isix_sem_create(NULL, 1);

		if(ctors_sem == NULL)
		{
			//Remove task if can't create semaphore
			terminate_process();
			return 0;
		}
	}

	if(ctors_sem)
		isix_sem_wait(ctors_sem, ISIX_TIME_INFINITE);

	setInUse(guard_object);
	return 1;
#else
	if (initializerHasRun(guard_object))
		return 0;

	setInUse(guard_object);
	return 1;
#endif
}

void __cxa_guard_release(__guard *guard_object)
{
    setInitializerHasRun(guard_object);

#ifdef COMPILED_UNDER_ISIX
	if(ctors_sem)
		isix_sem_signal(ctors_sem);
#endif
}

//Pure virtual call error handler
void __cxa_pure_virtual()
{
	terminate_process();
}

//Deleted virtual
void __cxa_deleted_virtual (void) 
{
	terminate_process();
}

void __cxa_guard_abort(__guard *guard_object)
{
	setNotInUse(guard_object);

#ifdef COMPILED_UNDER_ISIX
	if(ctors_sem)
		isix_sem_signal(ctors_sem);
#endif
}

static bool initializerHasRun(__guard *guard_object)
{
    return ( *((uint8_t*)guard_object) != 0 );
}

static void setInitializerHasRun(__guard *guard_object)
{
    *((uint8_t*)guard_object) = 1;
}

static void setInUse(__guard *guard_object)
{
    ((uint8_t*)guard_object)[1] = 1;
}

static void setNotInUse(__guard *guard_object)
{
    ((uint8_t*)guard_object)[1] = 0;
}

#endif /*CPP_STARTUP_CODE*/

/* ------------------------------------------------------------------ */

__attribute__ ((used))
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
int __snprintf_lite(char *__buf, size_t __bufsize, const char *__fmt, va_list __ap)
{
	return fnd::tiny_vaprintf( &__buf, __bufsize, __fmt, __ap );
}
}//namespace __gnu_cxx


/* -------------------------------------------------------------- */
//Bad function call handler if no exception
#if (__cplusplus > 199711L) && !defined(__EXCEPTIONS) 
namespace std
{
	  void
	  __throw_bad_exception()
	  { terminate_process(); for(;;); }

	  void
	  __throw_bad_alloc()
	  { terminate_process(); for(;;); }

	  void
	  __throw_bad_cast()
	  { terminate_process(); for(;;); }

	  void
	  __throw_bad_typeid()
	  { terminate_process(); for(;;); }

	  void
	  __throw_logic_error(const char* __s __attribute__((unused)))
	  { terminate_process(); for(;;); }

	  void
	  __throw_domain_error(const char* __s __attribute__((unused)))
	  { terminate_process(); for(;;); }

	  void
	  __throw_invalid_argument(const char* __s __attribute__((unused)))
	  { terminate_process(); for(;;); }

	  void
	  __throw_length_error(const char* __s __attribute__((unused)))
	  { terminate_process(); for(;;); }

	  void
	  __throw_out_of_range(const char* __s __attribute__((unused)))
	  { terminate_process(); for(;;); }

	  void
	  __throw_runtime_error(const char* __s __attribute__((unused)))
	  { terminate_process(); for(;;); }

	  void
	  __throw_range_error(const char* __s __attribute__((unused)))
	  { terminate_process(); for(;;); }

	  void
	  __throw_overflow_error(const char* __s __attribute__((unused)))
	  { terminate_process(); for(;;); }

	  void
	  __throw_underflow_error(const char* __s __attribute__((unused)))
	  { terminate_process(); for(;;); }

	  void
	  __throw_ios_failure(const char* __s __attribute__((unused)))
	  { terminate_process(); for(;;); }

	  void
	  __throw_system_error(int __i __attribute__((unused)))
	  { terminate_process(); for(;;); }

	  void
	  __throw_future_error(int __i __attribute__((unused)))
	  { terminate_process(); for(;;); }

	  void
	  __throw_bad_function_call()
	  { terminate_process(); for(;;); }

	  void
	  __throw_regex_error()
	  { terminate_process(); for(;;); }

	void
	__throw_out_of_range_fmt(const char* , ...)
	  { terminate_process(); for(;;); }
}
#endif
/* -------------------------------------------------------------- */
