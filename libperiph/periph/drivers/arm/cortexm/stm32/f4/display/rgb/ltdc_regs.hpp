/**
 * =====================================================================================
 * 	File: ltdc_regs.hpp
 * 	Created Date: Sunday, August 18th 2019, 8:47:07 pm
 * 	Author: Lucjan Bryndza
 * 	Copyright (c) 2019 BoFF
 * 
 * 	GPL v2/3
 * =====================================================================================
 */


#pragma once
#define	LTDC_SSCR	0x0008 /*   */
#define	 SSCR_HSW_S	16
#define	 SSCR_VSW_S	0
#define	LTDC_BPCR	0x000C /*   */
#define	 BPCR_AHBP_S	16
#define	 BPCR_AVBP_S	0
#define	LTDC_AWCR	0x0010 /*   */
#define	 AWCR_AAW_S	16
#define	 AWCR_AAH_S	0
#define	LTDC_TWCR	0x0014 /*   */
#define	 TWCR_TOTALW_S	16
#define	 TWCR_TOTALH_S	0
#define	LTDC_GCR	0x0018 /*   */
#define	 GCR_HSPOL	(1 << 31) /* Horizontal Synchronization Polarity */
#define	 GCR_VSPOL	(1 << 30) /* Vertical Synchronization Polarity */
#define	 GCR_DEPOL	(1 << 29) /* Data Enable Polarity */
#define	 GCR_PCPOL	(1 << 28) /* Pixel Clock Polarity */
#define	 GCR_LTDCEN	(1 << 0) /* LCD-TFT controller enable bit */
#define	LTDC_SRCR	0x0024 /*   */
#define	 SRCR_IMR	(1 << 0) /* Immediate Reload */
#define	LTDC_BCCR	0x002C /*   */
#define	LTDC_IER	0x0034 /*   */
#define	LTDC_ISR	0x0038 /*   */
#define	LTDC_ICR	0x003C /*   */
#define	LTDC_LIPCR	0x0040 /*   */
#define	LTDC_CPSR	0x0044 /*   */
#define	LTDC_CDSR	0x0048 /*   */

#define	LTDC_LCR(n)	(0x0084 + 0x80 * n)
#define	 LCR_LEN	(1 << 0) /* Layer Enable */
#define	LTDC_LWHPCR(n)	(0x0088 + 0x80 * n)
#define	LTDC_LWVPCR(n)	(0x008C + 0x80 * n)
#define	LTDC_LCKCR(n)	(0x0090 + 0x80 * n)
#define	LTDC_LPFCR(n)	(0x0094 + 0x80 * n)
#define	LTDC_LCACR(n)	(0x0098 + 0x80 * n)
#define	LTDC_LDCCR(n)	(0x009C + 0x80 * n)
#define	LTDC_LBFCR(n)	(0x00A0 + 0x80 * n)
#define	LTDC_LCFBAR(n)	(0x00AC + 0x80 * n) /* Layerx Color Frame Buffer Address */
#define	LTDC_LCFBLR(n)	(0x00B0 + 0x80 * n)
#define	LTDC_LCFBLNR(n)	(0x00B4 + 0x80 * n)
#define	LTDC_LCLUTWR(n)	(0x00C4 + 0x80 * n)

#define	LTDC_L1CR	0x0084 /*   */
#define	LTDC_L1WHPCR	0x0088 /*   */
#define	LTDC_L1WVPCR	0x008C /*   */
#define	LTDC_L1CKCR	0x0090 /*   */
#define	LTDC_L1PFCR	0x0094 /*   */
#define	LTDC_L1CACR	0x0098 /*   */
#define	LTDC_L1DCCR	0x009C /*   */
#define	LTDC_L1BFCR	0x00A0 /*   */
#define	LTDC_L1CFBAR	0x00AC /* Layerx Color Frame Buffer Address */
#define	LTDC_L1CFBLR	0x00B0 /*   */
#define	LTDC_L1CFBLNR	0x00B4 /*   */
#define	LTDC_L1CLUTWR	0x00C4 /*   */

#define	LTDC_L2CR	0x0104 /*   */
#define	LTDC_L2WHPCR	0x0108 /*   */
#define	LTDC_L2WVPCR	0x010C /*   */
#define	LTDC_L2CKCR	0x0110 /*   */
#define	LTDC_L2PFCR	0x0114 /*   */
#define	LTDC_L2CACR	0x0118 /*   */
#define	LTDC_L2DCCR	0x011C /*   */
#define	LTDC_L2BFCR	0x0120 /*   */
#define	LTDC_L2CFBAR	0x012C /*   */
#define	LTDC_L2CFBLR	0x0130 /*   */
#define	LTDC_L2CFBLNR	0x0134 /*   */
#define	LTDC_L2CLUTWR	0x0144 /*   */
