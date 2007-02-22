#include <isix/task.h>
#include <isix/semaphore.h>
#include <isix/printk.h>

//Semaphore signalization
sem_t *sem;

/*-----------------------------------------------------------------------*/
TASK_FUNC(wake_task,n)
{
   while(1)
   {
    sem_wait(NULL,HZ*2);
    //for(volatile int i=0;i<1000000;i++);
    printk("++++++++++");
    sem_signal(sem);
    printk("s\n");
   }
}
/*-----------------------------------------------------------------------*/

TASK_FUNC(fun_task,n)
{
    while(1)
    {
        printk("Go to sleep\n");
        sem_wait(sem,NULL);
        printk("***********\n");
    }
}
/*-----------------------------------------------------------------------*/
//Main test function
int main(void)
{
   printk("****** Hello from OS ******\n");
   sem = sem_create(NULL,0);
   task_create(fun_task,NULL,400,10);
   task_create(wake_task,NULL,400,1);
   return 0;
}
