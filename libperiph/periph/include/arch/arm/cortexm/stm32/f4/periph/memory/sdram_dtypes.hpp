/*
 * =====================================================================================
 *
 *       Filename:  stm32_dma_v2.hpp
 *
 *    Description:  SDRAM driver for ISIX devices
 *
 *        Version:  1.0
 *        Created:  10.08.2019 13:58:35
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Lucjan Bryndza (LB), lbryndza.p@boff.pl
 *   Organization:  BoFF
 *
 * =====================================================================================
 */

#pragma once
#include <periph/dt/types.hpp>
#include <cstdint>

namespace periph::memory {
    struct sdcr_reg {
	uint16_t rpipe:2;
	uint16_t rburst:1;	/* Burst read */
	uint16_t sdclk:2;	/* SDRAM clock configuration */
	uint16_t wp:1;		/* Write protection */
	uint16_t cas:2;		/* CAS Latency */
	uint16_t nb:1;		/* Number of internal banks */
	uint16_t mwid:2;	/* Memory data bus width */
	uint16_t nr:2;		/* Number of row address bits */
	uint16_t nc:2;		/* Number of column address bits */
};

struct sdtr_reg {
	uint32_t trcd:4;	/* Row to column delay */
	uint32_t trp:4;		/* Row precharge delay */
	uint32_t twr:4;		/* Recovery delay */
	uint32_t trc:4;		/* Row cycle delay */
	uint32_t tras:4;	/* Self refresh time */
	uint32_t txsr:4;	/* Exit Self-refresh delay */
	uint32_t tmrd:4;	/* Load Mode Register to Active */
};

struct sdcmr_reg {
	uint32_t burstlen;
};

struct sdram_bank {
	struct sdcr_reg sdcr;
	struct sdtr_reg sdtr;
	struct sdcmr_reg sdcmr;
};

struct sdram : public dt::device_conf_base {
	uint16_t sdrtr;
	uint8_t nrfs;
	struct sdram_bank bank1;
	struct sdram_bank bank2;
};

}