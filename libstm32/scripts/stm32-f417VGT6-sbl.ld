/*
Linker script for STM32F10x
Copyright RAISONANCE 2007 (modified by Lanchon 1-Feb-2008)
You can use, copy and distribute this file freely, but without any waranty.
Configure memory sizes, end of stack and boot mode for your project here.
*/


/* include the common STM32F10x sub-script */
INCLUDE "stm32_common.ld"

/* Memory Spaces Definitions */
MEMORY
{
  RAM (xrw) : ORIGIN = 0x20000000, LENGTH = 128K  /* also change _estack below */
  FLASH (rx) : ORIGIN = 0x8004000, LENGTH = (1M - 16k)
  FLASHB1 (rx) : ORIGIN = 0x00000000, LENGTH = 0
  EXTMEMB0 (rx) : ORIGIN = 0x00000000, LENGTH = 0
  EXTMEMB1 (rx) : ORIGIN = 0x00000000, LENGTH = 0
  EXTMEMB2 (rx) : ORIGIN = 0x00000000, LENGTH = 0
  EXTMEMB3 (rx) : ORIGIN = 0x00000000, LENGTH = 0
  CCM_RAM (rw)	: org = 0x10000000, LENGTH = 64k
  BKP_RAM (rwx)	: org = 0x40024000, LENGTH = 4k
}

/* highest address of the user mode stack */
_estack = 0x20020000;


__ccm_ram_start = ORIGIN(CCM_RAM);
__ccm_ram_size = LENGTH(CCM_RAM);
__ccm_ram_end = __ccm_ram_start + __ccm_ram_size;

__bkp_ram_start = ORIGIN(BKP_RAM);
__bkp_ram_size = LENGTH(BKP_RAM);
__bkp_ram_end = __bkp_ram_start + __bkp_ram_size;

PROVIDE(__ccm_ram_start = __ccm_ram_start);
PROVIDE(__ccm_ram_size = __ccm_ram_size);
PROVIDE(__ccm_ram_end = __ccm_ram_end);

PROVIDE(__bkp_ram_start = __bkp_ram_start);
PROVIDE(__bkp_ram_size = __bkp_ram_size);
PROVIDE(__bkp_ram_end = __bkp_ram_end);



/* include the section management sub-script */
/* (either "STM32_SEC_FLASH.ld" or "STM32_SEC_RAM.ld") */
INCLUDE "stm32_flash.ld"

