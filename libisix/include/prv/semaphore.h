#ifndef PRV_ISIX_SEMAPHORE_H_
#define PRV_ISIX_SEMAPHORE_H_


#include <prv/list.h>

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
    int value;
    //Task val waiting for semaphore
    list_entry_t sem_task;
    //Resource type
    bool static_mem;
    //Semaphore limit value
    int limit_value;
};

/*--------------------------------------------------------------*/

//Semaphore can by destroyed
static inline bool isixp_sem_can_destroy(sem_t *sem)
{
   return list_isempty(&sem->sem_task);
}

/*--------------------------------------------------------------*/

#endif