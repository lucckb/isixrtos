#ifndef __ISIX_SCHEDULER_H
#define __ISIX_SCHEDULER_H


//Definition of task operations
typedef struct task_field
{
    volatile reg_t *top_stack;
} task_t;



extern task_t * volatile current_task;

void scheduler(void);


reg_t* task_init_stack(reg_t *sp,void (*pfun)(void*),void *param);



#endif
