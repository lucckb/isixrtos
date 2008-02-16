#include <isix/task.h>
#include <isix/semaphore.h>
#include <isix/time.h>
#include <isix/printk.h>
#include <asm/lpc214x.h>
#include <asm/interrupt.h>
#include <isix/fifo.h>

//Semaphore signalization
task_t *t1,*t2,*t3,*t4,*t5,*t6;

typedef struct dupa
{
    bool on;
    u32 mask;
    u32 sec;
    u32 i;
} dupa;

dupa kupa[6] =
{
    {true,1<<16,HZ*5},
    {true,1<<17,HZ*5},
    {true,1<<18,HZ*5},
    {true,1<<19,HZ*5}
};

/*-----------------------------------------------------------------------*/
TASK_FUNC(simple_task,n)
{
   dupa *z = (dupa*)n;
   while(1)
   {
      printf("Hello from task %08x\n",z->mask);
      //schedule_timeout(z->sec);
      if(z->on) IO1SET = z->mask;
      else      IO1CLR = z->mask;
      z->on = !z->on;
      for(volatile int i=0;i<100000;i++);
      if(z->mask == (1<<19))
      {
        z->i++;
        if(z->i==50) task_change_prio(t1,4);
      }
   }
}
/*-----------------------------------------------------------------------*/
//Main test function
int main(void)
{
   printk("****** Hello from OS ******\n");
   t1 = task_create(simple_task,(void*)&kupa[0],400,10);
   t2 = task_create(simple_task,(void*)&kupa[1],400,10);
   t3 = task_create(simple_task,(void*)&kupa[2],400,10);
   t4 = task_create(simple_task,(void*)&kupa[3],400,10);
   //t5 = task_create(simple_task,(void*)50,400,10);
   //t6 = task_create(simple_task,(void*)60,400,10);
    IO1DIR |= 0xFF<<16;
   return 0;
}

