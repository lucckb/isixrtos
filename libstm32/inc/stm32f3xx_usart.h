#pragma once

typedef unsigned int stm32_usart_reg_t;
#define USART_HARDWARE_VERSION 2

/** @defgroup USART_Exported_Constants
  * @{
  */ 

/** @defgroup USART_Word_Length 
  * @{
  */ 

#define USART_WordLength_8b                  ((uint32_t)0x00000000)
#define USART_WordLength_9b                  USART_CR1_M

/** @defgroup USART_Stop_Bits 
  * @{
  */ 

#define USART_StopBits_1                     ((uint32_t)0x00000000)
#define USART_StopBits_2                     USART_CR2_STOP_1
#define USART_StopBits_1_5                   (USART_CR2_STOP_0 | USART_CR2_STOP_1)
/** @defgroup USART_Parity 
  * @{
  */ 

#define USART_Parity_No                      ((uint32_t)0x00000000)
#define USART_Parity_Even                    USART_CR1_PCE
#define USART_Parity_Odd                     (USART_CR1_PCE | USART_CR1_PS) 
/** @defgroup USART_Mode 
  * @{
  */ 

#define USART_Mode_Rx                        USART_CR1_RE
#define USART_Mode_Tx                        USART_CR1_TE

/** @defgroup USART_Hardware_Flow_Control 
  * @{
  */ 

#define USART_HardwareFlowControl_None       ((uint32_t)0x00000000)
#define USART_HardwareFlowControl_RTS        USART_CR3_RTSE
#define USART_HardwareFlowControl_CTS        USART_CR3_CTSE
#define USART_HardwareFlowControl_RTS_CTS    (USART_CR3_RTSE | USART_CR3_CTSE)


/** @defgroup USART_Clock 
  * @{
  */ 
  
#define USART_Clock_Disable                  ((uint32_t)0x00000000)
#define USART_Clock_Enable                   USART_CR2_CLKEN

/** @defgroup USART_Clock_Polarity 
  * @{
  */
  
#define USART_CPOL_Low                       ((uint32_t)0x00000000)
#define USART_CPOL_High                      USART_CR2_CPOL

/** @defgroup USART_Clock_Phase
  * @{
  */

#define USART_CPHA_1Edge                     ((uint32_t)0x00000000)
#define USART_CPHA_2Edge                     USART_CR2_CPHA

/** @defgroup USART_Last_Bit
  * @{
  */

#define USART_LastBit_Disable                ((uint32_t)0x00000000)
#define USART_LastBit_Enable                 USART_CR2_LBCL
  
/** @defgroup USART_DMA_Requests 
  * @{
  */

#define USART_DMAReq_Tx                      USART_CR3_DMAT
#define USART_DMAReq_Rx                      USART_CR3_DMAR

/** @defgroup USART_DMA_Recception_Error
  * @{
  */

#define USART_DMAOnError_Enable              ((uint32_t)0x00000000)
#define USART_DMAOnError_Disable             USART_CR3_DDRE

/** @defgroup USART_MuteMode_WakeUp_methods
  * @{
  */

#define USART_WakeUp_IdleLine                ((uint32_t)0x00000000)
#define USART_WakeUp_AddressMark             USART_CR1_WAKE

/** @defgroup USART_Address_Detection
  * @{
  */ 

#define USART_AddressLength_4b               ((uint32_t)0x00000000)
#define USART_AddressLength_7b               USART_CR2_ADDM7

/** @defgroup USART_StopMode_WakeUp_methods 
  * @{
  */ 

#define USART_WakeUpSource_AddressMatch      ((uint32_t)0x00000000)
#define USART_WakeUpSource_StartBit          USART_CR3_WUS_1
#define USART_WakeUpSource_RXNE              (USART_CR3_WUS_0 | USART_CR3_WUS_1)

/** @defgroup USART_LIN_Break_Detection_Length 
  * @{
  */
  
#define USART_LINBreakDetectLength_10b       ((uint32_t)0x00000000)
#define USART_LINBreakDetectLength_11b       USART_CR2_LBDL

/** @defgroup USART_IrDA_Low_Power 
  * @{
  */

#define USART_IrDAMode_LowPower              USART_CR3_IRLP
#define USART_IrDAMode_Normal                ((uint32_t)0x00000000)

/** @defgroup USART_DE_Polarity 
  * @{
  */

#define USART_DEPolarity_High                ((uint32_t)0x00000000)
#define USART_DEPolarity_Low                 USART_CR3_DEP

/** @defgroup USART_Inversion_Pins 
  * @{
  */

#define USART_InvPin_Tx                      USART_CR2_TXINV
#define USART_InvPin_Rx                      USART_CR2_RXINV

/** @defgroup USART_AutoBaudRate_Mode 
  * @{
  */

#define USART_AutoBaudRate_StartBit          ((uint32_t)0x00000000)
#define USART_AutoBaudRate_FallingEdge       USART_CR2_ABRMODE_0

/** @defgroup USART_OVR_DETECTION
  * @{
  */

#define USART_OVRDetection_Enable            ((uint32_t)0x00000000)
#define USART_OVRDetection_Disable           USART_CR3_OVRDIS

/** @defgroup USART_Request 
  * @{
  */

#define USART_Request_ABRRQ                  USART_RQR_ABRRQ
#define USART_Request_SBKRQ                  USART_RQR_SBKRQ
#define USART_Request_MMRQ                   USART_RQR_MMRQ
#define USART_Request_RXFRQ                  USART_RQR_RXFRQ
#define USART_Request_TXFRQ                  USART_RQR_TXFRQ


/** @defgroup USART_Flags 
  * @{
  */
#define USART_FLAG_REACK                     USART_ISR_REACK
#define USART_FLAG_TEACK                     USART_ISR_TEACK
#define USART_FLAG_WU                        USART_ISR_WUF
#define USART_FLAG_RWU                       USART_ISR_RWU
#define USART_FLAG_SBK                       USART_ISR_SBKF
#define USART_FLAG_CM                        USART_ISR_CMF
#define USART_FLAG_BUSY                      USART_ISR_BUSY
#define USART_FLAG_ABRF                      USART_ISR_ABRF
#define USART_FLAG_ABRE                      USART_ISR_ABRE
#define USART_FLAG_EOB                       USART_ISR_EOBF
#define USART_FLAG_RTO                       USART_ISR_RTOF
#define USART_FLAG_nCTSS                     USART_ISR_CTS 
#define USART_FLAG_CTS                       USART_ISR_CTSIF
#define USART_FLAG_LBD                       USART_ISR_LBD
#define USART_FLAG_TXE                       USART_ISR_TXE
#define USART_FLAG_TC                        USART_ISR_TC
#define USART_FLAG_RXNE                      USART_ISR_RXNE
#define USART_FLAG_IDLE                      USART_ISR_IDLE
#define USART_FLAG_ORE                       USART_ISR_ORE
#define USART_FLAG_NE                        USART_ISR_NE
#define USART_FLAG_FE                        USART_ISR_FE
#define USART_FLAG_PE                        USART_ISR_PE

/** @defgroup USART_Interrupt_definition 
  * @brief USART Interrupt definition
  * USART_IT possible values
  * Elements values convention: 0xZZZZYYXX
  *   XX: Position of the corresponding Interrupt
  *   YY: Register index
  *   ZZZZ: Flag position
  * @{
  */

#define USART_IT_WU                          ((uint32_t)0x00140316)
#define USART_IT_CM                          ((uint32_t)0x0011010E)
#define USART_IT_EOB                         ((uint32_t)0x000C011B)
#define USART_IT_RTO                         ((uint32_t)0x000B011A)
#define USART_IT_PE                          ((uint32_t)0x00000108)
#define USART_IT_TXE                         ((uint32_t)0x00070107)
#define USART_IT_TC                          ((uint32_t)0x00060106)
#define USART_IT_RXNE                        ((uint32_t)0x00050105)
#define USART_IT_IDLE                        ((uint32_t)0x00040104)
#define USART_IT_LBD                         ((uint32_t)0x00080206)
#define USART_IT_CTS                         ((uint32_t)0x0009030A) 
#define USART_IT_ERR                         ((uint32_t)0x00000300)
#define USART_IT_ORE                         ((uint32_t)0x00030300)
#define USART_IT_NE                          ((uint32_t)0x00020300)
#define USART_IT_FE                          ((uint32_t)0x00010300)


