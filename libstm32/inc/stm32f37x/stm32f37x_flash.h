/**
  ******************************************************************************
  * @file    stm32f37x_flash.h
  * @author  MCD Application Team
  * @version V1.0.0
  * @date    20-September-2012
  * @brief   This file contains all the functions prototypes for the FLASH 
  *          firmware library.
  ******************************************************************************/

#pragma once

typedef enum FLASH_StatusType
{ 
  FLASH_BUSY = 1,
  FLASH_ERROR_WRP,
  FLASH_ERROR_PROGRAM,
  FLASH_COMPLETE,
  FLASH_TIMEOUT
}FLASH_Status;


#define FLASH_Latency_0                ((uint8_t)0x0000)    /*!< FLASH Zero Latency cycle */
#define FLASH_Latency_1                FLASH_ACR_LATENCY_0  /*!< FLASH One Latency cycle */
#define FLASH_Latency_2                FLASH_ACR_LATENCY_1  /*!< FLASH Two Latency cycles */

/** @defgroup FLASH_Interrupts 
  * @{
  */
#define FLASH_IT_EOP                   FLASH_CR_EOPIE  /*!< End of programming interrupt source */
#define FLASH_IT_ERR                   FLASH_CR_ERRIE  /*!< Error interrupt source */


/**
  * @}
  */

/** @defgroup FLASH_Option_Bytes_Write_Protection 
  * @{
  */
  

#define OB_WRP_Pages0to1               ((uint32_t)0x00000001) /* Write protection of page 0 to 1 */
#define OB_WRP_Pages2to3               ((uint32_t)0x00000002) /* Write protection of page 2 to 3 */
#define OB_WRP_Pages4to5               ((uint32_t)0x00000004) /* Write protection of page 4 to 5 */
#define OB_WRP_Pages6to7               ((uint32_t)0x00000008) /* Write protection of page 6 to 7 */
#define OB_WRP_Pages8to9               ((uint32_t)0x00000010) /* Write protection of page 8 to 9 */
#define OB_WRP_Pages10to11             ((uint32_t)0x00000020) /* Write protection of page 10 to 11 */
#define OB_WRP_Pages12to13             ((uint32_t)0x00000040) /* Write protection of page 12 to 13 */
#define OB_WRP_Pages14to15             ((uint32_t)0x00000080) /* Write protection of page 14 to 15 */
#define OB_WRP_Pages16to17             ((uint32_t)0x00000100) /* Write protection of page 16 to 17 */
#define OB_WRP_Pages18to19             ((uint32_t)0x00000200) /* Write protection of page 18 to 19 */
#define OB_WRP_Pages20to21             ((uint32_t)0x00000400) /* Write protection of page 20 to 21 */
#define OB_WRP_Pages22to23             ((uint32_t)0x00000800) /* Write protection of page 22 to 23 */
#define OB_WRP_Pages24to25             ((uint32_t)0x00001000) /* Write protection of page 24 to 25 */
#define OB_WRP_Pages26to27             ((uint32_t)0x00002000) /* Write protection of page 26 to 27 */
#define OB_WRP_Pages28to29             ((uint32_t)0x00004000) /* Write protection of page 28 to 29 */
#define OB_WRP_Pages30to31             ((uint32_t)0x00008000) /* Write protection of page 30 to 31 */
#define OB_WRP_Pages32to33             ((uint32_t)0x00010000) /* Write protection of page 32 to 33 */
#define OB_WRP_Pages34to35             ((uint32_t)0x00020000) /* Write protection of page 34 to 35 */
#define OB_WRP_Pages36to37             ((uint32_t)0x00040000) /* Write protection of page 36 to 37 */
#define OB_WRP_Pages38to39             ((uint32_t)0x00080000) /* Write protection of page 38 to 39 */
#define OB_WRP_Pages40to41             ((uint32_t)0x00100000) /* Write protection of page 40 to 41 */
#define OB_WRP_Pages42to43             ((uint32_t)0x00200000) /* Write protection of page 42 to 43 */
#define OB_WRP_Pages44to45             ((uint32_t)0x00400000) /* Write protection of page 44 to 45 */
#define OB_WRP_Pages46to47             ((uint32_t)0x00800000) /* Write protection of page 46 to 47 */
#define OB_WRP_Pages48to49             ((uint32_t)0x01000000) /* Write protection of page 48 to 49 */
#define OB_WRP_Pages50to51             ((uint32_t)0x02000000) /* Write protection of page 50 to 51 */
#define OB_WRP_Pages52to53             ((uint32_t)0x04000000) /* Write protection of page 52 to 53 */
#define OB_WRP_Pages54to55             ((uint32_t)0x08000000) /* Write protection of page 54 to 55 */
#define OB_WRP_Pages56to57             ((uint32_t)0x10000000) /* Write protection of page 56 to 57 */
#define OB_WRP_Pages58to59             ((uint32_t)0x20000000) /* Write protection of page 58 to 59 */
#define OB_WRP_Pages60to61             ((uint32_t)0x40000000) /* Write protection of page 60 to 61 */
#define OB_WRP_Pages62to127            ((uint32_t)0x80000000) /* Write protection of page 62 to 127 */

#define OB_WRP_AllPages                ((uint32_t)0xFFFFFFFF) /*!< Write protection of all Sectors */



/** @defgroup FLASH_Option_Bytes_Read_Protection 
  * @{
  */ 

/** 
  * @brief  FLASH_Read_Protection_Level  
  */ 
#define OB_RDP_Level_0   ((uint8_t)0xAA)
#define OB_RDP_Level_1   ((uint8_t)0xBB)
/*#define OB_RDP_Level_2   ((uint8_t)0xCC)*/ /* Warning: When enabling read protection level 2 
                                                it's no more possible to go back to level 1 or 0 */

/** @defgroup FLASH_Option_Bytes_IWatchdog 
  * @{
  */

#define OB_IWDG_SW                     ((uint8_t)0x01)  /*!< Software IWDG selected */
#define OB_IWDG_HW                     ((uint8_t)0x00)  /*!< Hardware IWDG selected */

/** @defgroup FLASH_Option_Bytes_nRST_STOP 
  * @{
  */

#define OB_STOP_NoRST                  ((uint8_t)0x02) /*!< No reset generated when entering in STOP */
#define OB_STOP_RST                    ((uint8_t)0x00) /*!< Reset generated when entering in STOP */

/** @defgroup FLASH_Option_Bytes_nRST_STDBY 
  * @{
  */

#define OB_STDBY_NoRST                 ((uint8_t)0x04) /*!< No reset generated when entering in STANDBY */
#define OB_STDBY_RST                   ((uint8_t)0x00) /*!< Reset generated when entering in STANDBY */
/** @defgroup FLASH_Option_Bytes_BOOT1
  * @{
  */

#define OB_BOOT1_RESET                 ((uint8_t)0x00) /*!< BOOT1 Reset */
#define OB_BOOT1_SET                   ((uint8_t)0x10) /*!< BOOT1 Set */

/** @defgroup FLASH_Option_Bytes_VDDA_Analog_Monitoring
  * @{
  */

#define OB_VDDA_ANALOG_ON              ((uint8_t)0x20) /*!< Analog monitoring on VDDA Power source ON */
#define OB_VDDA_ANALOG_OFF             ((uint8_t)0x00) /*!< Analog monitoring on VDDA Power source OFF */

/** @defgroup FLASH_Option_Bytes_VDD_Analog_Monitoring
  * @{
  */

#define OB_VDD_SD12_ON              ((uint8_t)0x80) /*!< Analog monitoring on VDDA Power source ON */
#define OB_VDD_SD12_OFF             ((uint8_t)0x00) /*!< Analog monitoring on VDDA Power source OFF */

/** @defgroup FLASH_Option_Bytes_SRAM_Parity_Enable 
  * @{
  */

#define OB_SRAM_PARITY_SET              ((uint8_t)0x00) /*!< SRAM parity enable Set */
#define OB_SRAM_PARITY_RESET            ((uint8_t)0x40) /*!< SRAM parity enable reset */
      
/** @defgroup FLASH_Flags 
  * @{
  */ 

#define FLASH_FLAG_BSY                 FLASH_SR_BSY     /*!< FLASH Busy flag */
#define FLASH_FLAG_PGERR               FLASH_SR_PGERR   /*!< FLASH Programming error flag */
#define FLASH_FLAG_WRPERR              FLASH_SR_WRPERR  /*!< FLASH Write protected error flag */
#define FLASH_FLAG_EOP                 FLASH_SR_EOP     /*!< FLASH End of Programming flag */
 

/** @defgroup FLASH_Keys 
  * @{
  */ 
#define RDP_KEY                  ((uint16_t)0x00A5)
#define FLASH_KEY1               ((uint32_t)0x45670123)
#define FLASH_KEY2               ((uint32_t)0xCDEF89AB)
#define FLASH_OPTKEY1            FLASH_KEY1
#define FLASH_OPTKEY2            FLASH_KEY2
/**
  * @}
  */
  
/** @defgroup FLASH_Timeout_definition 
  * @{
  */ 

#define FLASH_ER_PRG_TIMEOUT         ((uint32_t)0x000B0000) 

/**
  * @}
  */

/** @defgroup FLASH_Legacy 
  * @{
  */
#define FLASH_WRProt_Pages0to1	       OB_WRP_Pages0to1
#define FLASH_WRProt_Pages2to3	       OB_WRP_Pages2to3
#define FLASH_WRProt_Pages4to5	       OB_WRP_Pages4to5
#define FLASH_WRProt_Pages6to7	       OB_WRP_Pages6to7
#define FLASH_WRProt_Pages8to9	       OB_WRP_Pages8to9
#define FLASH_WRProt_Pages10to11	   OB_WRP_Pages10to11
#define FLASH_WRProt_Pages12to13	   OB_WRP_Pages12to13
#define FLASH_WRProt_Pages14to15	   OB_WRP_Pages14to15
#define FLASH_WRProt_Pages16to17	   OB_WRP_Pages16to17
#define FLASH_WRProt_Pages18to19	   OB_WRP_Pages18to19
#define FLASH_WRProt_Pages20to21	   OB_WRP_Pages20to21
#define FLASH_WRProt_Pages22to23	   OB_WRP_Pages22to23
#define FLASH_WRProt_Pages24to25	   OB_WRP_Pages24to25
#define FLASH_WRProt_Pages26to27	   OB_WRP_Pages26to27
#define FLASH_WRProt_Pages28to29	   OB_WRP_Pages28to29
#define FLASH_WRProt_Pages30to31	   OB_WRP_Pages30to31
#define FLASH_WRProt_Pages32to33	   OB_WRP_Pages32to33
#define FLASH_WRProt_Pages34to35	   OB_WRP_Pages34to35
#define FLASH_WRProt_Pages36to37	   OB_WRP_Pages36to37
#define FLASH_WRProt_Pages38to39	   OB_WRP_Pages38to39
#define FLASH_WRProt_Pages40to41	   OB_WRP_Pages40to41
#define FLASH_WRProt_Pages42to43	   OB_WRP_Pages42to43
#define FLASH_WRProt_Pages44to45	   OB_WRP_Pages44to45
#define FLASH_WRProt_Pages46to47	   OB_WRP_Pages46to47
#define FLASH_WRProt_Pages48to49	   OB_WRP_Pages48to49
#define FLASH_WRProt_Pages50to51	   OB_WRP_Pages50to51
#define FLASH_WRProt_Pages52to53	   OB_WRP_Pages52to53
#define FLASH_WRProt_Pages54to55	   OB_WRP_Pages54to55
#define FLASH_WRProt_Pages56to57	   OB_WRP_Pages56to57
#define FLASH_WRProt_Pages58to59	   OB_WRP_Pages58to59
#define FLASH_WRProt_Pages60to61	   OB_WRP_Pages60to61
#define FLASH_WRProt_Pages62to127	   OB_WRP_Pages62to127

#define FLASH_WRProt_AllPages          OB_WRP_AllPages


/** @defgroup FLASH_Legacy 
  * @{
  */
#define FLASH_EraseOptionBytes               FLASH_OB_Erase
#define FLASH_EnableWriteProtection	         FLASH_OB_EnableWRP
#define FLASH_UserOptionByteConfig	         FLASH_OB_UserConfig
#define FLASH_ProgramOptionByteData          FLASH_OB_ProgramData
#define FLASH_GetUserOptionByte	             FLASH_OB_GetUser
#define FLASH_GetWriteProtectionOptionByte   FLASH_OB_GetWRP
