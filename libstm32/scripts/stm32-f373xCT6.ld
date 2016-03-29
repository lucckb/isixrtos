/*
Linker script for STM32F374
*/


/* include the common STM32F10x sub-script */

/* Memory Spaces Definitions */
MEMORY
{
  RAM (xrw) : ORIGIN = 0x20000000, LENGTH = 32K  /* also change _estack below */
  AUX_RAM (xrw) : ORIGIN = 0x00000000, LENGTH = 0  /* Second SRAM bank - unused */
  FLASH (rx) : ORIGIN = 0x8000000, LENGTH = 256K
  FLASHB1 (rx) : ORIGIN = 0x00000000, LENGTH = 0
  EXTMEMB0 (rx) : ORIGIN = 0x00000000, LENGTH = 0
  EXTMEMB1 (rx) : ORIGIN = 0x00000000, LENGTH = 0
  EXTMEMB2 (rx) : ORIGIN = 0x00000000, LENGTH = 0
  EXTMEMB3 (rx) : ORIGIN = 0x00000000, LENGTH = 0
}

/* highest address of the user mode stack */
_estack = 0x20008000;


/* include the section management sub-script */
/* (either "STM32_SEC_FLASH.ld" or "STM32_SEC_RAM.ld") */
INCLUDE "stm32_flash.ld"


