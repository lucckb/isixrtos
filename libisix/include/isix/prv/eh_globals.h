#pragma once

#ifdef __cplusplus
extern "C" {
#endif

//! Layout of __cxa_eh_globals of libsupc++ for the ARM EABI unwinder
struct isix_eh_globals
{
	void *caught_exceptions;
	unsigned int uncaught_exceptions;
	void *propagating_exceptions;
};

//! EH state of the running task provided by the kernel, NULL outside of a task
struct isix_eh_globals* _isixp_task_eh_slot(void);

#ifdef __cplusplus
}
#endif
