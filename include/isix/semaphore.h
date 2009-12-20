#ifndef __ISIX_SEMAPHORE_H
#define __ISIX_SEMAPHORE_H

#include <isix/config.h>
#include <isix/types.h>

/*--------------------------------------------------------------*/
#ifdef __cplusplus
extern "C" {
namespace isix {
#endif /*__cplusplus*/
/*--------------------------------------------------------------*/

struct sem_struct;
typedef struct sem_struct sem_t;
/*--------------------------------------------------------------*/
#ifdef __cplusplus
#else
static const unsigned ISIX_TIME_INFINITE = 0;
#define ISIX_TIME_INFINITE (0)
#endif /*__cplusplus*/
/*--------------------------------------------------------------*/
//Create semaphore
sem_t* isix_sem_create(sem_t *sem,int val);


/*--------------------------------------------------------------*/
//Wait for semaphore P()
int isix_sem_wait(sem_t *sem, tick_t timeout);

/*--------------------------------------------------------------*/
//Get semaphore from isr
int isix_sem_get_isr(sem_t *sem);

/*--------------------------------------------------------------*/
//Sem signal V()
int isixp_sem_signal(sem_t *sem, bool isr);

/*--------------------------------------------------------------*/
//Definition of sem and sem ISR
static inline int isix_sem_signal(sem_t *sem)
{
	return isixp_sem_signal( sem, false );
}

/*--------------------------------------------------------------*/
static inline int isix_sem_signal_isr(sem_t *sem)
{
	return isixp_sem_signal(sem,true);
}

/*--------------------------------------------------------------*/
//Sem value of semaphore
int isix_sem_setval(sem_t *sem,int val);

/*--------------------------------------------------------------*/
//Get value of semaphore
int isix_sem_getval(sem_t *sem);

/*--------------------------------------------------------------*/
//Sem destroy
int isix_sem_destroy(sem_t *sem);

/*--------------------------------------------------------------*/
//! Convert ms to ticks
tick_t isix_ms2tick(unsigned long ms);

/*--------------------------------------------------------------*/

//Wait for n jiffies
static inline int isix_wait(tick_t timeout)
{
	return isix_sem_wait(NULL,timeout);
}

/*-----------------------------------------------------------*/
#ifdef __cplusplus
}	//end namespace
}	//end extern-C
#endif /* __cplusplus */

/*-----------------------------------------------------------*/
//****************************** C++ API ***********************************
#ifdef __cplusplus
#include <cstddef>

namespace isix {
/*--------------------------------------------------------------*/
class semaphore
{
public:
	//Constructor
	explicit semaphore(int val)
	{
		sem = isix_sem_create(NULL,val);
	}
	//Destructor
	~semaphore()
	{
		isix_sem_destroy(sem);
	}
	//Check the fifo object is in valid state
	bool is_valid() { return sem!=0; }
	//Wait for sem
	int wait(tick_t timeout)
	{
		return isix_sem_wait( sem, timeout );
	}
	//Get from isr
	int get_isr()
	{
		return isix_sem_get_isr(sem);
	}
	//Signal
	int signal()
	{
		return isix_sem_signal(sem);
	}
	//Signal from ISR
	int signal_isr()
	{
		return isix_sem_signal_isr(sem);
	}
	//SetVal
	int setval(int val)
	{
		return isix_sem_setval( sem, val );
	}
	//GetVal
	int getval()
	{
		return isix_sem_getval( sem );
	}

private:
	sem_t *sem;
};


/*--------------------------------------------------------------*/
}
#endif /* __cplusplus */

/*-----------------------------------------------------------*/
#endif /* __ISIX_SEMAPHORE_H */
