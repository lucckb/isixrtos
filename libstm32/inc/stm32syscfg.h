/*
 * stm32syscfg.h
 *
 *  Created on: 24 lis 2013
 *      Author: lucck
 */
#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <stm32lib.h>

#if defined(STM32MCU_MAJOR_TYPE_F2) || defined(STM32MCU_MAJOR_TYPE_F4)


#ifdef __cplusplus
namespace stm32 {
#endif

enum SYSCFG_ETH_MediaInterface {
	SYSCFG_ETH_MediaInterface_MII  =  0x00000000,
	SYSCFG_ETH_MediaInterface_RMII =  0x00000001
};

enum SYSCFG_MemoryRemap {
	SYSCFG_MemoryRemap_Flash = 0x00,
	SYSCFG_MemoryRemap_SystemFlash = 0x01,
	SYSCFG_MemoryRemap_FSMC = 0x02,
	SYSCFG_MemoryRemap_SRAM = 0x03
};

//Private defines (undefined later)
#define SYSCFG_OFFSET             (SYSCFG_BASE - PERIPH_BASE)
#define PMC_OFFSET                (SYSCFG_OFFSET + 0x04)
#define MII_RMII_SEL_BitNumber    ((uint8_t)0x17)
#define PMC_MII_RMII_SEL_BB       (PERIPH_BB_BASE + (PMC_OFFSET * 32) + (MII_RMII_SEL_BitNumber * 4))
/* Alias word address of CMP_PD bit */
#define CMPCR_OFFSET              (SYSCFG_OFFSET + 0x20)
#define CMP_PD_BitNumber          ((uint8_t)0x00)
#define CMPCR_CMP_PD_BB           (PERIPH_BB_BASE + (CMPCR_OFFSET * 32) + (CMP_PD_BitNumber * 4))

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

/**
  * @brief  Checks whether the I/O Compensation Cell ready flag is set or not.
  * @param  None
  * @retval The new state of the I/O Compensation Cell ready flag (SET or RESET)
  */
static inline bool syscfg_get_compensation_cell_status(void)
{
  return ((SYSCFG->CMPCR & SYSCFG_CMPCR_READY ) != (uint32_t)0);
}

//Undefs
#undef SYSCFG_OFFSET
#undef PMC_OFFSET
#undef MII_RMII_SEL_BitNumber
#undef PMC_MII_RMII_SEL_BB
#undef CMPCR_OFFSET
#undef CMP_PD_BitNumber
#undef CMPCR_CMP_PD_BB

#ifdef __cplusplus
}
#endif
#elif defined(STM32MCU_MAJOR_TYPE_F37)
/**
  * @brief  Configures the memory mapping at address 0x00000000.
  * @param  SYSCFG_MemoryRemap: selects the memory remapping.
  *          This parameter can be one of the following values:
  *            @arg SYSCFG_MemoryRemap_Flash: Main Flash memory mapped at 0x00000000  
  *            @arg SYSCFG_MemoryRemap_SystemMemory: System Flash memory mapped at 0x00000000
  *            @arg SYSCFG_MemoryRemap_SRAM: Embedded SRAM mapped at 0x00000000
  * @retval None
  */
static inline void syscfg_memory_remap_config(uint32_t SYSCFG_MemoryRemap)
{
  uint32_t tmpctrl = 0;

  /* Get CFGR1 register value */
  tmpctrl = SYSCFG->CFGR1;

  /* Clear MEM_MODE bits */
  tmpctrl &= (uint32_t) (~SYSCFG_CFGR1_MEM_MODE);

  /* Set the new MEM_MODE bits value */
  tmpctrl |= (uint32_t) SYSCFG_MemoryRemap;

  /* Set CFGR1 register with the new memory remap configuration */
  SYSCFG->CFGR1 = tmpctrl;
}

/**
  * @brief  Configure the DMA channels remapping.
  * @param  SYSCFG_DMARemap: selects the DMA channels remap.
  *          This parameter can be one of the following values:
  *            @arg SYSCFG_DMARemap_TIM17: Remap TIM17 DMA1 requests from channel1 to channel2
  *            @arg SYSCFG_DMARemap_TIM16: Remap TIM16 DMA1 requests from channel3 to channel4
  *            @arg SYSCFG_DMARemap_TIM6DAC1Ch1: Remap TIM6/DAC1 channel1 DMA requests from DMA2 channel 3 to DMA1 channel 3
  *            @arg SYSCFG_DMARemap_TIM7DAC1Ch2: Remap TIM7/DAC1 channel2 DMA requests from DMA2 channel 4 to DMA1 channel 4
  *            @arg SYSCFG_DMARemap_TIM18DAC2Ch1: Remap TIM18/DAC2 channel1 DMA requests from DMA2 channel 5 to DMA1 channel 5
  * @param  NewState: new state of the DMA channel remapping. 
  *         This parameter can be: ENABLE or DISABLE.
  * @note   When enabled, DMA channel of the selected peripheral is remapped
  * @note   When disabled, Default DMA channel is mapped to the selected peripheral
  * @note   By default TIM17 DMA requests is mapped to channel 1, 
  *         use SYSCFG_DMAChannelRemapConfig(SYSCFG_DMARemap_TIM17, Enable) to remap
  *         TIM17 DMA requests to channel 2 and use
  *         SYSCFG_DMAChannelRemapConfig(SYSCFG_DMARemap_TIM17, Disable) to map
  *         TIM17 DMA requests to channel 1 (default mapping)
  * @retval None
  */
static inline void syscfg_dma_channel_remap_config(uint32_t SYSCFG_DMARemap, bool en )
{
  if ( en )
  {
    /* Remap the DMA channel */
    SYSCFG->CFGR1 |= (uint32_t)SYSCFG_DMARemap;
  }
  else
  {
    /* use the default DMA channel mapping */
    SYSCFG->CFGR1 &= (uint32_t)(~SYSCFG_DMARemap);
  }
}

/**
  * @brief  Configure the I2C fast mode plus driving capability.
  * @param  SYSCFG_I2CFastModePlus: selects the pin.
  *          This parameter can be one of the following values:
  *            @arg SYSCFG_I2CFastModePlus_PB6: Configure fast mode plus driving capability for PB6
  *            @arg SYSCFG_I2CFastModePlus_PB7: Configure fast mode plus driving capability for PB7
  *            @arg SYSCFG_I2CFastModePlus_PB8: Configure fast mode plus driving capability for PB8
  *            @arg SYSCFG_I2CFastModePlus_PB9: Configure fast mode plus driving capability for PB9
  *            @arg SYSCFG_I2CFastModePlus_I2C1: Configure fast mode plus driving capability for I2C1 pins
  *            @arg SYSCFG_I2CFastModePlus_I2C2: Configure fast mode plus driving capability for I2C2 pins
  * @param  NewState: new state of the DMA channel remapping. 
  *         This parameter can be: ENABLE or DISABLE.
  * @note   ENABLE: Enable fast mode plus driving capability for selected I2C pin
  * @note   DISABLE: Disable fast mode plus driving capability for selected I2C pin
  * @note  For I2C1, fast mode plus driving capability can be enabled on all selected
  *        I2C1 pins using SYSCFG_I2CFastModePlus_I2C1 parameter or independently
  *        on each one of the following pins PB6, PB7, PB8 and PB9.
  * @note  For remaining I2C1 pins (PA14, PA15...) fast mode plus driving capability
  *        can be enabled only by using SYSCFG_I2CFastModePlus_I2C1 parameter
  *        For all I2C2 pins fast mode plus driving capability can be enabled
  *        only by using SYSCFG_I2CFastModePlus_I2C2 parameter
  * @retval None
  */
static inline void syscfg_i2c_fast_mode_plus_config(uint32_t SYSCFG_I2CFastModePlus, bool en )
{
  if ( en )
  {
    /* Enable fast mode plus driving capability for selected I2C pin */
    SYSCFG->CFGR1 |= (uint32_t)SYSCFG_I2CFastModePlus;
  }
  else
  {
    /* Disable fast mode plus driving capability for selected I2C pin */
    SYSCFG->CFGR1 &= (uint32_t)(~SYSCFG_I2CFastModePlus);
  }
}


/**
  * @brief Control the VBAT monitoring
  * @param  NewState: new state of the VBAT connection to ADC channel 18. 
  *         This parameter can be: ENABLE or DISABLE.
  * @note   ENABLE: Enable VBAT monitoring by connecting internally VBAT to ADC channel 18
  * @note   DISABLE: Disable VBAT monitoring by disconnecting VBAT from ADC channel 18
  * @retval None
  */
static inline void syscfg_vbat_monitoring_cmd( bool en )
{
  if ( en )
  {
    /* Enable VBAT monitoring */
    SYSCFG->CFGR1 |= SYSCFG_CFGR1_VBAT;
  }
  else
  {
    /* Disable VBAT monitoring */
    SYSCFG->CFGR1 &= ((uint32_t)~SYSCFG_CFGR1_VBAT);
  }
}

/**
  * @brief  Enables or disables the selected SYSCFG interrupts.
  * @param  SYSCFG_IT: specifies the SYSCFG interrupt sources to be enabled or disabled.
  *          This parameter can be one of the following values:
  *           @arg SYSCFG_IT_IXC: Inexact Interrupt
  *           @arg SYSCFG_IT_IDC: Input denormal Interrupt
  *           @arg SYSCFG_IT_OFC: Overflow Interrupt
  *           @arg SYSCFG_IT_UFC: Underflow Interrupt
  *           @arg SYSCFG_IT_DZC: Divide-by-zero Interrupt
  *           @arg SYSCFG_IT_IOC: Invalid operation Interrupt
  * @param  NewState: new state of the specified SDADC interrupts.
  *          This parameter can be: ENABLE or DISABLE.
  * @retval None
  */
static inline void syscfg_it_config(uint32_t SYSCFG_IT,  bool en )
{
  if ( en )
  {
    /* Enable the selected SYSCFG interrupts */
    SYSCFG->CFGR1 |= SYSCFG_IT;
  }
  else
  {
    /* Disable the selected SYSCFG interrupts */
    SYSCFG->CFGR1 &= ((uint32_t)~SYSCFG_IT);
  }
}

/**
  * @brief  Connect the selected parameter to the break input of TIM15/TIM16/TIM17.
  * @note   The selected configuration is locked and can be unlocked by system reset
  * @param  SYSCFG_Break: selects the configuration to be connected to break
  *         input of TIM15/TIM16/TIM17
  *          This parameter can be any combination of the following values:
  *            @arg SYSCFG_Break_PVD: Connects the PVD event to the Break Input of TIM15/TIM16/TIM17.
  *            @arg SYSCFG_Break_SRAMParity: Connects the SRAM_PARITY error signal to the Break Input of TIM15/TIM16/TIM17 .
  *            @arg SYSCFG_Break_Lockup: Connects Lockup output of CortexM4 to the break input of TIM15/TIM16/TIM17.
  * @retval None
  */
static inline void syscfg_break_config(uint32_t SYSCFG_Break)
{
  SYSCFG->CFGR2 |= (uint32_t) SYSCFG_Break;
}


/**
  * @brief  Checks whether the specified SYSCFG flag is set or not.
  * @param  SYSCFG_Flag: specifies the SYSCFG flag to check. 
  *          This parameter can be one of the following values:
  *            @arg SYSCFG_FLAG_PE: SRAM parity error flag.
  * @retval The new state of SYSCFG_Flag (SET or RESET).
  */
static inline bool syscfg_get_flag_status(uint32_t SYSCFG_Flag)
{
  /* Check the status of the specified SPI flag */
  return ((SYSCFG->CFGR2 & SYSCFG_Flag) != (uint32_t)0);
}


/**
  * @brief  Clear the selected SYSCFG flag.
  * @param  SYSCFG_Flag: selects the flag to be cleared.
  *          This parameter can be any combination of the following values:
  *            @arg SYSCFG_FLAG_PE: SRAM parity error flag.
  * @retval None
  */
static inline void syscfg_clear_flag(uint32_t SYSCFG_Flag)
{
  SYSCFG->CFGR2 |= (uint32_t) SYSCFG_Flag;
}

#endif /* defined(STM32MCU_MAJOR_TYPE_F2) || STM32MCU_MAJOR_TYPE_F4) */
