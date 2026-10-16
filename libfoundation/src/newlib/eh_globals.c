/*
 * Per task C++ exception handling state: replaces the single global state of
 * libsupc++ so that exceptions are safe in many tasks.
 *
 * Author: Lucjan Bryndza
 */
#include <isix/config.h>
#include <isix/prv/eh_globals.h>

#ifdef CONFIG_ISIX_TASK_EH_GLOBALS

//! State used outside of a task: before the scheduler start and in ISRs
static struct isix_eh_globals g_boot_eh_globals;

//! Overrides __cxa_get_globals() and __cxa_get_globals_fast() of libsupc++
struct isix_eh_globals* __cxa_get_globals_fast(void);
struct isix_eh_globals* __cxa_get_globals(void);

struct isix_eh_globals* __cxa_get_globals_fast(void)
{
	struct isix_eh_globals* const slot = _isixp_task_eh_slot();
	return slot ? slot : &g_boot_eh_globals;
}

struct isix_eh_globals* __cxa_get_globals(void)
{
	return __cxa_get_globals_fast();
}

#endif
