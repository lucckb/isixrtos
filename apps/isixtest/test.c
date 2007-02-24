#include <isix/task.h>
#include <isix/semaphore.h>
#include <isix/time.h>
#include <isix/printk.h>
#include <asm/lpc214x.h>

//Semaphore signalization
sem_t *sem;
task_t *t1,*t2,*t3;
/*-----------------------------------------------------------------------*/
TASK_FUNC(wake_task,n)
{
   u32 kb_prev = 1<<4;
   while(1)
   {
    schedule_timeout(HZ/10);
    if(kb_prev && !(IO0PIN & (1<<4)) )
    {
        printk("++++++++++\n");
        sem_signal(sem);
    }
    kb_prev = IO0PIN & (1<<4);
   }
}
/*-----------------------------------------------------------------------*/

TASK_FUNC(fun_task,n)
{
    while(1)
    {
        printk("Go to sleep\n");
        sem_wait(sem,0);
        printk("***********\n");
        if(t3) { task_delete(t3); t3=NULL;}
    }
}
/*-----------------------------------------------------------------------*/


TASK_FUNC(dupa_task,n)
{
    while(1) {
         schedule_timeout(HZ);
        //for(volatile int i=0;i<1000000;i++);
        printk("$\n");
    }
}

//Main test function
int main(void)
{
   printk("****** Hello from OS ******\n");
   sem = sem_create(NULL,0);
   t1 = task_create(fun_task,NULL,400,10);
   t2 = task_create(wake_task,NULL,400,8);
   t3 = task_create(dupa_task,NULL,128+32,22);
   return 0;
}
