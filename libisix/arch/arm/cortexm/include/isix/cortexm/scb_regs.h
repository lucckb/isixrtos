/*
 * =====================================================================================
 *
 *       Filename:  scb_memmap.h
 *
 *    Description:  SCB cortex M core memory map
 *
 *        Version:  1.0
 *        Created:  15.04.2017 21:18:02
 *       Revision:  none
 *       Compiler:  gcc
 * 	 	License: LGPL
 *
 *
 * =====================================================================================
 */

#pragma once

#include "memorymap.h"
#include <isix/arch/io.h>


/* CPUID: CPUID base register */
#define SCB_CPUID				MMIO32(SCB_BASE + 0x00)

/* ICSR: Interrupt Control State Register */
#define SCB_ICSR				MMIO32(SCB_BASE + 0x04)

/* VTOR: Vector Table Offset Register */
#define SCB_VTOR				MMIO32(SCB_BASE + 0x08)

/* AIRCR: Application Interrupt and Reset Control Register */
#define SCB_AIRCR				MMIO32(SCB_BASE + 0x0C)

/* SCR: System Control Register */
#define SCB_SCR					MMIO32(SCB_BASE + 0x10)

/* CCR: Configuration Control Register */
#define SCB_CCR					MMIO32(SCB_BASE + 0x14)

/* SHP: System Handler Priority Registers */
/* Note: 12 8bit registers */
#define SCB_SHPR(shpr_id)			MMIO8(SCB_BASE + 0x18 + (shpr_id))
#define SCB_SHPR1				MMIO32(SCB_BASE + 0x18)
#define SCB_SHPR2				MMIO32(SCB_BASE + 0x1C)
#define SCB_SHPR3				MMIO32(SCB_BASE + 0x20)

/* SHCSR: System Handler Control and State Register */
#define SCB_SHCSR				MMIO32(SCB_BASE + 0x24)

/* DFSR: Debug Fault Status Register */
#define SCB_DFSR				MMIO32(SCB_BASE + 0x30)



/* Those defined only on ARMv7 and above */
#if defined(__ARM_ARCH_7M__) || defined(__ARM_ARCH_7EM__)
/* CFSR: Configurable Fault Status Registers */
#define SCB_CFSR				MMIO32(SCB_BASE + 0x28)

/* HFSR: Hard Fault Status Register */
#define SCB_HFSR				MMIO32(SCB_BASE + 0x2C)

/* MMFAR: Memory Manage Fault Address Register */
#define SCB_MMFAR				MMIO32(SCB_BASE + 0x34)

/* BFAR: Bus Fault Address Register */
#define SCB_BFAR				MMIO32(SCB_BASE + 0x38)

/* AFSR: Auxiliary Fault Status Register */
#define SCB_AFSR				MMIO32(SCB_BASE + 0x3C)

/* ID_PFR0: Processor Feature Register 0 */
#define SCB_ID_PFR0				MMIO32(SCB_BASE + 0x40)

/* ID_PFR1: Processor Feature Register 1 */
#define SCB_ID_PFR1				MMIO32(SCB_BASE + 0x44)

/* ID_DFR0: Debug Features Register 0 */
#define SCB_ID_DFR0				MMIO32(SCB_BASE + 0x48)

/* ID_AFR0: Auxiliary Features Register 0 */
#define SCB_ID_AFR0				MMIO32(SCB_BASE + 0x4C)

/* ID_MMFR0: Memory Model Feature Register 0 */
#define SCB_ID_MMFR0				MMIO32(SCB_BASE + 0x50)

/* ID_MMFR1: Memory Model Feature Register 1 */
#define SCB_ID_MMFR1				MMIO32(SCB_BASE + 0x54)

/* ID_MMFR2: Memory Model Feature Register 2 */
#define SCB_ID_MMFR2				MMIO32(SCB_BASE + 0x58)

/* ID_MMFR3: Memory Model Feature Register 3 */
#define SCB_ID_MMFR3				MMIO32(SCB_BASE + 0x5C)

/* ID_ISAR0: Instruction Set Attributes Register 0 */
#define SCB_ID_ISAR0				MMIO32(SCB_BASE + 0x60)

/* ID_ISAR1: Instruction Set Attributes Register 1 */
#define SCB_ID_ISAR1				MMIO32(SCB_BASE + 0x64)

/* ID_ISAR2: Instruction Set Attributes Register 2 */
#define SCB_ID_ISAR2				MMIO32(SCB_BASE + 0x68)

/* ID_ISAR3: Instruction Set Attributes Register 3 */
#define SCB_ID_ISAR3				MMIO32(SCB_BASE + 0x6C)

/* ID_ISAR4: Instruction Set Attributes Register 4 */
#define SCB_ID_ISAR4				MMIO32(SCB_BASE + 0x70)

/*!< Offset: 0x078 (R/ )  Cache Level ID register */
#define SCB_CLIDR				MMIO32(SCB_BASE + 0x78)

/*!< Offset: 0x07C (R/ )  Cache Type register */
#define SCB_CTR				MMIO32(SCB_BASE + 0x7C)

/*!< Offset: 0x080 (R/ )  Cache Size ID Register */
#define SCB_CCSIDR				MMIO32(SCB_BASE + 0x80)

/*!< Offset: 0x084 (R/W)  Cache Size Selection Register */
#define SCB_CSSELR				MMIO32(SCB_BASE + 0x84)



/* CPACR: Coprocessor Access Control Register */
#define SCB_CPACR				MMIO32(SCB_BASE + 0x88)

/* FPCCR: Floating-Point Context Control Register */
#define SCB_FPCCR				MMIO32(SCB_BASE + 0x234)

/* FPCAR: Floating-Point Context Address Register */
#define SCB_FPCAR				MMIO32(SCB_BASE + 0x238)

/* FPDSCR: Floating-Point Default Status Control Register */
#define SCB_FPDSCR				MMIO32(SCB_BASE + 0x23C)

/* MVFR0: Media and Floating-Point Feature Register 0 */
#define SCB_MVFR0				MMIO32(SCB_BASE + 0x240)

/* MVFR1: Media and Floating-Point Feature Register 1 */
#define SCB_MVFR1				MMIO32(SCB_BASE + 0x244)

/*!< Offset: 0x248 (R/ )  Media and VFP Feature Register 2 */
#define SCB_MVFR2				MMIO32(SCB_BASE + 0x248)

/*!< Offset: 0x250 ( /W)  I-Cache Invalidate All to PoU */
#define SCB_ICIALLU				MMIO32(SCB_BASE + 0x250)

/*!< Offset: 0x258 ( /W)  I-Cache Invalidate by MVA to PoU */
#define SCB_ICIMVAU				MMIO32(SCB_BASE + 0x258)

/*!< Offset: 0x25C ( /W)  D-Cache Invalidate by MVA to PoC */
#define SCB_DCIMVAC				MMIO32(SCB_BASE + 0x25C)

/*!< Offset: 0x260 ( /W)  D-Cache Invalidate by Set-way */
#define SCB_DCISW				MMIO32(SCB_BASE + 0x260)

/*!< Offset: 0x264 ( /W)  D-Cache Clean by MVA to PoU */
#define SCB_DCCMVAU				MMIO32(SCB_BASE + 0x264)

/*!< Offset: 0x268 ( /W)  D-Cache Clean by MVA to PoC */
#define SCB_DCCMVAC				MMIO32(SCB_BASE + 0x268)

/*!< Offset: 0x26C ( /W)  D-Cache Clean by Set-way */
#define SCB_DCCSW				MMIO32(SCB_BASE + 0x26C)

/*!< Offset: 0x270 ( /W)  D-Cache Clean and Invalidate by MVA to PoC */
#define SCB_DCCIMVAC				MMIO32(SCB_BASE + 0x270)

/*!< Offset: 0x274 ( /W)  D-Cache Clean and Invalidate by Set-way */
#define SCB_DCCISW				MMIO32(SCB_BASE + 0x274)

/*!< Offset: 0x290 (R/W)  Instruction Tightly-Coupled Memory Control Register */
#define SCB_ITCMCR				MMIO32(SCB_BASE + 0x290)

/*!< Offset: 0x294 (R/W)  Data Tightly-Coupled Memory Control Registers */
#define SCB_DTCMCR				MMIO32(SCB_BASE + 0x294)

/*!< Offset: 0x298 (R/W)  AHBP Control Register */
#define SCB_AHBPCR				MMIO32(SCB_BASE + 0x298)

/*!< Offset: 0x29C (R/W)  L1 Cache Control Register */
#define SCB_CACR				MMIO32(SCB_BASE + 0x29C)

/*!< Offset: 0x2A0 (R/W)  AHB Slave Control Register */
#define SCB_AHBSCR				MMIO32(SCB_BASE + 0x2A0)

/*!< Offset: 0x2A8 (R/W)  Auxiliary Bus Fault Status Register */
#define SCB_ABFSR				MMIO32(SCB_BASE + 0x2A8)


#endif

/* --- SCB values ---------------------------------------------------------- */

/* --- SCB_CPUID values ---------------------------------------------------- */

/* Implementer[31:24]: Implementer code */
#define SCB_CPUID_IMPLEMENTER_LSB	24
#define SCB_CPUID_IMPLEMENTER		(0xFF << SCB_CPUID_IMPLEMENTER_LSB)
/* Variant[23:20]: Variant number */
#define SCB_CPUID_VARIANT_LSB		2
#define SCB_CPUID_VARIANT		(0xF << SCB_CPUID_VARIANT_LSB)
/* Constant[19:16]: Reads as 0xF (ARMv7-M) M3, M4  */
/* Constant[19:16]: Reads as 0xC (ARMv6-M) M0, M0+ */
#define SCB_CPUID_CONSTANT_LSB		16
#define SCB_CPUID_CONSTANT		(0xF << SCB_CPUID_CONSTANT_LSB)
#define SCB_CPUID_CONSTANT_ARMV6	(0xC << SCB_CPUID_CONSTANT_LSB)
#define SCB_CPUID_CONSTANT_ARMV7	(0xF << SCB_CPUID_CONSTANT_LSB)

/* PartNo[15:4]: Part number of the processor */
#define SCB_CPUID_PARTNO_LSB		4
#define SCB_CPUID_PARTNO		(0xFFF << SCB_CPUID_PARTNO_LSB)
/* Revision[3:0]: Revision number */
#define SCB_CPUID_REVISION_LSB			0
#define SCB_CPUID_REVISION		(0xF << SCB_CPUID_REVISION_LSB)

/* --- SCB_ICSR values ----------------------------------------------------- */

/* NMIPENDSET: NMI set-pending bit */
#define SCB_ICSR_NMIPENDSET		(1 << 31)
/* Bits [30:29]: reserved - must be kept cleared */
/* PENDSVSET: PendSV set-pending bit */
#define SCB_ICSR_PENDSVSET		(1 << 28)
/* PENDSVCLR: PendSV clear-pending bit */
#define SCB_ICSR_PENDSVCLR		(1 << 27)
/* PENDSTSET: SysTick exception set-pending bit */
#define SCB_ICSR_PENDSTSET		(1 << 26)
/* PENDSTCLR: SysTick exception clear-pending bit */
#define SCB_ICSR_PENDSTCLR		(1 << 25)
/* Bit 24: reserved - must be kept cleared */
/* Bit 23: reserved for debug - reads as 0 when not in debug mode */
#define SCB_ICSR_ISRPREEMPT		(1 << 23)
/* ISRPENDING: Interrupt pending flag, excluding NMI and Faults */
#define SCB_ICSR_ISRPENDING		(1 << 22)
/* VECTPENDING[21:12] Pending vector */
#define SCB_ICSR_VECTPENDING_LSB	12
#define SCB_ICSR_VECTPENDING		(0x1FF << SCB_ICSR_VECTPENDING_LSB)
/* RETOBASE: Return to base level */
#define SCB_ICSR_RETOBASE		(1 << 11)
/* Bits [10:9]: reserved - must be kept cleared */
/* VECTACTIVE[8:0] Active vector */
#define SCB_ICSR_VECTACTIVE_LSB		0
#define SCB_ICSR_VECTACTIVE		(0x1FF << SCB_ICSR_VECTACTIVE_LSB)


/* --- SCB_VTOR values ----------------------------------------------------- */

/* IMPLEMENTATION DEFINED */

#if defined(__ARM_ARCH_6M__)

#define SCB_VTOR_TBLOFF_LSB		7
#define SCB_VTOR_TBLOFF			(0x1FFFFFF << SCB_VTOR_TBLOFF_LSB)

#elif defined(CM1)
/* VTOR not defined there */

#elif defined(__ARM_ARCH_7M__) || defined(__ARM_ARCH_7EM__)

/* Bits [31:30]: reserved - must be kept cleared */
/* TBLOFF[29:9]: Vector table base offset field */
/* inconsistent datasheet - LSB could be 11 */
/* BUG: TBLOFF is in the ARMv6 Architecture reference manual defined from b7 */
#define SCB_VTOR_TBLOFF_LSB		9
#define SCB_VTOR_TBLOFF			(0x7FFFFF << SCB_VTOR_TBLOFF_LSB)

#endif

/* --- SCB_AIRCR values ---------------------------------------------------- */

/* VECTKEYSTAT[31:16]/ VECTKEY[31:16] Register key */
#define SCB_AIRCR_VECTKEYSTAT_LSB	16
#define SCB_AIRCR_VECTKEYSTAT		(0xFFFF << SCB_AIRCR_VECTKEYSTAT_LSB)
#define SCB_AIRCR_VECTKEY		(0x05FA << SCB_AIRCR_VECTKEYSTAT_LSB)

/* ENDIANNESS Data endianness bit */
#define SCB_AIRCR_ENDIANESS			(1 << 15)

/* Those defined only on ARMv7 and above */
#if defined(__ARM_ARCH_7M__) || defined(__ARM_ARCH_7EM__)
/* Bits [14:11]: reserved - must be kept cleared */
/* PRIGROUP[10:8]: Interrupt priority grouping field */
#define SCB_AIRCR_PRIGROUP_GROUP16_NOSUB	(0x3 << 8)
#define SCB_AIRCR_PRIGROUP_GROUP8_SUB2		(0x4 << 8)
#define SCB_AIRCR_PRIGROUP_GROUP4_SUB4		(0x5 << 8)
#define SCB_AIRCR_PRIGROUP_GROUP2_SUB8		(0x6 << 8)
#define SCB_AIRCR_PRIGROUP_NOGROUP_SUB16	(0x7 << 8)
#define SCB_AIRCR_PRIGROUP_MASK			(0x7 << 8)
#define SCB_AIRCR_PRIGROUP_SHIFT		8
/* Bits [7:3]: reserved - must be kept cleared */
#endif

/* SYSRESETREQ System reset request */
#define SCB_AIRCR_SYSRESETREQ			(1 << 2)
/* VECTCLRACTIVE */
#define SCB_AIRCR_VECTCLRACTIVE			(1 << 1)

/* Those defined only on ARMv7 and above */
#if defined(__ARM_ARCH_7M__) || defined(__ARM_ARCH_7EM__)
/* VECTRESET */
#define SCB_AIRCR_VECTRESET			(1 << 0)
#endif

/* --- SCB_SCR values ------------------------------------------------------ */

/* Bits [31:5]: reserved - must be kept cleared */
/* SEVEONPEND Send Event on Pending bit */
#define SCB_SCR_SEVEONPEND			(1 << 4)
/* Bit 3: reserved - must be kept cleared */
/* SLEEPDEEP */
#define SCB_SCR_SLEEPDEEP			(1 << 2)
/* SLEEPONEXIT */
#define SCB_SCR_SLEEPONEXIT			(1 << 1)
/* Bit 0: reserved - must be kept cleared */

/* --- SCB_CCR values ------------------------------------------------------ */

/* Bits [31:10]: reserved - must be kept cleared */
/* STKALIGN */
#define SCB_CCR_STKALIGN			(1 << 9)

/* Those defined only on ARMv7 and above */
#if defined(__ARM_ARCH_7M__) || defined(__ARM_ARCH_7EM__)
/* BFHFNMIGN */
#define SCB_CCR_BFHFNMIGN			(1 << 8)
/* Bits [7:5]: reserved - must be kept cleared */
/* DIV_0_TRP */
#define SCB_CCR_DIV_0_TRP			(1 << 4)
#endif

/* UNALIGN_TRP */
#define SCB_CCR_UNALIGN_TRP			(1 << 3)

/* Those defined only on ARMv7 and above */
#if defined(__ARM_ARCH_7M__) || defined(__ARM_ARCH_7EM__)
/* Bit 2: reserved - must be kept cleared */
/* USERSETMPEND */
#define SCB_CCR_USERSETMPEND			(1 << 1)
/* NONBASETHRDENA */
#define SCB_CCR_NONBASETHRDENA			(1 << 0)
#endif

#if defined(__ARM_ARCH_7EM__)
// Data cache enable
#define SCB_CCR_DCACHE				(1U<16U)
// Instruction cache enable
#define SCB_CCR_ICACHE				(1U<17U)
// Branch predictor enable
#define SCB_CCR_BRANCH_PREDICTOR	(1U<18U)
#endif

/* These numbers are designed to be used with the SCB_SHPR() macro */
/* SCB_SHPR1 */
#define SCB_SHPR_PRI_4_MEMMANAGE	0
#define SCB_SHPR_PRI_5_BUSFAULT		1
#define SCB_SHPR_PRI_6_USAGEFAULT	2
#define SCB_SHPR_PRI_7_RESERVED		3
/* SCB_SHPR2  */
#define SCB_SHPR_PRI_8_RESERVED		4
#define SCB_SHPR_PRI_9_RESERVED		5
#define SCB_SHPR_PRI_10_RESERVED	6
#define SCB_SHPR_PRI_11_SVCALL		7
/* SCB_SHPR3 */
#define SCB_SHPR_PRI_12_RESERVED	8
#define SCB_SHPR_PRI_13_RESERVED	9
#define SCB_SHPR_PRI_14_PENDSV		10
#define SCB_SHPR_PRI_15_SYSTICK		11

/* --- SCB_SHCSR values ---------------------------------------------------- */

/* Bits [31:19]: reserved - must be kept cleared */

/* Those defined only on ARMv7 and above */
#if defined(__ARM_ARCH_7M__) || defined(__ARM_ARCH_7EM__)
/* USGFAULTENA: Usage fault enable */
#define SCB_SHCSR_USGFAULTENA			(1 << 18)
/* BUSFAULTENA: Bus fault enable */
#define SCB_SHCSR_BUSFAULTENA			(1 << 17)
/* MEMFAULTENA: Memory management fault enable */
#define SCB_SHCSR_MEMFAULTENA			(1 << 16)
#endif

/* SVCALLPENDED: SVC call pending */
#define SCB_SHCSR_SVCALLPENDED			(1 << 15)

/* Those defined only on ARMv7 and above */
#if defined(__ARM_ARCH_7M__) || defined(__ARM_ARCH_7EM__)
/* BUSFAULTPENDED: Bus fault exception pending */
#define SCB_SHCSR_BUSFAULTPENDED		(1 << 14)
/* MEMFAULTPENDED: Memory management fault exception pending */
#define SCB_SHCSR_MEMFAULTPENDED		(1 << 13)
/* USGFAULTPENDED: Usage fault exception pending */
#define SCB_SHCSR_USGFAULTPENDED		(1 << 12)
/* SYSTICKACT: SysTick exception active */
#define SCB_SHCSR_SYSTICKACT			(1 << 11)
/* PENDSVACT: PendSV exception active */
#define SCB_SHCSR_PENDSVACT			(1 << 10)
/* Bit 9: reserved - must be kept cleared */
/* MONITORACT: Debug monitor active */
#define SCB_SHCSR_MONITORACT			(1 << 8)
/* SVCALLACT: SVC call active */
#define SCB_SHCSR_SVCALLACT			(1 << 7)
/* Bits [6:4]: reserved - must be kept cleared */
/* USGFAULTACT: Usage fault exception active */
#define SCB_SHCSR_USGFAULTACT			(1 << 3)
/* Bit 2: reserved - must be kept cleared */
/* BUSFAULTACT: Bus fault exception active */
#define SCB_SHCSR_BUSFAULTACT			(1 << 1)
/* MEMFAULTACT: Memory management fault exception active */
#define SCB_SHCSR_MEMFAULTACT			(1 << 0)

/* --- SCB_CFSR values ----------------------------------------------------- */

/* Bits [31:26]: reserved - must be kept cleared */
/* DIVBYZERO: Divide by zero usage fault */
#define SCB_CFSR_DIVBYZERO			(1 << 25)
/* UNALIGNED: Unaligned access usage fault */
#define SCB_CFSR_UNALIGNED			(1 << 24)
/* Bits [23:20]: reserved - must be kept cleared */
/* NOCP: No coprocessor usage fault */
#define SCB_CFSR_NOCP				(1 << 19)
/* INVPC: Invalid PC load usage fault */
#define SCB_CFSR_INVPC				(1 << 18)
/* INVSTATE: Invalid state usage fault */
#define SCB_CFSR_INVSTATE			(1 << 17)
/* UNDEFINSTR: Undefined instruction usage fault */
#define SCB_CFSR_UNDEFINSTR			(1 << 16)
/* BFARVALID: Bus Fault Address Register (BFAR) valid flag */
#define SCB_CFSR_BFARVALID			(1 << 15)
/* Bits [14:13]: reserved - must be kept cleared */
/* STKERR: Bus fault on stacking for exception entry */
#define SCB_CFSR_STKERR				(1 << 12)
/* UNSTKERR: Bus fault on unstacking for a return from exception */
#define SCB_CFSR_UNSTKERR			(1 << 11)
/* IMPRECISERR: Imprecise data bus error */
#define SCB_CFSR_IMPRECISERR			(1 << 10)
/* PRECISERR: Precise data bus error */
#define SCB_CFSR_PRECISERR			(1 << 9)
/* IBUSERR: Instruction bus error */
#define SCB_CFSR_IBUSERR			(1 << 8)
/* MMARVALID: Memory Management Fault Address Register (MMAR) valid flag */
#define SCB_CFSR_MMARVALID			(1 << 7)
/* Bits [6:5]: reserved - must be kept cleared */
/* MSTKERR: Memory manager fault on stacking for exception entry */
#define SCB_CFSR_MSTKERR			(1 << 4)
/* MUNSTKERR: Memory manager fault on unstacking for a return from exception */
#define SCB_CFSR_MUNSTKERR			(1 << 3)
/* Bit 2: reserved - must be kept cleared */
/* DACCVIOL: Data access violation flag */
#define SCB_CFSR_DACCVIOL			(1 << 1)
/* IACCVIOL: Instruction access violation flag */
#define SCB_CFSR_IACCVIOL			(1 << 0)

/* --- SCB_HFSR values ----------------------------------------------------- */

/* DEBUG_VT: reserved for debug use */
#define SCB_HFSR_DEBUG_VT			(1 << 31)
/* FORCED: Forced hard fault */
#define SCB_HFSR_FORCED				(1 << 30)
/* Bits [29:2]: reserved - must be kept cleared */
/* VECTTBL: Vector table hard fault */
#define SCB_HFSR_VECTTBL			(1 << 1)
/* Bit 0: reserved - must be kept cleared */

/* --- SCB_MMFAR values ---------------------------------------------------- */

/* MMFAR [31:0]: Memory management fault address */

/* --- SCB_BFAR values ----------------------------------------------------- */

/* BFAR [31:0]: Bus fault address */

/* --- SCB_CPACR values ---------------------------------------------------- */

/* CPACR CPn: Access privileges values */
#define SCB_CPACR_NONE			0	/* Access denied */
#define SCB_CPACR_PRIV			1	/* Privileged access only */
#define SCB_CPACR_FULL			3	/* Full access */

/* CPACR [20:21]: Access privileges for coprocessor 10 */
#define SCB_CPACR_CP10			(1 << 20)
/* CPACR [22:23]: Access privileges for coprocessor 11 */
#define SCB_CPACR_CP11			(1 << 22)
#endif


/** SCB Cache Size ID register */

/* SCB Cache Size ID Register Definitions */
 /*!< SCB CCSIDR: WT Position */
#define SCB_CCSIDR_WT_BIT                  31U
#define SCB_CCSIDR_WT                  (1UL << SCB_CCSIDR_WT_BIT)

/*!< SCB CCSIDR: WB Position */
#define SCB_CCSIDR_WB_BIT                  30U
#define SCB_CCSIDR_WB                  (1UL << SCB_CCSIDR_WB_BIT)

/*!< SCB CCSIDR: RA Position */
#define SCB_CCSIDR_RA_BIT                  29U
#define SCB_CCSIDR_RA                  (1UL << SCB_CCSIDR_RA_BIT)

/*!< SCB CCSIDR: WA Position */
#define SCB_CCSIDR_WA_BIT                  28U
#define SCB_CCSIDR_WA                  (1UL << SCB_CCSIDR_WA_BIT)

/*!< SCB CCSIDR: NumSets Position */
#define SCB_CCSIDR_NUMSETS_BIT             13U
#define SCB_CCSIDR_NUMSETS             (0x7FFFUL << SCB_CCSIDR_NUMSETS_BIT)

/*!< SCB CCSIDR: Associativity Position */
#define SCB_CCSIDR_ASSOCIATIVITY_BIT        3U
#define SCB_CCSIDR_ASSOCIATIVITY       (0x3FFUL << SCB_CCSIDR_ASSOCIATIVITY_BIT)

/*!< SCB CCSIDR: LineSize Position */
#define SCB_CCSIDR_LINESIZE_BIT             0U
#define SCB_CCSIDR_LINESIZE            (7UL /*<< SCB_CCSIDR_LINESIZE_BIT*/)


/* SCB D-Cache Invalidate by Set-way Register Definitions */
/*!< SCB DCISW: Way Position */
#define SCB_DCISW_WAY_BIT                  30U
#define SCB_DCISW_WAY                  (3UL << SCB_DCISW_WAY_BIT)

/*!< SCB DCISW: Set Position */
#define SCB_DCISW_SET_BIT                   5U
#define SCB_DCISW_SET                  (0x1FFUL << SCB_DCISW_SET_BIT)

/* SCB D-Cache Clean by Set-way Register Definitions */

/*!< SCB DCCSW: Way Position */
#define SCB_DCCSW_WAY_BIT                  30U
#define SCB_DCCSW_WAY                  (3UL << SCB_DCCSW_WAY_BIT)

/*!< SCB DCCSW: Set Position */
#define SCB_DCCSW_SET_BIT                   5U
#define SCB_DCCSW_SET                  (0x1FFUL << SCB_DCCSW_SET_BIT)

/* SCB D-Cache Clean and Invalidate by Set-way Register Definitions */
/*!< SCB DCCISW: Way Position */
#define SCB_DCCISW_WAY_BIT                 30U
#define SCB_DCCISW_WAY                 (3UL << SCB_DCCISW_WAY_BIT)
/*!< SCB DCCISW: Set Position */
#define SCB_DCCISW_SET_BIT                  5U
#define SCB_DCCISW_SET                 (0x1FFUL << SCB_DCCISW_SET_BIT)


