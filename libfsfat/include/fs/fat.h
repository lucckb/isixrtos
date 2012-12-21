/*
 * fat.h
 *
 *  Created on: 22-11-2012
 *      Author: lucck
 */

#ifndef ISIX_FATFS_H_
#define ISIX_FATFS_H_
#include "../ff.h"

#ifdef __cplusplus
extern "C" {
#endif
	unsigned char disk_add( unsigned char disk_id, void *ops );

#ifdef __cplusplus
}
#endif
#endif
