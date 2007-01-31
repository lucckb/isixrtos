#ifndef __ISIX_TASK_H
#define __ISIX_TASK_H



//Definition of task function
#define TASK_FUNC(FUNC, ARG)							\
	void FUNC(void *ARG) __attribute__ ((noreturn));	\
	void FUNC(void *ARG)

static inline void sched_lock(void) {}


static inline void  sched_unlock(void) {}


#endif

