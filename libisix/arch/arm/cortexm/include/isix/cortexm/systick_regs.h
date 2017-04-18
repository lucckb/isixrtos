#include <isix/arch/io.h>
#include "memorymap.h"

#pragma once

/* --- SYSTICK registers --------------------------------------------------- */

/* Control and status register (STK_CTRL) */
#define STK_CSR				MMIO32(SYS_TICK_BASE + 0x00)

/* reload value register (STK_LOAD) */
#define STK_RVR				MMIO32(SYS_TICK_BASE + 0x04)

/* current value register (STK_VAL) */
#define STK_CVR				MMIO32(SYS_TICK_BASE + 0x08)

/* calibration value register (STK_CALIB) */
#define STK_CALIB			MMIO32(SYS_TICK_BASE + 0x0C)

/* --- STK_CSR values ------------------------------------------------------ */
/* Bits [31:17] Reserved, must be kept cleared. */
/* COUNTFLAG: */
#define STK_CSR_COUNTFLAG		(1 << 16)

/* Bits [15:3] Reserved, must be kept cleared. */
/* CLKSOURCE: Clock source selection */
#define STK_CSR_CLKSOURCE_LSB		2
#define STK_CSR_CLKSOURCE		(1 << STK_CSR_CLKSOURCE_LSB)

/** @defgroup systick_clksource Clock source selection
@ingroup CM3_systick_defines

@{*/
#if defined(__ARM_ARCH_6M__)
#define STK_CSR_CLKSOURCE_EXT		(0 << STK_CSR_CLKSOURCE_LSB)
#define STK_CSR_CLKSOURCE_AHB		(1 << STK_CSR_CLKSOURCE_LSB)
#else
#define STK_CSR_CLKSOURCE_AHB_DIV8	(0 << STK_CSR_CLKSOURCE_LSB)
#define STK_CSR_CLKSOURCE_AHB		(1 << STK_CSR_CLKSOURCE_LSB)
#endif
/**@}*/

/* TICKINT: SysTick exception request enable */
#define STK_CSR_TICKINT			(1 << 1)
/* ENABLE: Counter enable */
#define STK_CSR_ENABLE			(1 << 0)

/* --- STK_RVR values ------------------------------------------------------ */
/* Bits [31:24] Reserved, must be kept cleared. */
/* RELOAD[23:0]: RELOAD value */
#define STK_RVR_RELOAD			0x00FFFFFF


/* --- STK_CVR values ------------------------------------------------------ */
/* Bits [31:24] Reserved, must be kept cleared. */
/* CURRENT[23:0]: Current counter value */
#define STK_CVR_CURRENT			0x00FFFFFF


/* --- STK_CALIB values ---------------------------------------------------- */
/* NOREF: NOREF flag */
#define STK_CALIB_NOREF			(1 << 31)
/* SKEW: SKEW flag */
#define STK_CALIB_SKEW			(1 << 30)
/* Bits [29:24] Reserved, must be kept cleared. */
/* TENMS[23:0]: Calibration value */
#define STK_CALIB_TENMS			0x00FFFFFF

