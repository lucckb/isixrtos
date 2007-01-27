#include "lpc214x.h"
#include "printk.h"
#include "types.h"


#define cpu_save_context()  \
{   \
    extern volatile reg_t current_sp; \
    \
    asm volatile (  \
        /* Save used register R0 */ \
        "STMDB SP!, {R0} \t\n"  \
        /* Save user stack on top of supervisor stack */    \
        "STMDB SP,{SP}^ \t\n"   \
        "NOP \t\n"  \
        "SUB SP,SP,#4 \t\n" \
        /* Restore user stack pointer to R0 */  \
        "LDMIA SP!,{R0} \t\n"   \
        /* Push return adress into R0 */    \
        "STMDB R0!,{LR} \t\n"   \
        /* Use LR as user SP */ \
        "MOV LR,R0 \t\n"    \
        /* Now pop true R0 */   \
        "LDMIA SP!,{R0} \t\n"   \
        /* Push all system register */  \
        "STMDB LR,{R0-LR}^ \t\n"    \
        "NOP \t\n"  \
        "SUB LR,LR,#60 \t\n"    \
        /* Push SPSR on stack*/ \
        "MRS R0,SPSR \t\n"  \
        "STMDB LR!,{R0} \t\n"   \
        "LDR R0, =current_sp \t\n" \
        "STR LR,[R0] \t\n" \
        ); \
}

#define cpu_restore_context(void)   \
{   \
    extern volatile reg_t current_sp; \
    \
    asm volatile  ( \
        "LDR R0, =current_sp \t\n" \
        "LDR LR,[R0]\t\n" \
        /* Restore SPSR */  \
        "LDMIA LR!,{R0} \t\n"   \
        "MSR SPSR,R0 \t\n"  \
        "LDMIA LR,{R0-LR}^\t\n" \
        "NOP \t\n"  \
        /* Load valid return address */ \
        "LDR LR,[LR,#+60] \t\n"  \
        /* Return from SWI */   \
        "SUBS PC,LR,#4 \t\n"    \
    );   \
}

volatile reg_t current_sp;

unsigned char buf1[200];
unsigned char buf2[200];

volatile u32 *sp1,*sp2;

reg_t getsp(void)
{
    static u8 n=0;
    reg_t ret;
    if(n) ret = (reg_t)sp1;
    else ret = (reg_t)sp2;
    n = !n;
    return ret;
}

void cpu_swi_yield(void) __attribute__((interrupt("SWI"),naked));

void cpu_swi_yield(void)
{
    //Add offset to LR according to SWI
    asm volatile("ADD LR,LR,#4 \t\n");
    cpu_save_context();
    current_sp = getsp();
    cpu_restore_context();
}


#define sched_yield() asm volatile("swi #1\t\n")

void fun1(void) __attribute__((noreturn));
void fun1(void) 
{
    while(1)
    {
    for(volatile int i=0;i<1000000;i++);
    printk("Hello from fun1\n");
    sched_yield();
    }
}

void fun2(void) __attribute__((noreturn));
void fun2(void) 
{
    while(1)
    {
    for(volatile int i=0;i<1000000;i++);
    printk("Hello from fun2\n");
    sched_yield();
    }
}



//void cpu_swi_yield(void) __attribute__((interrupt("SWI")));

//void cpu_swi_yield(void) { printk("In native SWI\n"); }


u32* init_stack(volatile u32 *sp,u32 pfun)
{
    *sp-- = pfun + 4;
    printk("init fun=%08x b=%08x\n",pfun,*(sp+1));
    *sp-- = 14; //R14
    *sp-- = 13; //R13
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
    *sp-- = 0;  //R0
    *sp = 0x0000001f;   //SPSR
    return sp;
}

int main(void)
{
	printk_init(UART_BAUD(115200));
	printk("Hello from OS\n");
    sp1 = init_stack((u32*)&buf1[200-4],(u32)fun1);
    sp2 = init_stack((u32*)&buf2[200-4],(u32)fun2);
    current_sp = (reg_t)sp1;
    u32 n = *(u32*)(current_sp+64);
    printk("fun1=%08x current_sp=%08x\n",(u32)fun1,n);
    cpu_restore_context();
	return 0;
}

