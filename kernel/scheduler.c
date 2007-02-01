#include <isix/config.h>
#include <isix/printk.h>
#include <isix/types.h>
#include <isix/scheduler.h>
#include <asm/context.h>
#include <isix/task.h>
#include <isix/memory.h>

task_t * volatile current_task = NULL;




unsigned char stk1[200];
unsigned char stk2[200];

task_t task1,task2;



void scheduler(void)
{
    static u8 n=0;
    if(n) current_task = &task1;
    else current_task = &task2;
    n = !n;
}


void *x[10];

TASK_FUNC(fun1,n)
{
    const register reg_t tmp asm("r13");
    printk("func1(%08x)\n",(u32)n);
    while(1)
    {
    for(volatile int i=0;i<1000000;i++);
    printk("Hello from fun1 SP=%08x\n",tmp);
    cpu_yield();
    }
}

TASK_FUNC(fun2,n) 
{
   const register reg_t tmp asm("r13");
   printk("func2(%08x)\n",(u32)n);
   
   while(1)
    {
    for(volatile int i=0;i<1000000;i++);
    printk("Hello from fun2 SP=%08x\n",tmp);
    cpu_yield();
    }
}

void init_os(void) __attribute__((noreturn));

void init_os(void)
{
	printk_init(UART_BAUD(115200));
	printk("Hello from OSn\n");
    task1.top_stack = (reg_t*)&stk1[200-4];
    task2.top_stack = (reg_t*)&stk2[200-4];
    task1.top_stack = task_init_stack(task1.top_stack,fun1,(void*)0x10203040);
    task2.top_stack = task_init_stack(task2.top_stack,fun2,(void*)0xaabbccdd);
    printk("sp1=%08x sp2=%08x\n",(u32)task1.top_stack,(u32)task2.top_stack);
    current_task = &task1;
    cpu_restore_context();
}

int main(void)
{
    return 0;
}   

