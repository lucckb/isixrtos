#ifndef __ASM_PRV_CONTEXT_H
#define __ASM_PRV_CONTEXT_H

#include <isix/config.h>

/* 
    Switch and restore CPU context for ARM7TDMI 
    ISIX os (c) L. Bryndza 2007
*/

/*-----------------------------------------------------------------------*/

#define cpu_save_context()										\
    asm volatile (												\
    "mrs r0, psp\t\n"					        				\
    "stmdb r0!, {r4-r11}\t\n"                                   \
    "ldr r3,0f\t\n"                                   			\
    "ldr r2,[r3]\t\n"                                           \
    "str r0, [r2]\t\n"                                          \
    "stmdb sp!, {r3,r14}\t\n"                                   \
    "mov r0,%0\t\n"                                             \
    "msr basepri,r0\t\n"                                        \
    ::"i"(configMAX_SYSCALL_INTERRUPT_PRIORITY)                 \
	)
/*-----------------------------------------------------------------------*/

#define cpu_restore_context()                                   \
    asm volatile  (                                             \
    "mov r0,#0\t\n"                                             \
    "msr basepri,r0\t\n"                                        \
	"ldmia sp!, {r3,r14}\t\n"                                   \
    "ldr r1,[r3]\t\n"                                           \
    "ldr r0, [r1]\t\n"                                          \
    "ldmia r0!, {r4-r11}\t\n"                                   \
    "msr psp, r0\t\n"                                           \
    "bx r14\r\n"                                                \
    ".align 2 \t\n"												\
    "0: .word current_task\t\n"									\
   )
/*-----------------------------------------------------------------------*/

#endif
