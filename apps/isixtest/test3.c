#include <isix/task.h>
#include <isix/semaphore.h>
#include <isix/time.h>
#include <isix/printk.h>
#include <asm/lpc214x.h>
#include <asm/interrupt.h>
#include <isix/fifo.h>

//Semaphore signalization
task_t *t1,*t2,*t3,*t4,*t5,*t6;
/*-----------------------------------------------------------------------*/
TASK_FUNC(simple_task,n)
{
   while(1)
   {
      printk("Hello from task %d\n",n);
      schedule_timeout(HZ);
   }

}
/*-----------------------------------------------------------------------*/
//Main test function
int main(void)
{
   printk("****** Hello from OS ******\n");
   t1 = task_create(simple_task,(void*)10,400,10);
   t2 = task_create(simple_task,(void*)20,400,10);
   t3 = task_create(simple_task,(void*)30,400,10);
   t4 = task_create(simple_task,(void*)40,400,10);
   t5 = task_create(simple_task,(void*)50,400,10);
   t6 = task_create(simple_task,(void*)60,400,10);
   return 0;
}
