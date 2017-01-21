#pragma once

#include <stm32lib.h>


#ifdef __cplusplus
 namespace stm32 {
#endif


/**
  * @brief  Sets the code latency value.
  * @param  FLASH_Latency: specifies the FLASH Latency value.
  *          This parameter can be one of the following values:
  *            @arg FLASH_Latency_0: FLASH Zero Latency cycle
  *            @arg FLASH_Latency_1: FLASH One Latency cycle
  *            @arg FLASH_Latency_2: FLASH Two Latency cycles 
  * @retval None
  */
static inline void flash_set_latency(uint32_t FLASH_Latency)
{
   uint32_t tmpreg = 0;

  /* Read the ACR register */
  tmpreg = FLASH->ACR;

  /* Sets the Latency value */
  tmpreg &= (uint32_t) (~((uint32_t)FLASH_ACR_LATENCY));
  tmpreg |= FLASH_Latency;

  /* Write the ACR register */
  FLASH->ACR = tmpreg;
}


/**
  * @brief  Enables or disables the Half cycle flash access.
  * @param  FLASH_HalfCycleAccess: specifies the FLASH Half cycle Access mode.
  *          This parameter can be one of the following values:
  *            @arg FLASH_HalfCycleAccess_Enable: FLASH Half Cycle Enable
  *            @arg FLASH_HalfCycleAccess_Disable: FLASH Half Cycle Disable
  * @retval None
  */
static inline void flash_half_cycle_access_cmd( bool en )
{
  /* Check the parameters */

  if(en)
  {
    FLASH->ACR |= FLASH_ACR_HLFCYA;
  }
  else
  {
    FLASH->ACR &= (uint32_t)(~((uint32_t)FLASH_ACR_HLFCYA));
  }
}


/**
  * @brief  Enables or disables the Prefetch Buffer.
  * @param  NewState: new state of the Prefetch Buffer.
  *          This parameter  can be: ENABLE or DISABLE.
  * @retval None
  */
static inline void flash_prefetch_buffer_cmd( bool en )
{
  /* Check the parameters */
  if( en )
  {
    FLASH->ACR |= FLASH_ACR_PRFTBE;
  }
  else
  {
    FLASH->ACR &= (uint32_t)(~((uint32_t)FLASH_ACR_PRFTBE));
  }
}


/**
  * @brief  Unlocks the FLASH control register and program memory access.
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


/**
  * @brief  Returns the FLASH Status.
  * @param  None
  * @retval FLASH Status: The returned value can be: 
  *         FLASH_BUSY, FLASH_ERROR_PROGRAM, FLASH_ERROR_WRP or FLASH_COMPLETE.
  */
static inline FLASH_Status flash_get_status(void)
{
  FLASH_Status FLASHstatus = FLASH_COMPLETE;
  
  if((FLASH->SR & FLASH_FLAG_BSY) == FLASH_FLAG_BSY) 
  {
    FLASHstatus = FLASH_BUSY;
  }
  else 
  {  
    if((FLASH->SR & (uint32_t)FLASH_FLAG_WRPERR)!= (uint32_t)0x00)
    { 
      FLASHstatus = FLASH_ERROR_WRP;
    }
    else 
    {
      if((FLASH->SR & (uint32_t)(FLASH_SR_PGERR)) != (uint32_t)0x00)
      {
        FLASHstatus = FLASH_ERROR_PROGRAM; 
      }
      else
      {
        FLASHstatus = FLASH_COMPLETE;
      }
    }
  }
  /* Return the FLASH Status */
  return FLASHstatus;
}

/**
  * @brief  Waits for a FLASH operation to complete or a TIMEOUT to occur.
  * @param  Timeout: FLASH programming Timeout
  * @retval FLASH Status: The returned value can be: FLASH_BUSY, 
  *         FLASH_ERROR_PROGRAM, FLASH_ERROR_WRP, FLASH_COMPLETE or FLASH_TIMEOUT.
  */
static inline FLASH_Status flash_wait_for_last_operation(uint32_t Timeout)
{ 
  FLASH_Status status = FLASH_COMPLETE;
   
  /* Check for the FLASH Status */
  status = flash_get_status();
  
  /* Wait for a FLASH operation to complete or a TIMEOUT to occur */
  while((status == FLASH_BUSY) && (Timeout != 0x00))
  {
    status = flash_get_status();
    Timeout--;
  }
  
  if(Timeout == 0x00 )
  {
    status = FLASH_TIMEOUT;
  }
  /* Return the operation status */
  return status;
}


/**
  * @brief  Erases a specified page in program memory.
  * @note   To correctly run this function, the FLASH_Unlock() function must be called before.
  * @note   Call the FLASH_Lock() to disable the flash memory access 
  *         (recommended to protect the FLASH memory against possible unwanted operation)  
  * @param  Page_Address: The page address in program memory to be erased.
  * @note   A Page is erased in the Program memory only if the address to load 
  *         is the start address of a page (multiple of 1024 bytes).
  * @retval FLASH Status: The returned value can be: 
  *         FLASH_ERROR_PROGRAM, FLASH_ERROR_WRP, FLASH_COMPLETE or FLASH_TIMEOUT.
  */
static inline FLASH_Status flash_erase_page(uint32_t Page_Address)
{
  FLASH_Status status = FLASH_COMPLETE;

 
  /* Wait for last operation to be completed */
  status = flash_wait_for_last_operation(FLASH_ER_PRG_TIMEOUT);
  
  if(status == FLASH_COMPLETE)
  { 
    /* If the previous operation is completed, proceed to erase the page */
    FLASH->CR |= FLASH_CR_PER;
    FLASH->AR  = Page_Address; 
    FLASH->CR |= FLASH_CR_STRT;
    
    /* Wait for last operation to be completed */
    status = flash_wait_for_last_operation(FLASH_ER_PRG_TIMEOUT);
    
    /* Disable the PER Bit */
    FLASH->CR &= ~FLASH_CR_PER;
  }
    
  /* Return the Erase Status */
  return status;
}


/**
  * @brief  Erases all FLASH pages.
  * @note   To correctly run this function, the FLASH_Unlock() function
  *         must be called before.
  * @note   Call the FLASH_Lock() to disable the flash memory access 
  *         (recommended to protect the FLASH memory against possible unwanted operation)
  * @param  None              
  * @retval FLASH Status: The returned value can be: FLASH_ERROR_PG,
  *         FLASH_ERROR_WRP, FLASH_COMPLETE or FLASH_TIMEOUT.
  */
static inline FLASH_Status flash_erase_all_pages(void)
{
  FLASH_Status status = FLASH_COMPLETE;

  /* Wait for last operation to be completed */
  status = flash_wait_for_last_operation(FLASH_ER_PRG_TIMEOUT);
  
  if(status == FLASH_COMPLETE)
  {
    /* if the previous operation is completed, proceed to erase all pages */
     FLASH->CR |= FLASH_CR_MER;
     FLASH->CR |= FLASH_CR_STRT;
    
    /* Wait for last operation to be completed */
    status = flash_wait_for_last_operation(FLASH_ER_PRG_TIMEOUT);

    /* Disable the MER Bit */
    FLASH->CR &= ~FLASH_CR_MER;
  }

  /* Return the Erase Status */
  return status;
}


/**
  * @brief  Programs a word at a specified address.
  * @note   To correctly run this function, the FLASH_Unlock() function must be called before.
  * @note   Call the FLASH_Lock() to disable the flash memory access 
  *          (recommended to protect the FLASH memory against possible unwanted operation)   
  * @param  Address: specifies the address to be programmed.
  * @param  Data: specifies the data to be programmed.
  * @retval FLASH Status: The returned value can be: FLASH_ERROR_PG,
  *         FLASH_ERROR_WRP, FLASH_COMPLETE or FLASH_TIMEOUT. 
  */
static inline FLASH_Status flash_program_word(uint32_t Address, uint32_t Data)
{
  FLASH_Status status = FLASH_COMPLETE;
  __IO uint32_t tmp = 0;

  /* Wait for last operation to be completed */
  status = flash_wait_for_last_operation(FLASH_ER_PRG_TIMEOUT);
  
  if(status == FLASH_COMPLETE)
  {
    /* If the previous operation is completed, proceed to program the new first 
    half word */
    FLASH->CR |= FLASH_CR_PG;
  
    *(__IO uint16_t*)Address = (uint16_t)Data;
    
    /* Wait for last operation to be completed */
    status = flash_wait_for_last_operation(FLASH_ER_PRG_TIMEOUT);
 
    if(status == FLASH_COMPLETE)
    {
      /* If the previous operation is completed, proceed to program the new second 
      half word */
      tmp = Address + 2;

      *(__IO uint16_t*) tmp = Data >> 16;
    
      /* Wait for last operation to be completed */
      status = flash_wait_for_last_operation(FLASH_ER_PRG_TIMEOUT);
        
      /* Disable the PG Bit */
      FLASH->CR &= ~FLASH_CR_PG;
    }
    else
    {
      /* Disable the PG Bit */
      FLASH->CR &= ~FLASH_CR_PG;
    }
  }
   
  /* Return the Program Status */
  return status;
}



/**
  * @brief  Programs a half word at a specified address.
  * @note   To correctly run this function, the FLASH_Unlock() function must be called before.
  * @note   Call the FLASH_Lock() to disable the flash memory access 
  *         (recommended to protect the FLASH memory against possible unwanted operation)   
  * @param  Address: specifies the address to be programmed.
  * @param  Data: specifies the data to be programmed.
  * @retval FLASH Status: The returned value can be: FLASH_ERROR_PG,
  *         FLASH_ERROR_WRP, FLASH_COMPLETE or FLASH_TIMEOUT. 
  */
static inline FLASH_Status flash_program_half_word(uint32_t Address, uint16_t Data)
{
  FLASH_Status status = FLASH_COMPLETE;

  /* Wait for last operation to be completed */
  status = flash_wait_for_last_operation(FLASH_ER_PRG_TIMEOUT);
  
  if(status == FLASH_COMPLETE)
  {
    /* If the previous operation is completed, proceed to program the new data */
    FLASH->CR |= FLASH_CR_PG;
  
    *(__IO uint16_t*)Address = Data;

    /* Wait for last operation to be completed */
    status = flash_wait_for_last_operation(FLASH_ER_PRG_TIMEOUT);
    
    /* Disable the PG Bit */
    FLASH->CR &= ~FLASH_CR_PG;
  } 
  
  /* Return the Program Status */
  return status;
}


/**
  * @brief  Unlocks the option bytes block access.
  * @param  None
  * @retval None
  */
static inline void flash_ob_unlock(void)
{
  if((FLASH->CR & FLASH_CR_OPTWRE) == RESET)
  { 
    /* Unlocking the option bytes block access */
    FLASH->OPTKEYR = FLASH_OPTKEY1;
    FLASH->OPTKEYR = FLASH_OPTKEY2;
  }
}

/**
  * @brief  Locks the option bytes block access.
  * @param  None
  * @retval None
  */
static inline void flash_ob_lock(void)
{
  /* Set the OPTWREN Bit to lock the option bytes block access */
  FLASH->CR &= ~FLASH_CR_OPTWRE;
}


/**
  * @brief  Launch the option byte loading.
  * @param  None
  * @retval None
  */
static inline int flash_ob_launch(void)
{
  /* Set the OBL_Launch bit to launch the option byte loading */
  FLASH->CR |= FLASH_CR_OBL_LAUNCH;
  return FLASH_COMPLETE;
}


/**
  * @brief  Checks whether the FLASH Read out Protection Status is set or not.
  * @param  None
  * @retval FLASH ReadOut Protection Status(SET or RESET)
  */
static inline bool flash_ob_get_rdp(void)
{
  return ((uint8_t)(FLASH->OBR & (FLASH_OBR_RDPRT1 | FLASH_OBR_RDPRT2)) != 0 );
}

/**
  * @brief  Erases the FLASH option bytes.
  * @note   To correctly run this function, the FLASH_OB_Unlock() function must be called before.
  * @note   Call the FLASH_OB_Lock() to disable the flash control register access and the option bytes 
  *         (recommended to protect the FLASH memory against possible unwanted operation)   
  * @note   This functions erases all option bytes except the Read protection (RDP).
  * @param  None
  * @retval FLASH Status: The returned value can be: FLASH_ERROR_PG,
  *         FLASH_ERROR_WRP, FLASH_COMPLETE or FLASH_TIMEOUT.
  */
static inline FLASH_Status flash_ob_erase(void)
{
  uint16_t rdptmp = OB_RDP_Level_0;

  FLASH_Status status = FLASH_COMPLETE;

  /* Get the actual read protection Option Byte value */ 
  if(flash_ob_get_rdp() != RESET)
  {
    rdptmp = 0x00;  
  }

  /* Wait for last operation to be completed */
  status = flash_wait_for_last_operation(FLASH_ER_PRG_TIMEOUT);

  if(status == FLASH_COMPLETE)
  {   
    /* If the previous operation is completed, proceed to erase the option bytes */
    FLASH->CR |= FLASH_CR_OPTER;
    FLASH->CR |= FLASH_CR_STRT;

    /* Wait for last operation to be completed */
    status = flash_wait_for_last_operation(FLASH_ER_PRG_TIMEOUT);
    
    if(status == FLASH_COMPLETE)
    {
      /* If the erase operation is completed, disable the OPTER Bit */
      FLASH->CR &= ~FLASH_CR_OPTER;
       
      /* Enable the Option Bytes Programming operation */
      FLASH->CR |= FLASH_CR_OPTPG;

      /* Restore the last read protection Option Byte value */
      OB->RDP = (uint16_t)rdptmp; 

      /* Wait for last operation to be completed */
      status = flash_wait_for_last_operation(FLASH_ER_PRG_TIMEOUT);
 
      if(status != FLASH_TIMEOUT)
      {
        /* if the program operation is completed, disable the OPTPG Bit */
        FLASH->CR &= ~FLASH_CR_OPTPG;
      }
    }
    else
    {
      if (status != FLASH_TIMEOUT)
      {
        /* Disable the OPTPG Bit */
        FLASH->CR &= ~FLASH_CR_OPTPG;
      }
    }  
  }
  /* Return the erase status */
  return status;
}



/**
  * @brief  Programs a half word at a specified Option Byte Data address.
  * @note   To correctly run this function, the FLASH_OB_Unlock() function must be called before.
  * @note   Call the FLASH_OB_Lock() to disable the flash control register access and the option bytes 
  *         (recommended to protect the FLASH memory against possible unwanted operation)
  * @param  Address: specifies the address to be programmed.
  *         This parameter can be 0x1FFFF804 or 0x1FFFF806. 
  * @param  Data: specifies the data to be programmed.
  * @retval FLASH Status: The returned value can be: FLASH_ERROR_PG,
  *         FLASH_ERROR_WRP, FLASH_COMPLETE or FLASH_TIMEOUT.
  */
static inline FLASH_Status flash_ob_program_data(uint32_t Address, uint8_t Data)
{
  FLASH_Status status = FLASH_COMPLETE;
  /* Check the parameters */
  status = flash_wait_for_last_operation(FLASH_ER_PRG_TIMEOUT);

  if(status == FLASH_COMPLETE)
  {
    /* Enables the Option Bytes Programming operation */
    FLASH->CR |= FLASH_CR_OPTPG; 
    *(__IO uint16_t*)Address = Data;
    
    /* Wait for last operation to be completed */
    status = flash_wait_for_last_operation(FLASH_ER_PRG_TIMEOUT);
    
    if(status != FLASH_TIMEOUT)
    {
      /* If the program operation is completed, disable the OPTPG Bit */
      FLASH->CR &= ~FLASH_CR_OPTPG;
    }
  }
  /* Return the Option Byte Data Program Status */
  return status;
}


/**
  * @brief  Write protects the desired pages
  * @note   To correctly run this function, the FLASH_OB_Unlock() function must be called before.
  * @note   Call the FLASH_OB_Lock() to disable the flash control register access and the option bytes 
  *         (recommended to protect the FLASH memory against possible unwanted operation)    
  * @param  OB_WRP: specifies the address of the pages to be write protected.
  *          This parameter can be:
  *            @arg OB_WRP_Pages0to1..OB_WRP_Pages62to127
  *            @arg OB_WRP_AllPages
  * @retval FLASH Status: The returned value can be: 
  *         FLASH_ERROR_PROGRAM, FLASH_ERROR_WRP, FLASH_COMPLETE or FLASH_TIMEOUT.
  */
static inline FLASH_Status flash_ob_enable_wrp(uint32_t OB_WRP)
{
  uint16_t WRP0_Data = 0xFFFF, WRP1_Data = 0xFFFF, WRP2_Data = 0xFFFF, WRP3_Data = 0xFFFF;
  
  FLASH_Status status = FLASH_COMPLETE;
  
    
  OB_WRP = (uint32_t)(~OB_WRP);
  WRP0_Data = (uint16_t)(OB_WRP & OB_WRP0_WRP0);
  WRP1_Data = (uint16_t)((OB_WRP & OB_WRP0_nWRP0) >> 8); 
  WRP2_Data = (uint16_t)((OB_WRP & OB_WRP1_WRP1) >> 16);
  WRP3_Data = (uint16_t)((OB_WRP & OB_WRP1_nWRP1) >> 24); 
  
  /* Wait for last operation to be completed */
  status = flash_wait_for_last_operation(FLASH_ER_PRG_TIMEOUT);
  
  if(status == FLASH_COMPLETE)
  {
    FLASH->CR |= FLASH_CR_OPTPG;

    if(WRP0_Data != 0xFF)
    {
      OB->WRP0 = WRP0_Data;
      
      /* Wait for last operation to be completed */
      status = flash_wait_for_last_operation(FLASH_ER_PRG_TIMEOUT);
    }
    if((status == FLASH_COMPLETE) && (WRP1_Data != 0xFF))
    {
      OB->WRP1 = WRP1_Data;
      
      /* Wait for last operation to be completed */
      status = flash_wait_for_last_operation(FLASH_ER_PRG_TIMEOUT);
    }
	if((status == FLASH_COMPLETE) && (WRP2_Data != 0xFF))
    {
      OB->WRP2 = WRP2_Data;
      
      /* Wait for last operation to be completed */
      status = flash_wait_for_last_operation(FLASH_ER_PRG_TIMEOUT);
    }
	if((status == FLASH_COMPLETE) && (WRP3_Data != 0xFF))
    {
      OB->WRP3 = WRP3_Data;
      
      /* Wait for last operation to be completed */
      status = flash_wait_for_last_operation(FLASH_ER_PRG_TIMEOUT);
    }
          
    if(status != FLASH_TIMEOUT)
    {
      /* if the program operation is completed, disable the OPTPG Bit */
      FLASH->CR &= ~FLASH_CR_OPTPG;
    }
  } 
  /* Return the write protection operation Status */
  return status;      
}


/**
  * @brief  Enables or disables the read out protection.
  * @note   To correctly run this function, the FLASH_OB_Unlock() function must be called before.
  * @note   Call the FLASH_OB_Lock() to disable the flash control register access and the option bytes 
  *         (recommended to protect the FLASH memory against possible unwanted operation)   
  * @param  FLASH_ReadProtection_Level: specifies the read protection level. 
  *          This parameter can be:
  *             @arg OB_RDP_Level_0: No protection
  *             @arg OB_RDP_Level_1: Read protection of the memory
  *             @arg OB_RDP_Level_2: Chip protection
  * @note   When enabling OB_RDP level 2 it's no more possible to go back to level 1 or 0
  * @retval FLASH Status: The returned value can be: 
  *         FLASH_ERROR_PROGRAM, FLASH_ERROR_WRP, FLASH_COMPLETE or FLASH_TIMEOUT.
  */
static inline FLASH_Status flash_ob_rdp_config(uint8_t OB_RDP)
{
  FLASH_Status status = FLASH_COMPLETE;
  
  status = flash_wait_for_last_operation(FLASH_ER_PRG_TIMEOUT);
  
  if(status == FLASH_COMPLETE)
  {
    FLASH->CR |= FLASH_CR_OPTER;
    FLASH->CR |= FLASH_CR_STRT;
    
    /* Wait for last operation to be completed */
    status = flash_wait_for_last_operation(FLASH_ER_PRG_TIMEOUT);
    
    if(status == FLASH_COMPLETE)
    {
      /* If the erase operation is completed, disable the OPTER Bit */
      FLASH->CR &= ~FLASH_CR_OPTER;
      
      /* Enable the Option Bytes Programming operation */
      FLASH->CR |= FLASH_CR_OPTPG;
       
      OB->RDP = OB_RDP;

      /* Wait for last operation to be completed */
      status = flash_wait_for_last_operation(FLASH_ER_PRG_TIMEOUT); 
    
      if(status != FLASH_TIMEOUT)
      {
        /* if the program operation is completed, disable the OPTPG Bit */
        FLASH->CR &= ~FLASH_CR_OPTPG;
      }
    }
    else 
    {
      if(status != FLASH_TIMEOUT)
      {
        /* Disable the OPTER Bit */
        FLASH->CR &= ~FLASH_CR_OPTER;
      }
    }
  }
  /* Return the protection operation Status */
  return status;
}


/**
  * @brief  Programs the FLASH User Option Byte: IWDG_SW / RST_STOP / RST_STDBY.
  * @note   To correctly run this function, the FLASH_OB_Unlock() function must be called before.
  * @note   Call the FLASH_OB_Lock() to disable the flash control register access and the option
  *         bytes (recommended to protect the FLASH memory against possible unwanted operation)
  * @param  OB_IWDG: Selects the WDG mode
  *          This parameter can be one of the following values:
  *             @arg OB_IWDG_SW: Software WDG selected
  *             @arg OB_IWDG_HW: Hardware WDG selected
  * @param  OB_STOP: Reset event when entering STOP mode.
  *          This parameter can be one of the following values:
  *             @arg OB_STOP_NoRST: No reset generated when entering in STOP
  *             @arg OB_STOP_RST: Reset generated when entering in STOP
  * @param  OB_STDBY: Reset event when entering Standby mode.
  *          This parameter can be one of the following values:
  *             @arg OB_STDBY_NoRST: No reset generated when entering in STANDBY
  *             @arg OB_STDBY_RST: Reset generated when entering in STANDBY
  * @retval FLASH Status: The returned value can be: 
  *         FLASH_ERROR_PROGRAM, FLASH_ERROR_WRP, FLASH_COMPLETE or FLASH_TIMEOUT.
  */
static inline FLASH_Status flash_ob_userconfig(uint8_t OB_IWDG, uint8_t OB_STOP, uint8_t OB_STDBY)
{
  FLASH_Status status = FLASH_COMPLETE; 

  
  /* Wait for last operation to be completed */
  status = flash_wait_for_last_operation(FLASH_ER_PRG_TIMEOUT);
  
  if(status == FLASH_COMPLETE)
  {  
    /* Enable the Option Bytes Programming operation */
    FLASH->CR |= FLASH_CR_OPTPG; 

    OB->USER = (uint16_t)((uint16_t)(OB_IWDG | OB_STOP) | (uint16_t)(OB_STDBY | 0xF8));
  
    /* Wait for last operation to be completed */
    status = flash_wait_for_last_operation(FLASH_ER_PRG_TIMEOUT);

    if(status != FLASH_TIMEOUT)
    {
      /* If the program operation is completed, disable the OPTPG Bit */
      FLASH->CR &= ~FLASH_CR_OPTPG;
    }
  }    
  /* Return the Option Byte program Status */
  return status;
}


/**
  * @brief  Sets or resets the BOOT1 option bit.
  * @param  OB_BOOT1: Set or Reset the BOOT1 option bit.
  *          This parameter can be one of the following values:
  *             @arg OB_BOOT1_RESET: BOOT1 option bit reset
  *             @arg OB_BOOT1_SET: BOOT1 option bit set
  * @retval None
  */
static inline FLASH_Status flash_ob_boot_config(uint8_t OB_BOOT1)
{
  FLASH_Status status = FLASH_COMPLETE; 

  
  /* Wait for last operation to be completed */
  status = flash_wait_for_last_operation(FLASH_ER_PRG_TIMEOUT);
  
  if(status == FLASH_COMPLETE)
  {  
    /* Enable the Option Bytes Programming operation */
    FLASH->CR |= FLASH_CR_OPTPG; 
           
	OB->USER = OB_BOOT1|0xEF;
  
    /* Wait for last operation to be completed */
    status = flash_wait_for_last_operation(FLASH_ER_PRG_TIMEOUT);

    if(status != FLASH_TIMEOUT)
    {
      /* If the program operation is completed, disable the OPTPG Bit */
      FLASH->CR &= ~FLASH_CR_OPTPG;
    }
  }    
  /* Return the Option Byte program Status */
  return status;
}


/**
  * @brief  Sets or resets the analogue monitoring on VDDA Power source. 
  * @param  OB_VDDA_ANALOG: Selects the analog monitoring on VDDA Power source.
  *          This parameter can be one of the following values:
  *             @arg OB_VDDA_ANALOG_ON: Analog monitoring on VDDA Power source ON
  *             @arg OB_VDDA_ANALOG_OFF: Analog monitoring on VDDA Power source OFF
  * @retval None
  */
static inline FLASH_Status flash_ob_vdda_config(uint8_t OB_VDDA_ANALOG)
{
  FLASH_Status status = FLASH_COMPLETE; 

  
  /* Wait for last operation to be completed */
  status = flash_wait_for_last_operation(FLASH_ER_PRG_TIMEOUT);
  
  if(status == FLASH_COMPLETE)
  {  
    /* Enable the Option Bytes Programming operation */
    FLASH->CR |= FLASH_CR_OPTPG; 

    OB->USER = OB_VDDA_ANALOG | 0xDF;
  
    /* Wait for last operation to be completed */
    status = flash_wait_for_last_operation(FLASH_ER_PRG_TIMEOUT);

    if(status != FLASH_TIMEOUT)
    {
      /* if the program operation is completed, disable the OPTPG Bit */
      FLASH->CR &= ~FLASH_CR_OPTPG;
    }
  }    
  /* Return the Option Byte program Status */
  return status;
}


/**
  * @brief  Sets or resets the analogue monitoring on VDD_SD12 Power source. 
  * @param  OB_VDD_SD12: Selects the analog monitoring on VDD_SD12 Power source.
  *          This parameter can be one of the following values:
  *            @arg OB_VDD_SD12_ON: Analog monitoring on VDD_SD12 Power source ON
  *            @arg OB_VDD_SD12_OFF: Analog monitoring on VDD_SD12 Power source OFF
  * @retval None
  */
static inline FLASH_Status flash_ob_vdd_sd12_config(uint8_t OB_VDD_SD12)
{
  FLASH_Status status = FLASH_COMPLETE; 

  
  /* Wait for last operation to be completed */
  status = flash_wait_for_last_operation(FLASH_ER_PRG_TIMEOUT);
  
  if(status == FLASH_COMPLETE)
  {  
    /* Enable the Option Bytes Programming operation */
    FLASH->CR |= FLASH_CR_OPTPG; 
           
    OB->USER = OB_VDD_SD12 | 0x7F;
  
    /* Wait for last operation to be completed */
    status = flash_wait_for_last_operation(FLASH_ER_PRG_TIMEOUT);

    if(status != FLASH_TIMEOUT)
    {
      /* if the program operation is completed, disable the OPTPG Bit */
      FLASH->CR &= ~FLASH_CR_OPTPG;
    }
  }    
  /* Return the Option Byte program Status */
  return status;
}

/**
  * @brief  Sets or resets the SRAM partiy.
  * @param  OB_SRAM_Parity: SSet or Reset the SRAM partiy enable bit.
  *          This parameter can be one of the following values:
  *             @arg OB_SRAM_PARITY_SET: Set SRAM partiy.
  *             @arg OB_SRAM_PARITY_RESET: Reset SRAM partiy.
  * @retval None
  */
static inline FLASH_Status flash_ob_sram_parity_config(uint8_t OB_SRAM_Parity)
{
  FLASH_Status status = FLASH_COMPLETE; 


  /* Wait for last operation to be completed */
  status = flash_wait_for_last_operation(FLASH_ER_PRG_TIMEOUT);
  
  if(status == FLASH_COMPLETE)
  {  
    /* Enable the Option Bytes Programming operation */
    FLASH->CR |= FLASH_CR_OPTPG; 

    OB->USER = OB_SRAM_Parity | 0xBF;
  
    /* Wait for last operation to be completed */
    status = flash_wait_for_last_operation(FLASH_ER_PRG_TIMEOUT);

    if(status != FLASH_TIMEOUT)
    {
      /* if the program operation is completed, disable the OPTPG Bit */
      FLASH->CR &= ~FLASH_CR_OPTPG;
    }
  }
  /* Return the Option Byte program Status */
  return status;
}


/**
  * @brief  Programs the FLASH User Option Byte: IWDG_SW / RST_STOP / RST_STDBY/ BOOT1 / OB_VDDA_ANALOG and
  *         OB_VDD_SD12.
  * @note   To correctly run this function, the FLASH_OB_Unlock() function
  *           must be called before.
  * @note   Call the FLASH_OB_Lock() to disable the flash control register access and the option bytes 
  *          (recommended to protect the FLASH memory against possible unwanted operation)   
  * @param  OB_USER: Selects all user option bytes
  *          This parameter is a combination of the following values:
  *            @arg OB_IWDG_SW: Software WDG selected
  *            @arg OB_IWDG_HW: Hardware WDG selected
  *            @arg OB_STOP_NoRST: No reset generated when entering in STOP
  *            @arg OB_STOP_RST: Reset generated when entering in STOP
  *            @arg OB_STDBY_NoRST: No reset generated when entering in STANDBY
  *            @arg OB_STDBY_RST: Reset generated when entering in STANDBY
  *            @arg OB_BOOT1_RESET: BOOT1 Reset
  *            @arg OB_BOOT1_SET: BOOT1 Set
  *            @arg OB_VDDA_ANALOG_ON: Analog monitoring on VDDA Power source ON
  *            @arg OB_VDDA_ANALOG_OFF: Analog monitoring on VDDA Power source OFF
  *            @arg OB_VDD_SD12_ON: Analog monitoring on VDD_SD12 Power source ON
  *            @arg OB_VDD_SD12_OFF: Analog monitoring on VDD_SD12 Power source OFF 
  * @retval FLASH Status: The returned value can be: 
  * FLASH_ERROR_PROGRAM, FLASH_ERROR_WRP, FLASH_COMPLETE or FLASH_TIMEOUT.
  */
static inline FLASH_Status flash_ob_write_user(uint8_t OB_USER)
{
  FLASH_Status status = FLASH_COMPLETE; 
  
  /* Wait for last operation to be completed */
  status = flash_wait_for_last_operation(FLASH_ER_PRG_TIMEOUT);
  
  if(status == FLASH_COMPLETE)
  {  
    /* Enable the Option Bytes Programming operation */
    FLASH->CR |= FLASH_CR_OPTPG; 
           
    OB->USER = OB_USER | 0x48;
  
    /* Wait for last operation to be completed */
    status = flash_wait_for_last_operation(FLASH_ER_PRG_TIMEOUT);

    if(status != FLASH_TIMEOUT)
    {
      /* if the program operation is completed, disable the OPTPG Bit */
      FLASH->CR &= ~FLASH_CR_OPTPG;
    }
  }    
  /* Return the Option Byte program Status */
  return status;
}


/**
  * @brief  Returns the FLASH User Option Bytes values.
  * @param  None
  * @retval The FLASH User Option Bytes .
  */
static inline uint8_t flash_ob_get_user(void)
{
  /* Return the User Option Byte */
  return (uint8_t)(FLASH->OBR >> 8);
}

/**
  * @brief  Returns the FLASH Write Protection Option Bytes value.
  * @param  None
  * @retval The FLASH Write Protection Option Bytes value
  */
static inline uint32_t flash_ob_get_wrp(void)
{
  /* Return the FLASH write protection Register value */
  return (uint32_t)(FLASH->WRPR);
}



/**
  * @brief  Enables or disables the specified FLASH interrupts.
  * @param  FLASH_IT: specifies the FLASH interrupt sources to be enabled or 
  *         disabled.
  *          This parameter can be any combination of the following values:
  *             @arg FLASH_IT_EOP: FLASH end of programming Interrupt
  *             @arg FLASH_IT_ERR: FLASH Error Interrupt
  * @retval None 
  */
static inline void flash_it_config(uint32_t FLASH_IT, bool en)
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
  *             @arg FLASH_FLAG_BSY: FLASH write/erase operations in progress flag 
  *             @arg FLASH_FLAG_PGERR: FLASH Programming error flag flag
  *             @arg FLASH_FLAG_WRPERR: FLASH Write protected error flag
  *             @arg FLASH_FLAG_EOP: FLASH End of Programming flag
  * @retval The new state of FLASH_FLAG (SET or RESET).
  */
static inline bool flash_get_flag_status(uint32_t FLASH_FLAG)
{
  return ((FLASH->SR & FLASH_FLAG) != (uint32_t)0);
}


/**
  * @brief  Clears the FLASH's pending flags.
  * @param  FLASH_FLAG: specifies the FLASH flags to clear.
  *          This parameter can be any combination of the following values:
  *             @arg FLASH_FLAG_PGERR: FLASH Programming error flag flag
  *             @arg FLASH_FLAG_WRPERR: FLASH Write protected error flag
  *             @arg FLASH_FLAG_EOP: FLASH End of Programming flag
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
