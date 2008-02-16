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
    {true,1<<16,HZ/8,0},
    {true,1<<17,HZ/4,0},
    {true,1<<18,HZ/2,0},
    {true,1<<19,HZ,0}
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
      for(int i=0;i<1000000;i++) asm volatile("nop");
      if(z->mask == (1<<19))
      {
        z->i++;
        if(z->i==20)
        {
            task_change_prio(t1,4);
            task_change_prio(t2,4);
            //task_change_prio(t3,4);
        }
      }
   }
}
/*-----------------------------------------------------------------------*/
//Main test function
int main(void)
{
   printk("****** Hello from OS ******\n");
   t1 = task_create(simple_task,(void*)&kupa[0],512,10);
   t2 = task_create(simple_task,(void*)&kupa[1],512,10);
   t3 = task_create(simple_task,(void*)&kupa[2],512,10);
   t4 = task_create(simple_task,(void*)&kupa[3],512,10);
   //t5 = task_create(simple_task,(void*)50,400,10);
   //t6 = task_create(simple_task,(void*)60,400,10);
    IO1DIR |= 0xFF<<16;
   return 0;
}

