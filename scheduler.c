#include <asm/lpc214x.h>
#include <isix/printk.h>
#include <asm/types.h>


#define NULL (0)

typedef struct task_field
{
    volatile reg_t *top_stack;
    volatile reg_t *init_stack;
} task_s;


task_s * volatile current_task = NULL;


#define cpu_save_context()                                      \
    asm volatile (                                              \
        /* Save used register R0 */                             \
        "STMDB SP!, {R0} \t\n"                                  \
        /* Save user stack on top of supervisor stack */        \
        "STMDB SP,{SP}^ \t\n"                                   \
        "NOP \t\n"                                              \
        "SUB SP,SP,#4 \t\n"                                     \
        /* Restore user stack pointer to R0 */                  \
        "LDMIA SP!,{R0} \t\n"                                   \
        /* Push return adress into R0 */                        \
        "STMDB R0!,{LR} \t\n"                                   \
        /* Use LR as user SP */                                 \
        "MOV LR,R0 \t\n"                                        \
        /* Now pop true R0 */                                   \
        "LDMIA SP!,{R0} \t\n"                                   \
        /* Push all system register */                          \
        "STMDB LR,{R0-LR}^ \t\n"                                \
        "NOP \t\n"                                              \
        "SUB LR,LR,#60 \t\n"                                    \
        /* Push SPSR on stack*/                                 \
        "MRS R0,SPSR \t\n"                                      \
        "STMDB LR!,{R0} \t\n"                                   \
        "LDR R0,=current_task \t\n"                             \
        "LDR R0,[R0] \t\n"                                      \
        "STR LR,[R0] \t\n" )    

#define cpu_restore_context()                                   \
    asm volatile  (                                             \
        "LDR R0,=current_task\t\n"                              \
        "LDR R0,[R0] \t\n"                                      \
        "LDR LR,[R0] \t\n"                                      \
        /* Restore SPSR */                                      \
        "LDMIA LR!,{R0} \t\n"                                   \
        "MSR SPSR,R0 \t\n"                                      \
        "LDMIA LR,{R0-LR}^\t\n"                                 \
        "NOP \t\n"                                              \
        /* Load valid return address */                         \
        "LDR LR,[LR,#+60] \t\n"                                 \
        /* Return from SWI */                                   \
        "SUBS PC,LR,#4 \t\n" )



unsigned char stk1[200];
unsigned char stk2[200];

task_s task1,task2;



void scheduler(void)
{
    static u8 n=0;
    if(n) current_task = &task1;
    else current_task = &task2;
    n = !n;
}

void cpu_swi_yield(void) __attribute__((interrupt("SWI"),naked));

void cpu_swi_yield(void)
{
    //Add offset to LR according to SWI
    asm volatile("ADD LR,LR,#4 \t\n");
    cpu_save_context();
    scheduler();
    //printk("SWI CURR_SP=%08x\n",current_task->top_stack);
    cpu_restore_context();
}


#define sched_yield() asm volatile("swi #1\t\n")

void fun1(void *n) __attribute__((noreturn));
void fun1(void *n) 
{
    const register reg_t tmp asm("r13");
    printk("func1(%08x)\n",(u32)n);
    while(1)
    {
    for(volatile int i=0;i<1000000;i++);
    printk("Hello from fun1 SP=%08x\n",tmp);
    sched_yield();
    }
}

void fun2(void *n) __attribute__((noreturn));
void fun2(void *n) 
{
   const register reg_t tmp asm("r13");
   printk("func2(%08x)\n",(u32)n);

   while(1)
    {
    for(volatile int i=0;i<1000000;i++);
    printk("Hello from fun2 SP=%08x\n",tmp);
    sched_yield();
    }
}



reg_t* init_stack(reg_t *sp,void (*pfun)(void*),void *param)
{
     reg_t *orig_sp = sp;
    *sp-- = (u32)pfun + 4;
    *sp-- = 14; //R14
    *sp-- = (u32)orig_sp; //R13
    *sp-- = 12;  //R12
    *sp-- = 11; //R11
    *sp-- = 10; //R10
    *sp-- = 9; //R9
    *sp-- = 8; //R8
    *sp-- = 7 ; //R7
    *sp-- = 6;   //R6
    *sp-- = 5;  //R5
    *sp-- = 4; //R4
    *sp-- = 3;  //R3
    *sp-- = 2;  //R2
    *sp-- = 1;  //R1
    *sp-- = (u32)param;  //R0
    *sp = 0x0000001f;   //SPSR
    return sp;
}

void init_os(void) __attribute__((noreturn));

void init_os(void)
{
	printk_init(UART_BAUD(115200));
	printk("Hello from OS\n");
    task1.top_stack = (reg_t*)&stk1[200-4];
    task2.top_stack = (reg_t*)&stk2[200-4];
    task1.init_stack = (reg_t*)stk1;
    task2.init_stack = (reg_t*)stk2;
    task1.top_stack = init_stack(task1.top_stack,fun1,(void*)0x10203040);
    task2.top_stack = init_stack(task2.top_stack,fun2,(void*)0xaabbccdd);
    printk("sp1=%08x sp2=%08x\n",(u32)task1.top_stack,(u32)task2.top_stack);
    current_task = &task1;
    cpu_restore_context();
}

int main(void)
{
    return 0;
}   

