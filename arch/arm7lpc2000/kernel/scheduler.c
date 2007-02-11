#include <isix/config.h>
#include <isix/printk.h>
#include <isix/types.h>
#include <asm/context.h>
#include <isix/scheduler.h>


//System Mode enable IRQ and FIQ
#define INITIAL_SPSR 0x0000001f


/*-----------------------------------------------------------------------*/
//Yield processor
void cpu_swi_yield(void) __attribute__((interrupt("SWI"),naked));

void cpu_swi_yield(void)
{
    //Add offset to LR according to SWI
    asm volatile("ADD LR,LR,#4 \t\n");
    cpu_save_context();
    scheduler();
    cpu_restore_context();
}

/*-----------------------------------------------------------------------*/
//Create of stack context 
reg_t* task_init_stack(reg_t *sp,task_func_ptr_t pfun,void *param)
{
     reg_t *orig_sp = sp;
    *sp-- = (reg_t)pfun + 4;
    *sp-- = 14; //R14
    *sp-- = (reg_t)orig_sp; //R13
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
    *sp-- = (reg_t)param;  //R0
    *sp = INITIAL_SPSR;   //SPSR
    return sp;
}

/*-----------------------------------------------------------------------*/
