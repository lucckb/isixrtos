#ifndef PRV_ISIX_SEMAPHORE_H_
#define PRV_ISIX_SEMAPHORE_H_


#include <prv/list.h>
#include <isix/port_atomic.h>
/*--------------------------------------------------------------*/
enum ihandle_type
{
	IHANDLE_T_SEM=1,
	IHANDLE_T_FIFO=2
};

/*--------------------------------------------------------------*/

//Structure of semaphore
struct sem_struct
{
    enum ihandle_type type;
	//Semaphore val
  	_port_atomic_t value; 
    //Task val waiting for semaphore
    list_entry_t sem_task;
	//Atomic true if list is empty
	_port_atomic_int_t sem_task_count;
    //Resource type
    bool static_mem;
};

/*--------------------------------------------------------------*/

//Semaphore can by destroyed
static inline bool _isixp_sem_can_destroy(sem_t *sem)
{
   return list_isempty(&sem->sem_task);
}

/*--------------------------------------------------------------*/

#endif
