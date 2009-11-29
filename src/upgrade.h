/* ------------------------------------------------------------ */
/*
 * loader.h
 *
 *  Created on: 2009-10-11
 *      Author: lucck
 */
/* ------------------------------------------------------------ */
#ifndef LOADER_H_
#define LOADER_H_

/* ------------------------------------------------------------ */
//! Enable access to the loader registers
void upgrade_init(void);

/* ------------------------------------------------------------ */
typedef enum upgrade_status_e
{
	UPGRADE_STATUS_NONE,		//!None action required
	UPGRADE_STATUS_REQ,			//!Upgrade required
	UPGRADE_STATUS_FINISHED,	//!Upgrade complette and OK
	LOADER_STATUS_FAILED,		//!Upgrade failed
} upgrade_status;

/* ------------------------------------------------------------ */
//! Set loader status
void upgrade_setStatus(upgrade_status cmd_type);

/* ------------------------------------------------------------ */
//! Get loader status from backup reg
upgrade_status upgrade_getStatus(void);

/* ------------------------------------------------------------ */
#endif /* LOADER_H_ */
