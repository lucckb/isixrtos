/******************************************************************************
 * @file:    core_cm3.h
 * @purpose: CMSIS Cortex-M3 Core Peripheral Access Layer Header File
 * @version: V1.20
 * @date:    22. May 2009
 *----------------------------------------------------------------------------
 *
 * Copyright (C) 2009 ARM Limited. All rights reserved.
 *
 * ARM Limited (ARM) is supplying this software for use with Cortex-Mx 
 * processor based microcontrollers.  This file can be freely distributed 
 * within development tools that are supporting such ARM based processors. 
 *
 * THIS SOFTWARE IS PROVIDED "AS IS".  NO WARRANTIES, WHETHER EXPRESS, IMPLIED
 * OR STATUTORY, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE.
 * ARM SHALL NOT, IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL, OR
 * CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.
 *
 ******************************************************************************/

#ifndef __CM3_CORE_H__
#define __CM3_CORE_H__

#ifdef __cplusplus
 extern "C" {
#endif 

#define __CM3_CMSIS_VERSION_MAIN  (0x01)                                                       /*!< [31:16] CMSIS HAL main version */
#define __CM3_CMSIS_VERSION_SUB   (0x20)                                                       /*!< [15:0]  CMSIS HAL sub version  */
#define __CM3_CMSIS_VERSION       ((__CM3_CMSIS_VERSION_MAIN << 16) | __CM3_CMSIS_VERSION_SUB) /*!< CMSIS HAL version number       */

#define __CORTEX_M                (0x03)                                                       /*!< Cortex core                    */

/**
 *  Lint configuration \n
 *  ----------------------- \n
 *
 *  The following Lint messages will be suppressed and not shown: \n
 *  \n
 *    --- Error 10: --- \n
 *    register uint32_t __regBasePri         __asm("basepri"); \n
 *    Error 10: Expecting ';' \n
 *     \n
 *    --- Error 530: --- \n
 *    return(__regBasePri); \n
 *    Warning 530: Symbol '__regBasePri' (line 264) not initialized \n
 *     \n
 *    --- Error 550: --- \n
 *      __regBasePri = (basePri & 0x1ff); \n
 *    } \n
 *    Warning 550: Symbol '__regBasePri' (line 271) not accessed \n
 *     \n
 *    --- Error 754: --- \n
 *    uint32_t RESERVED0[24]; \n
 *    Info 754: local structure member '<some, not used in the HAL>' (line 109, file ./cm3_core.h) not referenced \n
 *     \n
 *    --- Error 750: --- \n
 *    #define __CM3_CORE_H__ \n
 *    Info 750: local macro '__CM3_CORE_H__' (line 43, file./cm3_core.h) not referenced \n
 *     \n
 *    --- Error 528: --- \n
 *    static __INLINE void NVIC_DisableIRQ(uint32_t IRQn) \n
 *    Warning 528: Symbol 'NVIC_DisableIRQ(unsigned int)' (line 419, file ./cm3_core.h) not referenced \n
 *     \n
 *    --- Error 751: --- \n
 *    } InterruptType_Type; \n
 *    Info 751: local typedef 'InterruptType_Type' (line 170, file ./cm3_core.h) not referenced \n
 * \n
 * \n
 *    Note:  To re-enable a Message, insert a space before 'lint' * \n
 *
 */

/*lint -save */
/*lint -e10  */
/*lint -e530 */
/*lint -e550 */
/*lint -e754 */
/*lint -e750 */
/*lint -e528 */
/*lint -e751 */


#include <stdint.h>                           /* Include standard types */

#if defined (__ICCARM__)
  #include <intrinsics.h>                     /* IAR Intrinsics   */
#endif


#ifndef __NVIC_PRIO_BITS
  #define __NVIC_PRIO_BITS    4               /*!< standard definition for NVIC Priority Bits */
#endif




/**
 * IO definitions
 *
 * define access restrictions to peripheral registers
 */

#ifdef __cplusplus
#define     __I     volatile                  /*!< defines 'read only' permissions      */
#else
#define     __I     volatile const            /*!< defines 'read only' permissions      */
#endif
#define     __O     volatile                  /*!< defines 'write only' permissions     */
#define     __IO    volatile                  /*!< defines 'read / write' permissions   */



/*******************************************************************************
 *                 Register Abstraction
 ******************************************************************************/


/* System Reset */
#define NVIC_VECTRESET              0         /*!< Vector Reset Bit             */
#define NVIC_SYSRESETREQ            2         /*!< System Reset Request         */
#define NVIC_AIRCR_VECTKEY    (0x5FA << 16)   /*!< AIRCR Key for write access   */
#define NVIC_AIRCR_ENDIANESS        15        /*!< Endianess                    */

/* Core Debug */
#define CoreDebug_DEMCR_TRCENA (1 << 24)      /*!< DEMCR TRCENA enable          */
#define ITM_TCR_ITMENA              1         /*!< ITM enable                   */




/* memory mapping struct for Nested Vectored Interrupt Controller (NVIC) */
typedef struct
{
  __IO uint32_t ISER[8];                      /*!< Interrupt Set Enable Register            */
       uint32_t RESERVED0[24];
  __IO uint32_t ICER[8];                      /*!< Interrupt Clear Enable Register          */
       uint32_t RSERVED1[24];
  __IO uint32_t ISPR[8];                      /*!< Interrupt Set Pending Register           */
       uint32_t RESERVED2[24];
  __IO uint32_t ICPR[8];                      /*!< Interrupt Clear Pending Register         */
       uint32_t RESERVED3[24];
  __IO uint32_t IABR[8];                      /*!< Interrupt Active bit Register            */
       uint32_t RESERVED4[56];
  __IO uint8_t  IP[240];                      /*!< Interrupt Priority Register, 8Bit wide   */
       uint32_t RESERVED5[644];
  __O  uint32_t STIR;                         /*!< Software Trigger Interrupt Register      */
}  NVIC_Type;


/* memory mapping struct for System Control Block */
typedef struct
{
  __I  uint32_t CPUID;                        /*!< CPU ID Base Register                                     */
  __IO uint32_t ICSR;                         /*!< Interrupt Control State Register                         */
  __IO uint32_t VTOR;                         /*!< Vector Table Offset Register                             */
  __IO uint32_t AIRCR;                        /*!< Application Interrupt / Reset Control Register           */
  __IO uint32_t SCR;                          /*!< System Control Register                                  */
  __IO uint32_t CCR;                          /*!< Configuration Control Register                           */
  __IO uint8_t  SHP[12];                      /*!< System Handlers Priority Registers (4-7, 8-11, 12-15)    */
  __IO uint32_t SHCSR;                        /*!< System Handler Control and State Register                */
  __IO uint32_t CFSR;                         /*!< Configurable Fault Status Register                       */
  __IO uint32_t HFSR;                         /*!< Hard Fault Status Register                               */
  __IO uint32_t DFSR;                         /*!< Debug Fault Status Register                              */
  __IO uint32_t MMFAR;                        /*!< Mem Manage Address Register                              */
  __IO uint32_t BFAR;                         /*!< Bus Fault Address Register                               */
  __IO uint32_t AFSR;                         /*!< Auxiliary Fault Status Register                          */
  __I  uint32_t PFR[2];                       /*!< Processor Feature Register                               */
  __I  uint32_t DFR;                          /*!< Debug Feature Register                                   */
  __I  uint32_t ADR;                          /*!< Auxiliary Feature Register                               */
  __I  uint32_t MMFR[4];                      /*!< Memory Model Feature Register                            */
  __I  uint32_t ISAR[5];                      /*!< ISA Feature Register                                     */
} SCB_Type;


/* memory mapping struct for SysTick */
typedef struct
{
  __IO uint32_t CTRL;                         /*!< SysTick Control and Status Register */
  __IO uint32_t LOAD;                         /*!< SysTick Reload Value Register       */
  __IO uint32_t VAL;                          /*!< SysTick Current Value Register      */
  __I  uint32_t CALIB;                        /*!< SysTick Calibration Register        */
} SysTick_Type;


/* memory mapping structur for ITM */
typedef struct
{
  __O  union  
  {
    __O  uint8_t    u8;                       /*!< ITM Stimulus Port 8-bit               */
    __O  uint16_t   u16;                      /*!< ITM Stimulus Port 16-bit              */
    __O  uint32_t   u32;                      /*!< ITM Stimulus Port 32-bit              */
  }  PORT [32];                               /*!< ITM Stimulus Port Registers           */
       uint32_t RESERVED0[864];
  __IO uint32_t TER;                          /*!< ITM Trace Enable Register             */
       uint32_t RESERVED1[15];
  __IO uint32_t TPR;                          /*!< ITM Trace Privilege Register          */
       uint32_t RESERVED2[15];
  __IO uint32_t TCR;                          /*!< ITM Trace Control Register            */
       uint32_t RESERVED3[29];
  __IO uint32_t IWR;                          /*!< ITM Integration Write Register        */
  __IO uint32_t IRR;                          /*!< ITM Integration Read Register         */
  __IO uint32_t IMCR;                         /*!< ITM Integration Mode Control Register */
       uint32_t RESERVED4[43];
  __IO uint32_t LAR;                          /*!< ITM Lock Access Register              */
  __IO uint32_t LSR;                          /*!< ITM Lock Status Register              */
       uint32_t RESERVED5[6];
  __I  uint32_t PID4;                         /*!< ITM Product ID Registers              */
  __I  uint32_t PID5;
  __I  uint32_t PID6;
  __I  uint32_t PID7;
  __I  uint32_t PID0;
  __I  uint32_t PID1;
  __I  uint32_t PID2;
  __I  uint32_t PID3;
  __I  uint32_t CID0;
  __I  uint32_t CID1;
  __I  uint32_t CID2;
  __I  uint32_t CID3;
} ITM_Type;


/* memory mapped struct for Interrupt Type */
typedef struct
{
       uint32_t RESERVED0;
  __I  uint32_t ICTR;                         /*!< Interrupt Control Type Register  */
#if ((defined __CM3_REV) && (__CM3_REV >= 0x200))
  __IO uint32_t ACTLR;                        /*!< Auxiliary Control Register       */
#else
       uint32_t RESERVED1;
#endif
} InterruptType_Type;


/* Memory Protection Unit */
#if defined (__MPU_PRESENT) && (__MPU_PRESENT == 1)
typedef struct
{
  __I  uint32_t TYPE;                         /*!< MPU Type Register                               */
  __IO uint32_t CTRL;                         /*!< MPU Control Register                            */
  __IO uint32_t RNR;                          /*!< MPU Region RNRber Register                      */
  __IO uint32_t RBAR;                         /*!< MPU Region Base Address Register                */
  __IO uint32_t RASR;                         /*!< MPU Region Attribute and Size Register          */
  __IO uint32_t RBAR_A1;                      /*!< MPU Alias 1 Region Base Address Register        */
  __IO uint32_t RASR_A1;                      /*!< MPU Alias 1 Region Attribute and Size Register  */
  __IO uint32_t RBAR_A2;                      /*!< MPU Alias 2 Region Base Address Register        */
  __IO uint32_t RASR_A2;                      /*!< MPU Alias 2 Region Attribute and Size Register  */
  __IO uint32_t RBAR_A3;                      /*!< MPU Alias 3 Region Base Address Register        */
  __IO uint32_t RASR_A3;                      /*!< MPU Alias 3 Region Attribute and Size Register  */
} MPU_Type;
#endif


/* Core Debug Register */
typedef struct
{
  __IO uint32_t DHCSR;                        /*!< Debug Halting Control and Status Register       */
  __O  uint32_t DCRSR;                        /*!< Debug Core Register Selector Register           */
  __IO uint32_t DCRDR;                        /*!< Debug Core Register Data Register               */
  __IO uint32_t DEMCR;                        /*!< Debug Exception and Monitor Control Register    */
} CoreDebug_Type;


/* Memory mapping of Cortex-M3 Hardware */
#define SCS_BASE            (0xE000E000)                              /*!< System Control Space Base Address    */
#define ITM_BASE            (0xE0000000)                              /*!< ITM Base Address                     */
#define CoreDebug_BASE      (0xE000EDF0)                              /*!< Core Debug Base Address              */
#define SysTick_BASE        (SCS_BASE +  0x0010)                      /*!< SysTick Base Address                 */
#define NVIC_BASE           (SCS_BASE +  0x0100)                      /*!< NVIC Base Address                    */
#define SCB_BASE            (SCS_BASE +  0x0D00)                      /*!< System Control Block Base Address    */

#define InterruptType       ((InterruptType_Type *) SCS_BASE)         /*!< Interrupt Type Register              */
#define SCB                 ((SCB_Type *)           SCB_BASE)         /*!< SCB configuration struct             */
#define SysTick             ((SysTick_Type *)       SysTick_BASE)     /*!< SysTick configuration struct         */
#define NVIC                ((NVIC_Type *)          NVIC_BASE)        /*!< NVIC configuration struct            */
#define ITM                 ((ITM_Type *)           ITM_BASE)         /*!< ITM configuration struct             */
#define CoreDebug           ((CoreDebug_Type *)     CoreDebug_BASE)   /*!< Core Debug configuration struct      */

#if defined (__MPU_PRESENT) && (__MPU_PRESENT == 1)
  #define MPU_BASE          (SCS_BASE +  0x0D90)                      /*!< Memory Protection Unit               */
  #define MPU               ((MPU_Type*)            MPU_BASE)         /*!< Memory Protection Unit               */
#endif



#ifdef __cplusplus
}
#endif

#endif /* __CM3_CORE_H__ */

/*lint -restore */
