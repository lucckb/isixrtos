#include <isix/task.h>
#include <isix/semaphore.h>
#include <isix/time.h>
#include <isix/printk.h>
#include <asm/lpc214x.h>
#include <asm/interrupt.h>
#include <isix/fifo.h>

//Semaphore signalization
task_t *t1,*t2,*t3;
fifo_t *fifo;
/*-----------------------------------------------------------------------*/
TASK_FUNC(wake_task,n)
{
   u32 kb_prev = 1<<4;
   u8 cnt = 0;
   while(1)
   {
    schedule_timeout(HZ/10);
    if(kb_prev && !(IO0PIN & (1<<4)) )
    {
        fifo_write(fifo,&cnt,0);
        if(cnt++>=0x80) cnt = 0;
        printk("Microseconds %d\n",get_usec());
    }
    kb_prev = IO0PIN & (1<<4);
   }

}
/*-----------------------------------------------------------------------*/

TASK_FUNC(fun_task,n)
{
    IO1DIR |= 0xFF<<16;
    u8 ch;
    while(1)
    {
        if(IO0PIN & (1<<5)) {schedule_timeout(HZ/10); continue; }
        if(fifo_read(fifo,&ch,HZ*2)>=0)
        {
            IO1SET = (u32)ch << 16;
            IO1CLR = ~((u32)ch << 16);
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
   char c = 0x55;
   fifo_write_isr(fifo,&c);
   EXTINT = 0x02;
   interrupt_isr_exit();
}

#define EINT1_SEL (2<<28)
#define P014_SEL_MASK (3<<28)

/*-----------------------------------------------------------------------*/
//Main test function
int main(void)
{
   printk("****** Hello from OS ******\n");

   interrupt_register(INTERRUPT_NUM_EINT1,INTERRUPT_PRIO(14),extint_isr);
   //Przerwanie zboczem
   EXTMODE |= 0x02;
   //Zbocze opadajace
   EXTPOLAR &= ~0x02;
   //Kasuj wystapienie przerwania (1 kasuje)
   EXTINT = 0x02;
   PINSEL0 &= ~P014_SEL_MASK;
   PINSEL0 |= EINT1_SEL;

   t1 = task_create(fun_task,NULL,400,10);
   t2 = task_create(wake_task,NULL,400,8);
   fifo = fifo_create(10,1);
   return 0;
}
