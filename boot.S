//	Plik startowy dla mikrokontrolerow LPC214x

/****************** KONFIGURACJA *********************/

/*Ustawienia stosu*/ 
#define Top_Stack      		0x40004000
#define UND_Stack_Size 		0x00000004
#define SVC_Stack_Size 		0x00000004
#define ABT_Stack_Size 		0x00000004
#define FIQ_Stack_Size		0x00000004
#define IRQ_Stack_Size		0x00000100
#define USR_Stack_Size		0x00000200

/* Konfiguracja VPBDIV 	*/
#define VPBDIV_SETUP		
#define VPBDIV_Val    		0x00000001

/* Konfiguracja petli PLL */
#define PLL_SETUP
#define PLLCFG_Val		0x00000024	

//Konfiguracja MAM
#define MAM_SETUP
#define MAMCR_Val		0x00000002
#define MAMTIM_Val		0x00000003


//Konfiguracja zewnetrznego kontrolera pamieci
#undef EMC_SETUP

//Konfiguracja banku 0
#define BCFG0_SETUP
#define BCFG0_Val		0x0000FBEF

//Konfiguracja banku 1
#define BCFG1_SETUP
#define BCFG1_Val		0x0000FBEF

//Konfiguracja banku 2
#define BCFG2_SETUP
#define BCFG2_Val		0x0000FBEF

//Konfiguracja banku 3
#define BCFG3_SETUP
#define BCFG3_Val		0x0000FBEF

#undef EXTERNAL_MODE
#define PINSEL2_Val	0x0E6149E4  

//Wektory w pamieci RAM
#undef RAM_INTVEC
	
/****************** PLIK STARTOWY *********************/

// Definicje trybow ochrony
#define Mode_USR       0x10
#define Mode_FIQ       0x11
#define Mode_IRQ       0x12
#define Mode_SVC       0x13
#define Mode_ABT       0x17
#define Mode_UND       0x1B
#define Mode_SYS       0x1F

#define I_Bit          0x80    /* when I bit is set, IRQ is disabled */
#define F_Bit          0x40    /* when F bit is set, FIQ is disabled */


// VPBDIV podzielnik dla PClk
#define VPBDIV         		0xE01FC100


// Definicje dla petli PLL
#define PLL_BASE 	        0xE01FC080  /* PLL Base Address */
#define PLLCON_OFS	     	0x00        /* PLL Control Offset*/
#define PLLCFG_OFS   		0x04        /* PLL Configuration Offset */
#define PLLSTAT_OFS		0x08        /* PLL Status Offset */
#define PLLFEED_OFS		0x0C        /* PLL Feed Offset */
#define PLLCON_PLLE    		(1<<0)      /* PLL Enable */
#define PLLCON_PLLC    		(1<<1)      /* PLL Connect */
#define PLLCFG_MSEL	    	(0x1F<<0)   /* PLL Multiplier */
#define PLLCFG_PSEL		(0x03<<5)   /* PLL Divider */
#define PLLSTAT_PLOCK		(1<<10)     /* PLL Lock Status */



// Definicje modulu MAM
#define MAM_BASE		0xE01FC000  /* MAM Base Address */
#define MAMCR_OFS		0x00        /* MAM Control Offset*/
#define MAMTIM_OFS     		0x04        /* MAM Timing Offset */



// Rejestry zewnetrznego kontrolera pamieci
#define EMC_BASE		0xFFE00000  /* EMC Base Address */
#define BCFG0_OFS		0x00        /* BCFG0 Offset */
#define BCFG1_OFS		0x04        /* BCFG1 Offset */
#define BCFG2_OFS		0x08        /* BCFG2 Offset */
#define BCFG3_OFS		0x0C        /* BCFG3 Offset */

// Piny pamiecie zewnetrznej
#define EXTERNAL_MODE
#define	PINSEL2		0xE002C014  /* PINSEL2 Address */



// Kod startowy musi byc zlinkowany na poczatku
        .text
        .arm	
        .global _startup
        .func   _startup
_startup:
	

//Wektory wyjatkow

Vectors:        LDR     PC, Reset_Addr         
                LDR     PC, Undef_Addr
                LDR     PC, SWI_Addr
                LDR     PC, PAbt_Addr
                LDR     PC, DAbt_Addr
                NOP                            /* Reserved Vector */
//               LDR     PC, IRQ_Addr
                LDR     PC, [PC, #-0x0FF0]     /* Vector from VicVectAddr */
                LDR     PC, FIQ_Addr

Reset_Addr:     .word   Reset_Handler
Undef_Addr:     .word   Undef_Handler
SWI_Addr:       .word   SWI_Handler
PAbt_Addr:      .word   PAbt_Handler
DAbt_Addr:      .word   DAbt_Handler
                .word   0                      /* Reserved Address */
IRQ_Addr:       .word   IRQ_Handler
FIQ_Addr:       .word   FIQ_Handler

Undef_Handler:  B       Undef_Handler
SWI_Handler:    B       SWI_Handler
PAbt_Handler:   B       PAbt_Handler
DAbt_Handler:   B       DAbt_Handler
IRQ_Handler:    B       IRQ_Handler
FIQ_Handler:    B       FIQ_Handler


// Wektor reset

Reset_Handler:  


#ifdef EXTERNAL_MODE
                LDR     R0, =PINSEL2
                LDR     R1, =PINSEL2_Val
                STR     R1, [R0]
#endif /*EXTERNAL_MODE*/


#ifdef EMC_SETUP
                LDR     R0, =EMC_BASE

#ifdef BCFG0_SETUP
                LDR     R1, =BCFG0_Val
                STR     R1, [R0, #BCFG0_OFS]
#endif /*BCFG0_SETUP*/

#ifdef BCFG1_SETUP
                LDR     R1, =BCFG1_Val
                STR     R1, [R0, #BCFG1_OFS]
#endif /*BCFG1_SETUP*/

#ifdef BCFG2_SETUP
                LDR     R1, =BCFG2_Val
                STR     R1, [R0, #BCFG2_OFS]
#endif /*BCFG2_SETUP*/

#ifdef BCFG3_SETUP
                LDR     R1, =BCFG3_Val
                STR     R1, [R0, #BCFG3_OFS]
#endif /*BCFG3_SETUP*/

#endif /*EMC_SETUP*/


#ifdef VPBDIV_SETUP
                LDR     R0, =VPBDIV
                LDR     R1, =VPBDIV_Val
                STR     R1, [R0]
#endif


#ifdef PLL_SETUP
                LDR     R0, =PLL_BASE
                MOV     R1, #0xAA
                MOV     R2, #0x55

// Configure and Enable PLL
                MOV     R3, #PLLCFG_Val
                STR     R3, [R0, #PLLCFG_OFS] 
                MOV     R3, #PLLCON_PLLE
                STR     R3, [R0, #PLLCON_OFS]
                STR     R1, [R0, #PLLFEED_OFS]
                STR     R2, [R0, #PLLFEED_OFS]

// Czekaj na synchronizacje PLL
PLL_Loop:       LDR     R3, [R0, #PLLSTAT_OFS]
                ANDS    R3, R3, #PLLSTAT_PLOCK
                BEQ     PLL_Loop

// Wlacz zegar z PLL
                MOV     R3, #(PLLCON_PLLE | PLLCON_PLLC)
                STR     R3, [R0, #PLLCON_OFS]
                STR     R1, [R0, #PLLFEED_OFS]
                STR     R2, [R0, #PLLFEED_OFS]
#endif


#ifdef MAM_SETUP
                LDR     R0, =MAM_BASE
                MOV     R1, #MAMTIM_Val
                STR     R1, [R0, #MAMTIM_OFS] 
                MOV     R1, #MAMCR_Val
                STR     R1, [R0, #MAMCR_OFS] 
#endif


// Wektor MEMMAP
#define MEMMAP		0xE01FC040  /* Memory Mapping Control */
        
#ifdef RAM_INTVEC
                LDR     R0, =MEMMAP
                MOV     R1, #2
                STR     R1, [R0]
#endif


// Inicjalizacja systemu przerwan



// Ustaw stos dla kazdego trybu

                LDR     R0, =Top_Stack

//  Stos dla trybu Undefined
                MSR     CPSR_c, #Mode_UND|I_Bit|F_Bit
                MOV     SP, R0
                SUB     R0, R0, #UND_Stack_Size

//  Stos dla trybu Abort
                MSR     CPSR_c, #Mode_ABT|I_Bit|F_Bit
                MOV     SP, R0
                SUB     R0, R0, #ABT_Stack_Size

//  Stos dla trybu FIQ
                MSR     CPSR_c, #Mode_FIQ|I_Bit|F_Bit
                MOV     SP, R0
                SUB     R0, R0, #FIQ_Stack_Size

//  Stos dla trybu IRQ
                MSR     CPSR_c, #Mode_IRQ|I_Bit|F_Bit
                MOV     SP, R0
                SUB     R0, R0, #IRQ_Stack_Size

//  Stos dla trybu SVC
                MSR     CPSR_c, #Mode_SVC|I_Bit|F_Bit
                MOV     SP, R0
                SUB     R0, R0, #SVC_Stack_Size

//  Stos dla trybu USER i SYSTEM
                MSR     CPSR_c, #Mode_SYS|I_Bit|F_Bit
                MOV     SP, R0

//  Setup a default Stack Limit (when compiled with "-mapcs-stack-check")
                SUB     SL, SP, #USR_Stack_Size


// Wyczysc sekcje .bss (Inicjalizacja 0)
                MOV     R0, #0
                LDR     R1, =__bss_start__
                LDR     R2, =__bss_end__
LoopZI:         CMP     R1, R2
                STRLO   R0, [R1], #4
                BLO     LoopZI

// Przenies sekcje .data kopiuj dane z ROM do RAM
                LDR     R1, =_etext
                LDR     R2, =_data
                LDR     R3, =_edata
LoopRel:        CMP     R2, R3
                LDRLO   R0, [R1], #4
                STRLO   R0, [R2], #4
                BLO     LoopRel

//Wywolaj konstruktory                
			  	LDR R0, =__ctors_start__
			  	LDR R1, =__ctors_end__
ctor_loop:
			  	CMP R0, R1
			  	BEQ ctor_end
			  	LDR R2, [R0], #+4
			  	STMFD SP!, {R0-R1}
			  	MOV LR, PC
			  	MOV PC, R2
			  	LDMFD SP!, {R0-R1}
			  	B ctor_loop
ctor_end:

//Uruchom funkcje main
                LDR R2, =main
                MOV LR, PC
                MOV PC, R2

//Wywolaj destruktory
			   	LDR R0, =__dtors_start__
			  	LDR R1, =__dtors_end__
dtor_loop:
			  	CMP R0, R1
			  	BEQ dtor_end
			  	LDR R2, [R0], #+4
			  	STMFD SP!, {R0-R1}
			  	MOV LR, PC
			  	MOV PC, R2
			  	LDMFD SP!, {R0-R1}
			  	B dtor_loop
dtor_end:

//Koniec krec sie w pustej petli
exit_loop:
  				B exit_loop

        .size   _startup, . - _startup
        .endfunc
        .end

