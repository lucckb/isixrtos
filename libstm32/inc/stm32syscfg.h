/*
 * stm32syscfg.h
 *
 *  Created on: 24 lis 2013
 *      Author: lucck
 */

#ifndef STM32SYSCFG_H_
#define STM32SYSCFG_H_
#if defined(STM32MCU_MAJOR_TYPE_F2) || defined(STM32MCU_MAJOR_TYPE_F4)
/*----------------------------------------------------------*/
#include <stdint.h>
#include <stdbool.h>
#include "stm32lib.h"
#include "stm32fxxx_spi.h"
#include <stm32lib.h>
/*----------------------------------------------------------*/
#ifdef __cplusplus
namespace stm32 {
#endif
/*----------------------------------------------------------*/
enum SYSCFG_ETH_MediaInterface {
	SYSCFG_ETH_MediaInterface_MII  =  0x00000000,
	SYSCFG_ETH_MediaInterface_RMII =  0x00000001
};
/*----------------------------------------------------------*/
enum SYSCFG_MemoryRemap {
	SYSCFG_MemoryRemap_Flash = 0x00,
	SYSCFG_MemoryRemap_SystemFlash = 0x01,
	SYSCFG_MemoryRemap_FSMC = 0x02,
	SYSCFG_MemoryRemap_SRAM = 0x03
};
/*----------------------------------------------------------*/
//Private defines (undefined later)
#define SYSCFG_OFFSET             (SYSCFG_BASE - PERIPH_BASE)
#define PMC_OFFSET                (SYSCFG_OFFSET + 0x04)
#define MII_RMII_SEL_BitNumber    ((uint8_t)0x17)
#define PMC_MII_RMII_SEL_BB       (PERIPH_BB_BASE + (PMC_OFFSET * 32) + (MII_RMII_SEL_BitNumber * 4))
/* Alias word address of CMP_PD bit */
#define CMPCR_OFFSET              (SYSCFG_OFFSET + 0x20)
#define CMP_PD_BitNumber          ((uint8_t)0x00)
#define CMPCR_CMP_PD_BB           (PERIPH_BB_BASE + (CMPCR_OFFSET * 32) + (CMP_PD_BitNumber * 4))
/*----------------------------------------------------------*/
/**
  * @brief  Changes the mapping of the specified pin.
  * @param  SYSCFG_Memory: selects the memory remapping.
  *         This parameter can be one of the following values:
  *            @arg SYSCFG_MemoryRemap_Flash:       Main Flash memory mapped at 0x00000000
  *            @arg SYSCFG_MemoryRemap_SystemFlash: System Flash memory mapped at 0x00000000
  *            @arg SYSCFG_MemoryRemap_FSMC:        FSMC (Bank1 (NOR/PSRAM 1 and 2) mapped at 0x00000000
  *            @arg SYSCFG_MemoryRemap_SRAM:        Embedded SRAM (112kB) mapped at 0x00000000
  * @retval None
  */
static inline void syscfg_memory_remap_config(uint8_t SYSCFG_MemoryRemap)
{

  SYSCFG->MEMRMP = SYSCFG_MemoryRemap;
}
/*----------------------------------------------------------*/
/**
  * @brief  Selects the ETHERNET media interface
  * @param  SYSCFG_ETH_MediaInterface: specifies the Media Interface mode.
  *          This parameter can be one of the following values:
  *            @arg SYSCFG_ETH_MediaInterface_MII: MII mode selected
  *            @arg SYSCFG_ETH_MediaInterface_RMII: RMII mode selected
  * @retval None
  */
static inline void syscfg_eth_media_interface_config(uint32_t SYSCFG_ETH_MediaInterface)
{
  /* Configure MII_RMII selection bit */
  *(__IO uint32_t *) PMC_MII_RMII_SEL_BB = SYSCFG_ETH_MediaInterface;
}
/*----------------------------------------------------------*/
/**
  * @brief  Enables or disables the I/O Compensation Cell.
  * @note   The I/O compensation cell can be used only when the device supply
  *         voltage ranges from 2.4 to 3.6 V.
  * @param  NewState: new state of the I/O Compensation Cell.
  *          This parameter can be one of the following values:
  *            @arg ENABLE: I/O compensation cell enabled
  *            @arg DISABLE: I/O compensation cell power-down mode
  * @retval None
  */
static inline void syscfg_compensation_cell_cmd( bool enabled )
{

  *(__IO uint32_t *) CMPCR_CMP_PD_BB = (uint32_t)enabled;
}
/*----------------------------------------------------------*/
/**
  * @brief  Checks whether the I/O Compensation Cell ready flag is set or not.
  * @param  None
  * @retval The new state of the I/O Compensation Cell ready flag (SET or RESET)
  */
static inline bool syscfg_get_compensation_cell_status(void)
{
  return ((SYSCFG->CMPCR & SYSCFG_CMPCR_READY ) != (uint32_t)0);
}
/*----------------------------------------------------------*/
//Undefs
#undef SYSCFG_OFFSET
#undef PMC_OFFSET
#undef MII_RMII_SEL_BitNumber
#undef PMC_MII_RMII_SEL_BB
#undef CMPCR_OFFSET
#undef CMP_PD_BitNumber
#undef CMPCR_CMP_PD_BB
/*----------------------------------------------------------*/
#ifdef __cplusplus
}
#endif
/*----------------------------------------------------------*/
#endif /* defined(STM32MCU_MAJOR_TYPE_F2) || STM32MCU_MAJOR_TYPE_F4) */
#endif /* STM32SYSCFG_H_ */
