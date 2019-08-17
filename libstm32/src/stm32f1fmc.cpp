 
/*
 * stm32fmc.cpp
 *
 *  Created on: 04-01-2011
 *      Author: lucck
 */
 
#include "stm32fmc.hpp"
#include <stdint.h>
#include <stm32system.h>
 

namespace stm32
{

 
namespace //private registers
{
/* Flash Access Control Register bits */
	const uint32_t ACR_LATENCY_Mask  =       0x00000038;
	const uint32_t ACR_HLFCYA_Mask   =       0xFFFFFFF7;
	const uint32_t ACR_PRFTBE_Mask   =       0xFFFFFFEF;

/* Flash Access Control Register bits */
	const uint32_t ACR_PRFTBS_Mask   =       0x00000020;

/* Flash Control Register bits */
	const uint32_t CR_PG_Set         =       0x00000001;
	const uint32_t CR_PG_Reset       =       0x00001FFE;
	const uint32_t CR_PER_Set        =       0x00000002;
	const uint32_t CR_PER_Reset      =       0x00001FFD;
	const uint32_t CR_MER_Set        =       0x00000004;
	const uint32_t CR_MER_Reset      =       0x00001FFB;
	const uint32_t CR_OPTPG_Set      =       0x00000010;
	const uint32_t CR_OPTPG_Reset    =       0x00001FEF;
	const uint32_t CR_OPTER_Set      =       0x00000020;
	const uint32_t CR_OPTER_Reset    =       0x00001FDF;
	const uint32_t CR_STRT_Set       =       0x00000040;
	const uint32_t CR_LOCK_Set       =       0x00000080;

/* FLASH Mask */
	const uint32_t RDPRT_Mask        =       0x00000002;
	const uint32_t WRP0_Mask         =       0x000000FF;
	const uint32_t WRP1_Mask         =       0x0000FF00;
	const uint32_t WRP2_Mask         =       0x00FF0000;
	const uint32_t WRP3_Mask         =       0xFF000000;

/* FLASH Keys */
	const uint32_t RDP_Key           =       0x00A5;
	const uint32_t FLASH_KEY1        =       0x45670123;
	const uint32_t FLASH_KEY2        =       0xCDEF89AB;

/* Delay definition */
	const uint32_t EraseTimeout      =       0x00000FFF;
	const uint32_t ProgramTimeout    =       0x0000000F;
	volatile uint16_t * const Memory_Size_Register = reinterpret_cast<volatile uint16_t *>(0x1FFFF7E0);
	const uint32_t FlashBaseAddr = 0x08000000;
	const uint16_t FLASH_PAGE_SIZE_1  = 0x400;
	const uint16_t FLASH_PAGE_SIZE_2  =  0x800;
}

 
namespace
{
   void delay()
   {
	   for(unsigned i=0; i<0xFF; i++) nop();
   }
}

 
//! Get pagesize
stm32fmc::poffs_t stm32fmc::page_size() const
{
#if defined(STM32F10X_LD) || defined(STM32F10X_LD_VL)
	return FLASH_PAGE_SIZE_1;
#elif defined(STM32F10X_MD) || defined(STM32F10X_MD_VL)
	return FLASH_PAGE_SIZE_1;
#elif defined(STM32F10X_HD) || defined(STM32F10X_HD_VL)
	return FLASH_PAGE_SIZE_2;
#elif defined(STM32F10X_CL)
	return FLASH_PAGE_SIZE_2;
#else
#error FLASH Memory size not defined
#endif
}

 
//!Get numer of pages
stm32fmc::paddr_t stm32fmc::num_pages() const
{
	uint32_t flash_mem_size = *Memory_Size_Register;
	return (flash_mem_size * 1024)/page_size();
}
 
//! Default constructor
stm32fmc::stm32fmc()
{
	//Flash Unlock
	/* Authorize the FPEC Access */
	FLASH->KEYR = FLASH_KEY1;
	FLASH->KEYR = FLASH_KEY2;
	//Clear flags
	FLASH->SR = FLASH_FLAG_BSY | FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPRTERR;
}
 
//Virtual destructor
stm32fmc::~stm32fmc()
{
	/* Set the Lock Bit to lock the FPEC and the FCR */
	FLASH->CR |= CR_LOCK_Set;
}

 
//Erase the page
int stm32fmc::page_erase(paddr_t pa)
{
	 errno status = ERRNO_COMPLETE;

	/* Wait for last operation to be completed */
	  status = wait_for_last_operation(EraseTimeout);
	  if(status == ERRNO_COMPLETE)
	  {
		/* if the previous operation is completed, proceed to erase the page */
	    FLASH->CR|= CR_PER_Set;
	    FLASH->AR = static_cast<uint32_t>(pa) * static_cast<uint32_t>(page_size()) + FlashBaseAddr;
	    FLASH->CR|= CR_STRT_Set;
	    /* Wait for last operation to be completed */
	    status = wait_for_last_operation(EraseTimeout);
	    if(status != ERRNO_TIMEOUT)
	    {
	      /* if the erase operation is completed, disable the PER Bit */
	      FLASH->CR &= CR_PER_Reset;
	    }
	  }
	  /* Return the Erase Status */
	  return status;
}
 
//Internal wait for last operation
stm32fmc::errno stm32fmc::wait_for_last_operation(unsigned timeout)
{
	  errno status = ERRNO_COMPLETE;
	  /* Check for the Flash Status */
	  status = get_status();
	  /* Wait for a Flash operation to complete or a TIMEOUT to occur */
	  while((status == ERRNO_BUSY) && (timeout != 0x00))
	  {
	    delay();
	    status = get_status();
	    timeout--;
	  }
	  if(timeout == 0x00 )
	  {
	    status = ERRNO_TIMEOUT;
	  }
	  /* Return the operation status */
	  return status;
}

 
//Get flash status
stm32fmc::errno stm32fmc::get_status() const
{
	  errno flashstatus = ERRNO_COMPLETE;

	  if((FLASH->SR & FLASH_FLAG_BSY) == FLASH_FLAG_BSY)
	  {
	    flashstatus = ERRNO_BUSY;
	  }
	  else
	  {
	    if((FLASH->SR & FLASH_FLAG_PGERR) != 0)
	    {
	      flashstatus = ERRNO_PG;
	    }
	    else
	    {
	      if((FLASH->SR & FLASH_FLAG_WRPRTERR) != 0 )
	      {
	        flashstatus = ERRNO_WRP;
	      }
	      else
	      {
	        flashstatus = ERRNO_COMPLETE;
	      }
	    }
	  }
	  /* Return the Flash Status */
	  return flashstatus;
}
 
//Write elements in the flash
int stm32fmc::write_halfword(paddr_t pg, poffs_t pa, unsigned short val)
{

	  errno status = ERRNO_COMPLETE;

	  /* Wait for last operation to be completed */
	  status = wait_for_last_operation(ProgramTimeout);

	  if(status == ERRNO_COMPLETE)
	  {
		/* if the previous operation is completed, proceed to program the new data */
	    FLASH->CR |= CR_PG_Set;
	    volatile uint16_t * const address = reinterpret_cast<volatile uint16_t*>
	    ( static_cast<uint32_t>(pg) * static_cast<uint32_t>(page_size()) + pa + FlashBaseAddr);
	    *address = val;
	    /* Wait for last operation to be completed */
	    status = wait_for_last_operation(ProgramTimeout);
	    if(status != ERRNO_TIMEOUT)
	    {
	      /* if the program operation is completed, disable the PG Bit */
	      FLASH->CR &= CR_PG_Reset;
	    }
	  }
	  /* Return the Program Status */
	  return status;
}
 
//Read halfword
int stm32fmc::read_halfword(paddr_t pg, poffs_t pa, unsigned short &val) const
{
	volatile uint16_t * const address = reinterpret_cast<volatile uint16_t*>(
			static_cast<uint32_t>(pg) * static_cast<uint32_t>(page_size())
		    + FlashBaseAddr + pa );
	val = *address;
	return ERRNO_COMPLETE;
}
 
//Read one element from flash return bytes written or error
int stm32fmc::read_word(paddr_t pg, poffs_t pa, unsigned &val) const
{
	volatile uint32_t * const address = reinterpret_cast<volatile uint32_t*>(
		static_cast<uint32_t>(pg) * static_cast<uint32_t>(page_size()) + FlashBaseAddr + pa );
	val = *address;
	return ERRNO_COMPLETE;
}

 
//Write one lement to flash return bytes written or error
int stm32fmc::write_word(paddr_t pg, poffs_t pa, unsigned val)
{
	  errno status = ERRNO_COMPLETE;

	  /* Wait for last operation to be completed */
	  status = wait_for_last_operation(ProgramTimeout);

	  if(status == ERRNO_COMPLETE)
	  {
	    /* if the previous operation is completed, proceed to program the new first
	    half word */
	    FLASH->CR |= CR_PG_Set;
	    {
	    volatile uint16_t * const address = reinterpret_cast<volatile uint16_t*>
	 	    ( static_cast<uint32_t>(pg) * static_cast<uint32_t>(page_size()) + pa + FlashBaseAddr);
	    *address = val;
	    }
	    /* Wait for last operation to be completed */
	    status = wait_for_last_operation(ProgramTimeout);

	    if(status == ERRNO_COMPLETE)
	    {
	      /* if the previous operation is completed, proceed to program the new second
	      half word */
	    	{
	    	   volatile uint16_t * const address = reinterpret_cast<volatile uint16_t*>
	    	   ( static_cast<uint32_t>(pg) * static_cast<uint32_t>(page_size()) + pa + FlashBaseAddr + 2);
	    	   *address = val >> 16;
	    	}

	      /* Wait for last operation to be completed */
	      status = wait_for_last_operation(ProgramTimeout);

	      if(status != ERRNO_TIMEOUT)
	      {
	        /* Disable the PG Bit */
	        FLASH->CR &= CR_PG_Reset;
	      }
	    }
	    else
	    {
	      if (status != ERRNO_TIMEOUT)
	      {
	        /* Disable the PG Bit */
	        FLASH->CR &= CR_PG_Reset;
	      }
	     }
	  }

	  /* Return the Program Status */
	  return status;
}
 

}
