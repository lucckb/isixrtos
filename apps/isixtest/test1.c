#include <isix/task.h>
#include <isix/semaphore.h>
#include <isix/time.h>
#include <isix/printk.h>
#include <asm/lpc214x.h>
#include <asm/interrupt.h>

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
        sem_signal(sem);
    }
    kb_prev = IO0PIN & (1<<4);
   }
}
/*-----------------------------------------------------------------------*/

TASK_FUNC(fun_task,n)
{
    IO1DIR |= 0xFF<<16;
    u8 state = 0;
    while(1)
    {
        if(sem_wait(sem,HZ*2)>=0)
        {
            if(!state)
            {
                state = 1;
                IO1SET = 1<<16;
                IO1CLR = 2<<16;
            }
            else
            {
                state = 0;
                IO1SET = 2<<16;
                IO1CLR = 1<<16;
                state = 0;
            }
        }
        else
        {
            IO1PIN ^= 1<<23;
        }
        for(volatile int i=0;i<40000;i++);
    }
}
/*-----------------------------------------------------------------------*/
INTERRUPT_PROC(extint_isr)
{
   sem_signal_isr(sem);
   EXTINT = 0x02;
   interrupt_isr_exit();
}

#define EINT1_SEL (2<<28)
#define P014_SEL_MASK (3<<28)


//Main test function
int main(void)
{
   printk("****** Hello from OS ******\n");
   sem = sem_create(NULL,0);
   t1 = task_create(fun_task,NULL,400,10);
   interrupt_register(INTERRUPT_NUM_EINT1,INTERRUPT_PRIO(14),extint_isr);
   //Przerwanie zboczem
   EXTMODE |= 0x02;
   //Zbocze opadajace
   EXTPOLAR &= ~0x02;
   //Kasuj wystapienie przerwania (1 kasuje)
   EXTINT = 0x02;
   PINSEL0 &= ~P014_SEL_MASK;
   PINSEL0 |= EINT1_SEL;
   t2 = task_create(wake_task,NULL,400,8);
   return 0;
}
