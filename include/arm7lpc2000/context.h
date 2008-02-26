#ifndef __ASM_PRV_CONTEXT_H
#define __ASM_PRV_CONTEXT_H

/* 
    Switch and restore CPU context for ARM7TDMI 
    ISIX os (c) L. Bryndza 2007
*/

/*-----------------------------------------------------------------------*/

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

/*-----------------------------------------------------------------------*/


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

/*-----------------------------------------------------------------------*/

#endif
