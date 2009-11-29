/* ------------------------------------------------------------ */
/*
 * upgrade-detect.c
 *
 *  Created on: 2009-10-12
 *      Author: lucck
 */

/* ------------------------------------------------------------ */
#include "stm32f10x_lib.h"
#include "upgrade.h"
/* ------------------------------------------------------------ */

#define BACKUP_MAGIC_KEY 0x1979
#define BACKUP_MAGIC_OFFSET 0x2379
/* ------------------------------------------------------------ */
//! Enable access to the loader registers
void upgrade_init(void)
{
	//Enable access to the backup doman
	RCC->APB1ENR |= RCC_APB1ENR_BKPEN | RCC_APB1ENR_PWREN;
}

/* ------------------------------------------------------------ */
//! Set loader status
void upgrade_setStatus(upgrade_status cmd_type)
{
	//Enable access to the bkp regs
	PWR->CR |= PWR_CR_DBP;
	BKP->DR1 = BACKUP_MAGIC_KEY;
	BKP->DR2 = BACKUP_MAGIC_OFFSET + cmd_type;
	//Disable access to the bkp regs
	PWR->CR &= ~ PWR_CR_DBP;
}

/* ------------------------------------------------------------ */
//! Get loader status from backup reg
upgrade_status upgrade_getStatus(void)
{
	//Enable access to the bkp regs
	PWR->CR |= PWR_CR_DBP;
	if(BKP->DR1 != BACKUP_MAGIC_KEY)
	{
		return UPGRADE_STATUS_NONE;
	}

	upgrade_status cmd = BKP->DR2 - BACKUP_MAGIC_OFFSET;

	//Disable access to the bkp regs
	PWR->CR &= ~ PWR_CR_DBP;

	if(cmd>=UPGRADE_STATUS_REQ && cmd<=LOADER_STATUS_FAILED)
	{
		return cmd;
	}
	else
	{
		return UPGRADE_STATUS_NONE;
	}
}
/* ------------------------------------------------------------ */
