/* ISIX RTOS C++ library runtime */

#if CONFIG_ISIX_WITHOUT_KERNEL
#	include "foundation/sys/tiny_alloc.h"
#else /* CONFIG_ISIX_WITHOUT_KERNEL */
#	include <isix.h>
#	include <isix/prv/semaphore.h>
#endif

#include <cstring>
#include <cstdarg>
#include <cstdint>
#include "foundation/sys/tiny_vaprintf.h"


#define terminate_process() isix_bug(__PRETTY_FUNCTION__)
typedef unsigned long __guard;

// Syscalls definitions
extern "C"
{
	void abort(void);
	void __cxa_pure_virtual();
	void __cxa_deleted_virtual (void);
	int * __errno(void);
	int __cxa_guard_acquire(__guard *);
	void __cxa_guard_release (__guard *);
	void __cxa_guard_abort(__guard *);
}


static bool initializerHasRun(__guard *);
static void setInitializerHasRun(__guard *);
static void setInUse(__guard *);
static void setNotInUse(__guard *);


#if !CONFIG_ISIX_WITHOUT_KERNEL
static struct isix_semaphore ctors_sem;
__attribute__((constructor))
	static void mutex_initializer(void) {
		isix_sem_create(&ctors_sem, 1);
	}

#endif


/* thread safe constructed objects  */
int __cxa_guard_acquire(__guard *guard_object)
{
#if !CONFIG_ISIX_WITHOUT_KERNEL
	// check that the initializer has not already been run
	if (initializerHasRun(guard_object))
		return 0;

	if( isix_is_scheduler_active() )
	if( isix_sem_wait(&ctors_sem,ISIX_TIME_INFINITE) ) {
		terminate_process();
		return 0;
	}

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

#if !CONFIG_ISIX_WITHOUT_KERNEL
	if( isix_is_scheduler_active() )
	if( isix_sem_signal(&ctors_sem )) {
		terminate_process();
	}
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

#if !CONFIG_ISIX_WITHOUT_KERNEL
	if( isix_sem_signal(&ctors_sem) ) {
		terminate_process();
	}
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


__attribute__ ((used))
void abort(void)
{
	terminate_process();
	for(;;);	//Prevent warning
}



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



//Bad function call handler if no exception
#if (__cplusplus > 199711L) && !defined(__EXCEPTIONS) && defined(TODO_FIX_LINK_ORDER)
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
