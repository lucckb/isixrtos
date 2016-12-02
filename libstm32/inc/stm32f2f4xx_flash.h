/*
 * =====================================================================================
 *
 *       Filename:  stm32f2f4xx_flash.h
 *
 *    Description:  STM32 flash programming interface
 *
 *        Version:  1.0
 *        Created:  02.10.2014 21:59:51
 *       Revision:  none
 *       Compiler:  gcc
 *
 *
 * =====================================================================================
 */
#pragma once 

#include "stm32f2f4xx_flash_defs.h"
#include "stm32lib.h"

#ifdef __cplusplus
namespace stm32 {
#endif
#define SECTOR_MASK               ((uint32_t)0xFFFFFF07)

/**
  * @brief  Sets the code latency value.
  * @param  FLASH_Latency: specifies the FLASH Latency value.
  *          This parameter can be one of the following values:
  *            @arg FLASH_Latency_0: FLASH Zero Latency cycle
  *            @arg FLASH_Latency_1: FLASH One Latency cycle
  *            @arg FLASH_Latency_2: FLASH Two Latency cycles
  *            @arg FLASH_Latency_3: FLASH Three Latency cycles
  *            @arg FLASH_Latency_4: FLASH Four Latency cycles 
  *            @arg FLASH_Latency_5: FLASH Five Latency cycles 
  *            @arg FLASH_Latency_6: FLASH Six Latency cycles
  *            @arg FLASH_Latency_7: FLASH Seven Latency cycles 
  * @retval None
  */
static inline void flash_set_latency(uint32_t flash_latency)
{
  /* Perform Byte access to FLASH_ACR[8:0] to set the Latency value */
  *(__IO uint8_t *)ACR_BYTE0_ADDRESS = (uint8_t)flash_latency;
}

static inline void flash_prefetch_buffer_cmd( bool en )
{
  
  /* Enable or disable the Prefetch Buffer */
  if( en )
  {
	FLASH->ACR |= FLASH_ACR_PRFTEN;
  }
  else
  {
	FLASH->ACR &= (~FLASH_ACR_PRFTEN);
  }
}

/*
  * @brief  Enables or disables the Instruction Cache feature.
  * @param  NewState: new state of the Instruction Cache.
  *          This parameter  can be: ENABLE or DISABLE.
  * @retval None
  */
static inline void flash_instruction_cache_cmd( bool  en )
{
  if( en )
  {
    FLASH->ACR |= FLASH_ACR_ICEN;
  }
  else
  {
    FLASH->ACR &= (~FLASH_ACR_ICEN);
  }
}

/**
  * @brief  Enables or disables the Data Cache feature.
  * @param  NewState: new state of the Data Cache.
  *          This parameter  can be: ENABLE or DISABLE.
  * @retval None
  */
static inline void flash_data_cache_cmd( bool en)
{
  
  if( en )
  {
    FLASH->ACR |= FLASH_ACR_DCEN;
  }
  else
  {
    FLASH->ACR &= (~FLASH_ACR_DCEN);
  }
}

/**
  * @brief  Resets the Instruction Cache.
  * @note   This function must be used only when the Instruction Cache is disabled.  
  * @param  None
  * @retval None
  */
static inline void flash_instruction_cache_reset(void)
{
  FLASH->ACR |= FLASH_ACR_ICRST;
}

/**
  * @brief  Resets the Data Cache.
  * @note   This function must be used only when the Data Cache is disabled.  
  * @param  None
  * @retval None
  */
static inline void flash_data_cache_reset(void)
{
  FLASH->ACR |= FLASH_ACR_DCRST;
}

/*
  * @brief  Unlocks the FLASH control register access
  * @param  None
  * @retval None
  */
static inline void flash_unlock(void)
{
  if((FLASH->CR & FLASH_CR_LOCK) != 0)
  {
    /* Authorize the FLASH Registers access */
    FLASH->KEYR = FLASH_KEY1;
    FLASH->KEYR = FLASH_KEY2;
  }  
}

/**
  * @brief  Locks the FLASH control register access
  * @param  None
  * @retval None
  */
static inline void flash_lock(void)
{
  /* Set the LOCK Bit to lock the FLASH Registers access */
  FLASH->CR |= FLASH_CR_LOCK;
}
/* 
  * @brief  Returns the FLASH Status.
  * @param  None
  * @retval FLASH Status: The returned value can be: FLASH_BUSY, FLASH_ERROR_PROGRAM,
  *                       FLASH_ERROR_WRP, FLASH_ERROR_OPERATION or FLASH_COMPLETE.
  */
static inline FLASH_Status flash_get_status(void)
{
  FLASH_Status flashstatus = FLASH_COMPLETE;
  
  if((FLASH->SR & FLASH_FLAG_BSY) == FLASH_FLAG_BSY) 
  {
    flashstatus = FLASH_BUSY;
  }
  else 
  {  
    if((FLASH->SR & FLASH_FLAG_WRPERR) != (uint32_t)0x00)
    { 
      flashstatus = FLASH_ERROR_WRP;
    }
    else 
    {
      if((FLASH->SR & (uint32_t)0xEF) != (uint32_t)0x00)
      {
        flashstatus = FLASH_ERROR_PROGRAM; 
      }
      else
      {
        if((FLASH->SR & FLASH_FLAG_OPERR) != (uint32_t)0x00)
        {
          flashstatus = FLASH_ERROR_OPERATION;
        }
        else
        {
          flashstatus = FLASH_COMPLETE;
        }
      }
    }
  }
  /* Return the FLASH Status */
  return flashstatus;
}

/**
  * @brief  Waits for a FLASH operation to complete.
  * @param  None
  * @retval FLASH Status: The returned value can be: FLASH_BUSY, FLASH_ERROR_PROGRAM,
  *                       FLASH_ERROR_WRP, FLASH_ERROR_OPERATION or FLASH_COMPLETE.
  */
static inline FLASH_Status flash_wait_for_last_operation(void)
{ 
  __IO FLASH_Status status = FLASH_COMPLETE;
   
  /* Check for the FLASH Status */
  status = flash_get_status();

  /* Wait for the FLASH operation to complete by polling on BUSY flag to be reset.
     Even if the FLASH operation fails, the BUSY flag will be reset and an error
     flag will be set */
  while(status == FLASH_BUSY)
  {
    status = flash_get_status();
  }
  /* Return the operation status */
  return status;
}

/**
  * @brief  Erases a specified FLASH Sector.
  *   
  * @param  FLASH_Sector: The Sector number to be erased.
  *          This parameter can be a value between FLASH_Sector_0 and FLASH_Sector_11
  *    
  * @param  VoltageRange: The device voltage range which defines the erase parallelism.  
  *          This parameter can be one of the following values:
  *            @arg VoltageRange_1: when the device voltage range is 1.8V to 2.1V, 
  *                                  the operation will be done by byte (8-bit) 
  *            @arg VoltageRange_2: when the device voltage range is 2.1V to 2.7V,
  *                                  the operation will be done by half word (16-bit)
  *            @arg VoltageRange_3: when the device voltage range is 2.7V to 3.6V,
  *                                  the operation will be done by word (32-bit)
  *            @arg VoltageRange_4: when the device voltage range is 2.7V to 3.6V + External Vpp, 
  *                                  the operation will be done by double word (64-bit)
  *       
  * @retval FLASH Status: The returned value can be: FLASH_BUSY, FLASH_ERROR_PROGRAM,
  *                       FLASH_ERROR_WRP, FLASH_ERROR_OPERATION or FLASH_COMPLETE.
  */
static inline FLASH_Status flash_erase_sector(uint32_t FLASH_Sector, uint8_t VoltageRange)
{
  uint32_t tmp_psize = 0x0;
  FLASH_Status status = FLASH_COMPLETE;

  
  if(VoltageRange == VoltageRange_1)
  {
     tmp_psize = FLASH_PSIZE_BYTE;
  }
  else if(VoltageRange == VoltageRange_2)
  {
    tmp_psize = FLASH_PSIZE_HALF_WORD;
  }
  else if(VoltageRange == VoltageRange_3)
  {
    tmp_psize = FLASH_PSIZE_WORD;
  }
  else
  {
    tmp_psize = FLASH_PSIZE_DOUBLE_WORD;
  }
  /* Wait for last operation to be completed */
  status = flash_wait_for_last_operation();
  
  if(status == FLASH_COMPLETE)
  { 
    /* if the previous operation is completed, proceed to erase the sector */
    FLASH->CR &= CR_PSIZE_MASK;
    FLASH->CR |= tmp_psize;
    FLASH->CR &= SECTOR_MASK;
    FLASH->CR |= FLASH_CR_SER | FLASH_Sector;
    FLASH->CR |= FLASH_CR_STRT;
    
    /* Wait for last operation to be completed */
    status = flash_wait_for_last_operation();
    
    /* if the erase operation is completed, disable the SER Bit */
    FLASH->CR &= (~FLASH_CR_SER);
    FLASH->CR &= SECTOR_MASK; 
  }
  /* Return the Erase Status */
  return status;
}

/**
  * @brief  Erases all FLASH Sectors.
  *    
  * @param  VoltageRange: The device voltage range which defines the erase parallelism.  
  *          This parameter can be one of the following values:
  *            @arg VoltageRange_1: when the device voltage range is 1.8V to 2.1V, 
  *                                  the operation will be done by byte (8-bit) 
  *            @arg VoltageRange_2: when the device voltage range is 2.1V to 2.7V,
  *                                  the operation will be done by half word (16-bit)
  *            @arg VoltageRange_3: when the device voltage range is 2.7V to 3.6V,
  *                                  the operation will be done by word (32-bit)
  *            @arg VoltageRange_4: when the device voltage range is 2.7V to 3.6V + External Vpp, 
  *                                  the operation will be done by double word (64-bit)
  *       
  * @retval FLASH Status: The returned value can be: FLASH_BUSY, FLASH_ERROR_PROGRAM,
  *                       FLASH_ERROR_WRP, FLASH_ERROR_OPERATION or FLASH_COMPLETE.
  */
static inline FLASH_Status flash_erase_all_sectors(uint8_t VoltageRange)
{
  uint32_t tmp_psize = 0x0;
  FLASH_Status status = FLASH_COMPLETE;
  
  /* Wait for last operation to be completed */
  status = flash_wait_for_last_operation();
  
  if(VoltageRange == VoltageRange_1)
  {
     tmp_psize = FLASH_PSIZE_BYTE;
  }
  else if(VoltageRange == VoltageRange_2)
  {
    tmp_psize = FLASH_PSIZE_HALF_WORD;
  }
  else if(VoltageRange == VoltageRange_3)
  {
    tmp_psize = FLASH_PSIZE_WORD;
  }
  else
  {
    tmp_psize = FLASH_PSIZE_DOUBLE_WORD;
  }  
  if(status == FLASH_COMPLETE)
  {
    /* if the previous operation is completed, proceed to erase all sectors */
     FLASH->CR &= CR_PSIZE_MASK;
     FLASH->CR |= tmp_psize;
     FLASH->CR |= FLASH_CR_MER;
     FLASH->CR |= FLASH_CR_STRT;
    
    /* Wait for last operation to be completed */
    status = flash_wait_for_last_operation();

    /* if the erase operation is completed, disable the MER Bit */
    FLASH->CR &= (~FLASH_CR_MER);

  }   
  /* Return the Erase Status */
  return status;
}

/**
  * @brief  Programs a double word (64-bit) at a specified address.
  * @note   This function must be used when the device voltage range is from
  *         2.7V to 3.6V and an External Vpp is present.           
  * @param  Address: specifies the address to be programmed.
  * @param  Data: specifies the data to be programmed.
  * @retval FLASH Status: The returned value can be: FLASH_BUSY, FLASH_ERROR_PROGRAM,
  *                       FLASH_ERROR_WRP, FLASH_ERROR_OPERATION or FLASH_COMPLETE.
  */
static inline FLASH_Status flash_program_double_word(uint32_t Address, uint64_t Data)
{
  FLASH_Status status = FLASH_COMPLETE;


  /* Wait for last operation to be completed */
  status = flash_wait_for_last_operation();
  
  if(status == FLASH_COMPLETE)
  {
    /* if the previous operation is completed, proceed to program the new data */
    FLASH->CR &= CR_PSIZE_MASK;
    FLASH->CR |= FLASH_PSIZE_DOUBLE_WORD;
    FLASH->CR |= FLASH_CR_PG;
  
    *(__IO uint64_t*)Address = Data;
        
    /* Wait for last operation to be completed */
    status = flash_wait_for_last_operation();

    /* if the program operation is completed, disable the PG Bit */
    FLASH->CR &= (~FLASH_CR_PG);
  } 
  /* Return the Program Status */
  return status;
}

/**
  * @brief  Programs a word (32-bit) at a specified address.
  * @param  Address: specifies the address to be programmed.
  *         This parameter can be any address in Program memory zone or in OTP zone.  
  * @note   This function must be used when the device voltage range is from 2.7V to 3.6V. 
  * @param  Data: specifies the data to be programmed.
  * @retval FLASH Status: The returned value can be: FLASH_BUSY, FLASH_ERROR_PROGRAM,
  *                       FLASH_ERROR_WRP, FLASH_ERROR_OPERATION or FLASH_COMPLETE.
  */
static inline FLASH_Status flash_program_word(uint32_t Address, uint32_t Data)
{
  FLASH_Status status = FLASH_COMPLETE;


  /* Wait for last operation to be completed */
  status = flash_wait_for_last_operation();
  
  if(status == FLASH_COMPLETE)
  {
    /* if the previous operation is completed, proceed to program the new data */
    FLASH->CR &= CR_PSIZE_MASK;
    FLASH->CR |= FLASH_PSIZE_WORD;
    FLASH->CR |= FLASH_CR_PG;
  
    *(__IO uint32_t*)Address = Data;
        
    /* Wait for last operation to be completed */
    status = flash_wait_for_last_operation();

    /* if the program operation is completed, disable the PG Bit */
    FLASH->CR &= (~FLASH_CR_PG);
  } 
  /* Return the Program Status */
  return status;
}

/**
  * @brief  Programs a half word (16-bit) at a specified address. 
  * @note   This function must be used when the device voltage range is from 2.1V to 3.6V.               
  * @param  Address: specifies the address to be programmed.
  *         This parameter can be any address in Program memory zone or in OTP zone.  
  * @param  Data: specifies the data to be programmed.
  * @retval FLASH Status: The returned value can be: FLASH_BUSY, FLASH_ERROR_PROGRAM,
  *                       FLASH_ERROR_WRP, FLASH_ERROR_OPERATION or FLASH_COMPLETE.
  */
static inline FLASH_Status flash_program_half_word(uint32_t Address, uint16_t Data)
{
  FLASH_Status status = FLASH_COMPLETE;


  /* Wait for last operation to be completed */
  status = flash_wait_for_last_operation();
  
  if(status == FLASH_COMPLETE)
  {
    /* if the previous operation is completed, proceed to program the new data */
    FLASH->CR &= CR_PSIZE_MASK;
    FLASH->CR |= FLASH_PSIZE_HALF_WORD;
    FLASH->CR |= FLASH_CR_PG;
  
    *(__IO uint16_t*)Address = Data;
        
    /* Wait for last operation to be completed */
    status = flash_wait_for_last_operation();

    /* if the program operation is completed, disable the PG Bit */
    FLASH->CR &= (~FLASH_CR_PG);
  } 
  /* Return the Program Status */
  return status;
}

/**
  * @brief  Programs a byte (8-bit) at a specified address.
  * @note   This function can be used within all the device supply voltage ranges.
  * @param  Address: specifies the address to be programmed.
  *         This parameter can be any address in Program memory zone or in OTP zone.
  * @param  Data: specifies the data to be programmed.
  * @retval FLASH Status: The returned value can be: FLASH_BUSY, FLASH_ERROR_PROGRAM,
  *                       FLASH_ERROR_WRP, FLASH_ERROR_OPERATION or FLASH_COMPLETE.
  */
static inline FLASH_Status flash_program_byte(uint32_t Address, uint8_t Data)
{
  FLASH_Status status = FLASH_COMPLETE;


  /* Wait for last operation to be completed */
  status = flash_wait_for_last_operation();
  
  if(status == FLASH_COMPLETE)
  {
    /* if the previous operation is completed, proceed to program the new data */
    FLASH->CR &= CR_PSIZE_MASK;
    FLASH->CR |= FLASH_PSIZE_BYTE;
    FLASH->CR |= FLASH_CR_PG;
  
    *(__IO uint8_t*)Address = Data;
        
    /* Wait for last operation to be completed */
    status = flash_wait_for_last_operation();

    /* if the program operation is completed, disable the PG Bit */
    FLASH->CR &= (~FLASH_CR_PG);
  } 

  /* Return the Program Status */
  return status;
}

/**
  * @brief  Unlocks the FLASH Option Control Registers access.
  * @param  None
  * @retval None
  */
static inline void flash_ob_unlock(void)
{
  if((FLASH->OPTCR & FLASH_OPTCR_OPTLOCK) != 0)
  {
    /* Authorizes the Option Byte register programming */
    FLASH->OPTKEYR = FLASH_OPT_KEY1;
    FLASH->OPTKEYR = FLASH_OPT_KEY2;
  }  
}

/**
  * @brief  Locks the FLASH Option Control Registers access.
  * @param  None
  * @retval None
  */
static inline void flash_ob_lock(void)
{
  /* Set the OPTLOCK Bit to lock the FLASH Option Byte Registers access */
  FLASH->OPTCR |= FLASH_OPTCR_OPTLOCK;
}

/**
  * @brief  Enables or disables the write protection of the desired sectors
  * @param  OB_WRP: specifies the sector(s) to be write protected or unprotected.
  *          This parameter can be one of the following values:
  *            @arg OB_WRP: A value between OB_WRP_Sector0 and OB_WRP_Sector11                      
  *            @arg OB_WRP_Sector_All
  * @param  Newstate: new state of the Write Protection.
  *          This parameter can be: ENABLE or DISABLE.
  * @retval None  
  */
static inline void flash_ob_wrp_config(uint32_t OB_WRP, bool en )
{ 
  FLASH_Status status = FLASH_COMPLETE;
  
  status = flash_wait_for_last_operation();

  if(status == FLASH_COMPLETE)
  { 
    if( en )
    {
      *(__IO uint16_t*)OPTCR_BYTE2_ADDRESS &= (~OB_WRP);
    }
    else
    {
      *(__IO uint16_t*)OPTCR_BYTE2_ADDRESS |= (uint16_t)OB_WRP;
    }
  }
}

/**
  * @brief  Sets the read protection level.
  * @param  OB_RDP: specifies the read protection level.
  *          This parameter can be one of the following values:
  *            @arg OB_RDP_Level_0: No protection
  *            @arg OB_RDP_Level_1: Read protection of the memory
  *            @arg OB_RDP_Level_2: Full chip protection
  *   
  * !!!Warning!!! When enabling OB_RDP level 2 it's no more possible to go back to level 1 or 0
  *    
  * @retval None
  */
static inline void flash_ob_rdp_config(uint8_t OB_RDP)
{
  FLASH_Status status = FLASH_COMPLETE;

  status = flash_wait_for_last_operation();

  if(status == FLASH_COMPLETE)
  {
    *(__IO uint8_t*)OPTCR_BYTE1_ADDRESS = OB_RDP;

  }
}

/**
  * @brief  Programs the FLASH User Option Byte: IWDG_SW / RST_STOP / RST_STDBY.    
  * @param  OB_IWDG: Selects the IWDG mode
  *          This parameter can be one of the following values:
  *            @arg OB_IWDG_SW: Software IWDG selected
  *            @arg OB_IWDG_HW: Hardware IWDG selected
  * @param  OB_STOP: Reset event when entering STOP mode.
  *          This parameter  can be one of the following values:
  *            @arg OB_STOP_NoRST: No reset generated when entering in STOP
  *            @arg OB_STOP_RST: Reset generated when entering in STOP
  * @param  OB_STDBY: Reset event when entering Standby mode.
  *          This parameter  can be one of the following values:
  *            @arg OB_STDBY_NoRST: No reset generated when entering in STANDBY
  *            @arg OB_STDBY_RST: Reset generated when entering in STANDBY
  * @retval None
  */
static inline void flash_ob_user_config(uint8_t OB_IWDG, uint8_t OB_STOP, uint8_t OB_STDBY)
{
  uint8_t optiontmp = 0xFF;
  FLASH_Status status = FLASH_COMPLETE; 

  /* Wait for last operation to be completed */
  status = flash_wait_for_last_operation();
  
  if(status == FLASH_COMPLETE)
  { 
    /* Mask OPTLOCK, OPTSTRT and BOR_LEV bits */
    optiontmp =  (uint8_t)((*(__IO uint8_t *)OPTCR_BYTE0_ADDRESS) & (uint8_t)0x0F); 

    /* Update User Option Byte */
    *(__IO uint8_t *)OPTCR_BYTE0_ADDRESS = OB_IWDG | (uint8_t)(OB_STDBY | (uint8_t)(OB_STOP | ((uint8_t)optiontmp))); 
  }  
}

/**
  * @brief  Sets the BOR Level. 
  * @param  OB_BOR: specifies the Option Bytes BOR Reset Level.
  *          This parameter can be one of the following values:
  *            @arg OB_BOR_LEVEL3: Supply voltage ranges from 2.7 to 3.6 V
  *            @arg OB_BOR_LEVEL2: Supply voltage ranges from 2.4 to 2.7 V
  *            @arg OB_BOR_LEVEL1: Supply voltage ranges from 2.1 to 2.4 V
  *            @arg OB_BOR_OFF: Supply voltage ranges from 1.62 to 2.1 V
  * @retval None
  */
static inline void flash_ob_bor_config(uint8_t OB_BOR)
{
  /* Set the BOR Level */
  *(__IO uint8_t *)OPTCR_BYTE0_ADDRESS &= (~FLASH_OPTCR_BOR_LEV);
  *(__IO uint8_t *)OPTCR_BYTE0_ADDRESS |= OB_BOR;

}

/**
  * @brief  Launch the option byte loading.
  * @param  None
  * @retval FLASH Status: The returned value can be: FLASH_BUSY, FLASH_ERROR_PROGRAM,
  *                       FLASH_ERROR_WRP, FLASH_ERROR_OPERATION or FLASH_COMPLETE.
  */
static inline FLASH_Status flash_ob_launch(void)
{
  FLASH_Status status = FLASH_COMPLETE;

  /* Set the OPTSTRT bit in OPTCR register */
  *(__IO uint8_t *)OPTCR_BYTE0_ADDRESS |= FLASH_OPTCR_OPTSTRT;

  /* Wait for last operation to be completed */
  status = flash_wait_for_last_operation();

  return status;
}

/**
  * @brief  Returns the FLASH User Option Bytes values.
  * @param  None
  * @retval The FLASH User Option Bytes values: IWDG_SW(Bit0), RST_STOP(Bit1)
  *         and RST_STDBY(Bit2).
  */
static inline uint8_t flash_ob_get_user(void)
{
  /* Return the User Option Byte */
  return (uint8_t)(FLASH->OPTCR >> 5);
}

/**
  * @brief  Returns the FLASH Write Protection Option Bytes value.
  * @param  None
  * @retval The FLASH Write Protection  Option Bytes value
  */
static inline uint16_t flash_ob_get_wrp(void)
{
  /* Return the FLASH write protection Register value */
  return (*(__IO uint16_t *)(OPTCR_BYTE2_ADDRESS));
}

/**
  * @brief  Returns the FLASH Read Protection level.
  * @param  None
  * @retval FLASH ReadOut Protection Status:
  *           - SET, when OB_RDP_Level_1 or OB_RDP_Level_2 is set
  *           - RESET, when OB_RDP_Level_0 is set
  */
static inline bool flash_ob_get_rdp(void)
{
  return ((*(__IO uint8_t*)(OPTCR_BYTE1_ADDRESS) != (uint8_t)OB_RDP_Level_0));
}

/**
  * @brief  Returns the FLASH BOR level.
  * @param  None
  * @retval The FLASH BOR level:
  *           - OB_BOR_LEVEL3: Supply voltage ranges from 2.7 to 3.6 V
  *           - OB_BOR_LEVEL2: Supply voltage ranges from 2.4 to 2.7 V
  *           - OB_BOR_LEVEL1: Supply voltage ranges from 2.1 to 2.4 V
  *           - OB_BOR_OFF   : Supply voltage ranges from 1.62 to 2.1 V  
  */
static inline uint8_t flash_ob_get_bor(void)
{
  /* Return the FLASH BOR level */
  return (uint8_t)(*(__IO uint8_t *)(OPTCR_BYTE0_ADDRESS) & (uint8_t)0x0C);
}

/**
  * @brief  Enables or disables the specified FLASH interrupts.
  * @param  FLASH_IT: specifies the FLASH interrupt sources to be enabled or disabled.
  *          This parameter can be any combination of the following values:
  *            @arg FLASH_IT_ERR: FLASH Error Interrupt
  *            @arg FLASH_IT_EOP: FLASH end of operation Interrupt
  * @retval None 
  */
static inline void flash_it_config(uint32_t FLASH_IT, bool en )
{

  if( en )
  {
    /* Enable the interrupt sources */
    FLASH->CR |= FLASH_IT;
  }
  else
  {
    /* Disable the interrupt sources */
    FLASH->CR &= ~(uint32_t)FLASH_IT;
  }
}

/**
  * @brief  Checks whether the specified FLASH flag is set or not.
  * @param  FLASH_FLAG: specifies the FLASH flag to check.
  *          This parameter can be one of the following values:
  *            @arg FLASH_FLAG_EOP: FLASH End of Operation flag 
  *            @arg FLASH_FLAG_OPERR: FLASH operation Error flag 
  *            @arg FLASH_FLAG_WRPERR: FLASH Write protected error flag 
  *            @arg FLASH_FLAG_PGAERR: FLASH Programming Alignment error flag
  *            @arg FLASH_FLAG_PGPERR: FLASH Programming Parallelism error flag
  *            @arg FLASH_FLAG_PGSERR: FLASH Programming Sequence error flag
  *            @arg FLASH_FLAG_BSY: FLASH Busy flag
  * @retval The new state of FLASH_FLAG (SET or RESET).
  */
static inline bool flash_get_flag_status(uint32_t FLASH_FLAG)
{

  return((FLASH->SR & FLASH_FLAG) != (uint32_t)0);
}

/**
  * @brief  Clears the FLASH's pending flags.
  * @param  FLASH_FLAG: specifies the FLASH flags to clear.
  *          This parameter can be any combination of the following values:
  *            @arg FLASH_FLAG_EOP: FLASH End of Operation flag 
  *            @arg FLASH_FLAG_OPERR: FLASH operation Error flag 
  *            @arg FLASH_FLAG_WRPERR: FLASH Write protected error flag 
  *            @arg FLASH_FLAG_PGAERR: FLASH Programming Alignment error flag 
  *            @arg FLASH_FLAG_PGPERR: FLASH Programming Parallelism error flag
  *            @arg FLASH_FLAG_PGSERR: FLASH Programming Sequence error flag
  * @retval None
  */
static inline void flash_clear_flag(uint32_t FLASH_FLAG)
{
  /* Clear the flags */
  FLASH->SR = FLASH_FLAG;
}

#ifdef __cplusplus
}
#endif

//Undef macros
#undef SECTOR_MASK
