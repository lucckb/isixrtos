/******************** (C) COPYRIGHT 2010 STMicroelectronics ********************
* File Name          : usb_regs.h
* Author             : MCD Application Team & MP
* Version            : V3.2.1 with corrections added by MP
* Date               : 29.09.2012
* Description        : Interface prototype functions to USB cell registers
********************************************************************************
* THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
* WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE TIME.
* AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY DIRECT,
* INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING FROM THE
* CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE CODING
* INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
*******************************************************************************/

#ifndef _USB_REGS_H
#define _USB_REGS_H 1

/* MP: micros with full speed USB device peripheral */
#if defined STM32F10X_MD || defined STM32F10X_HD  || \
    defined STM32F30X    || defined STM32F37X     || \
    defined STM32L1XX_MD || defined STM32L1XX_MDP || defined STM32L1XX_HD

/* MP: added helpful header */
#include <stm32.h>

#ifdef __cplusplus
extern "C" {
#endif

/* double buffered endpoint direction */
typedef enum _EP_DBUF_DIR {
  EP_DBUF_ERR,
  EP_DBUF_OUT,
  EP_DBUF_IN
} EP_DBUF_DIR;

/* endpoint buffer number */
enum EP_BUF_NUM {
  EP_NOBUF,
  EP_BUF0,
  EP_BUF1
};

/* Exported constants --------------------------------------------------------*/

/* MP: Microcontroller supports only 8 endpoints. */
#define _USB_EP_MAX_COUNT  8

#define RegBase (0x40005C00L) /* USB_IP Peripheral Registers base address */
#define PMAAddr (0x40006000L) /* USB_IP Packet Memory Area base address   */

/******************************************************************************/
/*                         General registers                                  */
/******************************************************************************/

/* Control register */
#define CNTR    ((__IO unsigned *)(RegBase + 0x40))
/* Interrupt status register */
#define ISTR    ((__IO unsigned *)(RegBase + 0x44))
/* Frame number register */
#define FNR     ((__IO unsigned *)(RegBase + 0x48))
/* Device address register */
#define DADDR   ((__IO unsigned *)(RegBase + 0x4C))
/* Buffer Table address register */
#define BTABLE  ((__IO unsigned *)(RegBase + 0x50))

/******************************************************************************/
/*                         Endpoint registers                                 */
/******************************************************************************/

#define EP0REG  ((__IO unsigned *)(RegBase)) /* endpoint 0 register address */

/* Endpoint Addresses (w/direction) */
/* MP: Use ENDP1 | ENP_IN or ENDP1 | ENP_OUT, etc., see also usb_def.h.
#define EP0_OUT     ((uint8_t)0x00)
#define EP0_IN      ((uint8_t)0x80)
#define EP1_OUT     ((uint8_t)0x01)
#define EP1_IN      ((uint8_t)0x81)
#define EP2_OUT     ((uint8_t)0x02)
#define EP2_IN      ((uint8_t)0x82)
#define EP3_OUT     ((uint8_t)0x03)
#define EP3_IN      ((uint8_t)0x83)
#define EP4_OUT     ((uint8_t)0x04)
#define EP4_IN      ((uint8_t)0x84)
#define EP5_OUT     ((uint8_t)0x05)
#define EP5_IN      ((uint8_t)0x85)
#define EP6_OUT     ((uint8_t)0x06)
#define EP6_IN      ((uint8_t)0x86)
#define EP7_OUT     ((uint8_t)0x07)
#define EP7_IN      ((uint8_t)0x87)
*/

/* Endpoints enumeration */
/* MP: This is a part of the standard, hence it is moved to usb_def.h.
#define ENDP0       ((uint8_t)0)
#define ENDP1       ((uint8_t)1)
#define ENDP2       ((uint8_t)2)
#define ENDP3       ((uint8_t)3)
#define ENDP4       ((uint8_t)4)
#define ENDP5       ((uint8_t)5)
#define ENDP6       ((uint8_t)6)
#define ENDP7       ((uint8_t)7)
*/

/******************************************************************************/
/*                       ISTR interrupt events                                */
/******************************************************************************/

#define ISTR_CTR    (0x8000) /* Correct TRansfer (clear-only bit) */
#define ISTR_DOVR   (0x4000) /* DMA OVeR/underrun (clear-only bit) */
#define ISTR_ERR    (0x2000) /* ERRor (clear-only bit) */
#define ISTR_WKUP   (0x1000) /* WaKe UP (clear-only bit) */
#define ISTR_SUSP   (0x0800) /* SUSPend (clear-only bit) */
#define ISTR_RESET  (0x0400) /* RESET (clear-only bit) */
#define ISTR_SOF    (0x0200) /* Start Of Frame (clear-only bit) */
#define ISTR_ESOF   (0x0100) /* Expected Start Of Frame (clear-only bit) */

#define ISTR_DIR    (0x0010)  /* DIRection of transaction (read-only bit) */
#define ISTR_EP_ID  (0x000F)  /* EndPoint IDentifier (read-only bit)  */

#define CLR_CTR   ((uint16_t)~ISTR_CTR) /* clear Correct TRansfer bit */
#define CLR_DOVR  (~ISTR_DOVR)          /* clear DMA OVeR/underrun bit */
#define CLR_ERR   (~ISTR_ERR)           /* clear ERRor bit */
#define CLR_WKUP  (~ISTR_WKUP)          /* clear WaKe UP bit */
#define CLR_SUSP  (~ISTR_SUSP)          /* clear SUSPend bit */
#define CLR_RESET (~ISTR_RESET)         /* clear RESET bit */
#define CLR_SOF   (~ISTR_SOF)           /* clear Start Of Frame bit */
#define CLR_ESOF  (~ISTR_ESOF)          /* clear Expected Start Of Frame bit */

/******************************************************************************/
/*             CNTR control register bits definitions                         */
/******************************************************************************/

#define CNTR_CTRM   (0x8000) /* Correct TRansfer Mask */
#define CNTR_DOVRM  (0x4000) /* DMA OVeR/underrun Mask */
#define CNTR_ERRM   (0x2000) /* ERRor Mask */
#define CNTR_WKUPM  (0x1000) /* WaKe UP Mask */
#define CNTR_SUSPM  (0x0800) /* SUSPend Mask */
#define CNTR_RESETM (0x0400) /* RESET Mask   */
#define CNTR_SOFM   (0x0200) /* Start Of Frame Mask */
#define CNTR_ESOFM  (0x0100) /* Expected Start Of Frame Mask */

#define CNTR_RESUME (0x0010) /* RESUME request */
#define CNTR_FSUSP  (0x0008) /* Force SUSPend */
#define CNTR_LPMODE (0x0004) /* Low-power MODE */
#define CNTR_PDWN   (0x0002) /* Power DoWN */
#define CNTR_FRES   (0x0001) /* Force USB RESet */

/******************************************************************************/
/*                FNR Frame Number Register bit definitions                   */
/******************************************************************************/

#define FNR_RXDP (0x8000) /* status of D+ data line */
#define FNR_RXDM (0x4000) /* status of D- data line */
#define FNR_LCK  (0x2000) /* LoCKed */
#define FNR_LSOF (0x1800) /* Lost SOF */
#define FNR_FN   (0x07FF) /* Frame Number */

/******************************************************************************/
/*               DADDR Device ADDRess bit definitions                         */
/******************************************************************************/

#define DADDR_EF  (0x80)
#define DADDR_ADD (0x7F)

/******************************************************************************/
/*                            Endpoint register                               */
/******************************************************************************/

/* bit positions */
#define EP_CTR_RX      (0x8000) /* EndPoint Correct TRansfer RX */
#define EP_DTOG_RX     (0x4000) /* EndPoint Data TOGGLE RX */
#define EPRX_STAT      (0x3000) /* EndPoint RX STATus bit field */
#define EP_SETUP       (0x0800) /* EndPoint SETUP */
#define EP_T_FIELD     (0x0600) /* EndPoint TYPE */
#define EP_KIND        (0x0100) /* EndPoint KIND */
#define EP_CTR_TX      (0x0080) /* EndPoint Correct TRansfer TX */
#define EP_DTOG_TX     (0x0040) /* EndPoint Data TOGGLE TX */
#define EPTX_STAT      (0x0030) /* EndPoint TX STATus bit field */
#define EPADDR_FIELD   (0x000F) /* EndPoint ADDRess FIELD */

/* EndPoint REGister MASK (no toggle fields) */
#define EPREG_MASK (EP_CTR_RX|EP_SETUP|EP_T_FIELD|EP_KIND|EP_CTR_TX|EPADDR_FIELD)

/* EP_TYPE[1:0] EndPoint TYPE */
/* #define EP_TYPE_MASK   (0x0600) */ /* EndPoint TYPE Mask */
#define EP_BULK        (0x0000) /* EndPoint BULK */
#define EP_CONTROL     (0x0200) /* EndPoint CONTROL */
#define EP_ISOCHRONOUS (0x0400) /* EndPoint ISOCHRONOUS */
#define EP_INTERRUPT   (0x0600) /* EndPoint INTERRUPT */
#define EP_T_MASK      (~EP_T_FIELD & EPREG_MASK)

/* EP_KIND EndPoint KIND */
#define EPKIND_MASK    (~EP_KIND & EPREG_MASK)

/* STAT_TX[1:0] STATus for TX transfer */
#define EP_TX_DIS      (0x0000) /* EndPoint TX DISabled */
#define EP_TX_STALL    (0x0010) /* EndPoint TX STALLed */
#define EP_TX_NAK      (0x0020) /* EndPoint TX NAKed */
#define EP_TX_VALID    (0x0030) /* EndPoint TX VALID */
#define EPTX_DTOG1     (0x0010) /* EndPoint TX Data TOGgle bit1 */
#define EPTX_DTOG2     (0x0020) /* EndPoint TX Data TOGgle bit2 */
#define EPTX_DTOGMASK  (EPTX_STAT | EPREG_MASK)

/* STAT_RX[1:0] STATus for RX transfer */
#define EP_RX_DIS      (0x0000) /* EndPoint RX DISabled */
#define EP_RX_STALL    (0x1000) /* EndPoint RX STALLed */
#define EP_RX_NAK      (0x2000) /* EndPoint RX NAKed */
#define EP_RX_VALID    (0x3000) /* EndPoint RX VALID */
#define EPRX_DTOG1     (0x1000) /* EndPoint RX Data TOGgle bit1 */
#define EPRX_DTOG2     (0x2000) /* EndPoint RX Data TOGgle bit1 */
#define EPRX_DTOGMASK  (EPRX_STAT | EPREG_MASK)

/* Exported macros -----------------------------------------------------------*/

/* SetCNTR */
#define _SetCNTR(wRegValue)  *CNTR  = (uint16_t)(wRegValue)

/* SetISTR */
#define _SetISTR(wRegValue)  *ISTR  = (uint16_t)(wRegValue)

/* SetDADDR */
#define _SetDADDR(wRegValue) *DADDR = (uint16_t)(wRegValue)

/* SetBTABLE */
#define _SetBTABLE(wRegValue) \
 *BTABLE = (uint16_t)((wRegValue) & 0xFFF8)

/* GetCNTR */
#define _GetCNTR()   ((uint16_t) *CNTR)

/* GetISTR */
#define _GetISTR()   ((uint16_t) *ISTR)

/* GetFNR */
#define _GetFNR()    ((uint16_t) *FNR)

/* GetDADDR */
#define _GetDADDR()  ((uint16_t) *DADDR)

/* GetBTABLE */
#define _GetBTABLE() ((uint16_t) *BTABLE)

/* SetENDPOINT */
#define _SetENDPOINT(bEpNum, wRegValue) \
  *(EP0REG + (bEpNum)) = (uint16_t)(wRegValue)

/* GetENDPOINT */
#define _GetENDPOINT(bEpNum) ((uint16_t)(*(EP0REG + (bEpNum))))

/*******************************************************************************
* Macro Name  : SetEPType
* Description : Sets the type in the endpoint register (bits EP_TYPE[1:0]).
* Input       : bEpNum: the physical endpoint number
*               wType:  the endpoint type
* Output      : none
* Return      : none
*******************************************************************************/
#define _SetEPType(bEpNum, wType) \
  _SetENDPOINT(bEpNum, ((_GetENDPOINT(bEpNum) & EP_T_MASK) | (wType)))

/*******************************************************************************
* Macro Name  : GetEPType
* Description : Gets the type in the endpoint register (bits EP_TYPE[1:0]).
* Input       : bEpNum: the physical endpoint number
* Output      : none
* Return      : the endpoint type
*******************************************************************************/
#define _GetEPType(bEpNum) (_GetENDPOINT(bEpNum) & EP_T_FIELD)

/*******************************************************************************
* Macro Name  : SetEPTxStatus
* Description : Sets the status for tx transfer (bits STAT_TX[1:0]).
* Input       : bEpNum: the physical endpoint number
*               wState: new state
* Output      : none
* Return      : none
*******************************************************************************/
/* MP: an optimized version of the macro */
#define _SetEPTxStatus(bEpNum, wState) \
  _SetENDPOINT(bEpNum, ((_GetENDPOINT(bEpNum) & EPTX_DTOGMASK) ^ (wState)) \
                       | EP_CTR_RX | EP_CTR_TX)
#if 0
/* MP: the oryginal version of the macro */
#define _SetEPTxStatus(bEpNum,wState) {                   \
  register uint16_t _wRegVal;                             \
  _wRegVal = _GetENDPOINT(bEpNum) & EPTX_DTOGMASK;        \
  /* toggle first bit ? */                                \
  if ((EPTX_DTOG1 & wState) != 0)                         \
    _wRegVal ^= EPTX_DTOG1;                               \
  /* toggle second bit ?  */                              \
  if ((EPTX_DTOG2 & wState) != 0)                         \
    _wRegVal ^= EPTX_DTOG2;                               \
  _SetENDPOINT(bEpNum, (_wRegVal | EP_CTR_RX|EP_CTR_TX)); \
} /* _SetEPTxStatus */
#endif

/*******************************************************************************
* Macro Name  : SetEPRxStatus
* Description : Sets the status for rx transfer (bits STAT_RX[1:0]).
* Input       : bEpNum: the physical endpoint number
*               wState: new state
* Output      : none
* Return      : none
*******************************************************************************/
/* MP: an optimized version of the macro */
#define _SetEPRxStatus(bEpNum, wState) \
  _SetENDPOINT(bEpNum, ((_GetENDPOINT(bEpNum) & EPRX_DTOGMASK) ^ (wState)) \
                       | EP_CTR_RX | EP_CTR_TX)
#if 0
/* MP: the oryginal version of the macro */
#define _SetEPRxStatus(bEpNum,wState) {                   \
  register uint16_t _wRegVal;                             \
  _wRegVal = _GetENDPOINT(bEpNum) & EPRX_DTOGMASK;        \
  /* toggle first bit ? */                                \
  if ((EPRX_DTOG1 & wState) != 0)                         \
    _wRegVal ^= EPRX_DTOG1;                               \
  /* toggle second bit ? */                               \
  if ((EPRX_DTOG2 & wState) != 0)                         \
    _wRegVal ^= EPRX_DTOG2;                               \
  _SetENDPOINT(bEpNum, (_wRegVal | EP_CTR_RX|EP_CTR_TX)); \
} /* _SetEPRxStatus */
#endif

/*******************************************************************************
* Macro Name  : SetEPRxTxStatus
* Description : Sets the status for rx & tx (bits STAT_TX[1:0] & STAT_RX[1:0]).
* Input       : bEpNum: the physical endpoint number
*               wStateRx: new state
*               wStateTx: new state
* Output      : none
* Return      : none
*******************************************************************************/
/* MP: an optimized version of the macro */
#define _SetEPRxTxStatus(bEpNum, wStateRx, wStateTx) \
  _SetENDPOINT(bEpNum, ((_GetENDPOINT(bEpNum) & (EPRX_DTOGMASK | EPTX_DTOGMASK)) ^ \
                       ((wStateRx) | (wStateTx))) | EP_CTR_RX | EP_CTR_TX)
#if 0
/* MP: the oryginal version of the macro */
#define _SetEPRxTxStatus(bEpNum,wStaterx,wStatetx) {              \
  register uint32_t _wRegVal;                                     \
  _wRegVal = _GetENDPOINT(bEpNum) & (EPRX_DTOGMASK | EPTX_STAT) ; \
  /* toggle first bit ? */                                        \
  if ((EPRX_DTOG1 & wStaterx) != 0)                               \
    _wRegVal ^= EPRX_DTOG1;                                       \
  /* toggle second bit ? */                                       \
  if ((EPRX_DTOG2 & wStaterx) != 0)                               \
    _wRegVal ^= EPRX_DTOG2;                                       \
  /* toggle first bit ? */                                        \
  if ((EPTX_DTOG1 & wStatetx) != 0)                               \
    _wRegVal ^= EPTX_DTOG1;                                       \
  /* toggle second bit ?  */                                      \
  if((EPTX_DTOG2 & wStatetx)!= 0)                                 \
    _wRegVal ^= EPTX_DTOG2;                                       \
  _SetENDPOINT(bEpNum, _wRegVal | EP_CTR_RX|EP_CTR_TX);           \
} /* _SetEPRxTxStatus */
#endif

/*******************************************************************************
* Macro Name  : GetEPTxStatus / GetEPRxStatus
* Description : Gets the status for tx/rx transfer (bits STAT_TX[1:0] &
*               STAT_RX[1:0]).
* Input       : bEpNum: the physical endpoint number
* Output      : none
* Return      : status
*******************************************************************************/
#define _GetEPTxStatus(bEpNum) ((uint16_t)_GetENDPOINT(bEpNum) & EPTX_STAT)
#define _GetEPRxStatus(bEpNum) ((uint16_t)_GetENDPOINT(bEpNum) & EPRX_STAT)

/*******************************************************************************
* Macro Name  : SetEPTxValid / SetEPRxValid
* Description : Sets directly the VALID tx/rx-status into the enpoint register.
* Input       : bEpNum: the physical endpoint number
* Output      : none
* Return      : none
*******************************************************************************/
#define _SetEPTxValid(bEpNum) _SetEPTxStatus(bEpNum, EP_TX_VALID)
#define _SetEPRxValid(bEpNum) _SetEPRxStatus(bEpNum, EP_RX_VALID)

/*******************************************************************************
* Macro Name  : GetTxStallStatus / GetRxStallStatus
* Description : Checks stall condition in an endpoint.
* Input       : bEpNum: the physical endpoint number
* Output      : none
* Return      : TRUE iff the endpoint in stall condition
*******************************************************************************/
#define _GetTxStallStatus(bEpNum) \
  (_GetEPTxStatus(bEpNum) == EP_TX_STALL)
#define _GetRxStallStatus(bEpNum) \
  (_GetEPRxStatus(bEpNum) == EP_RX_STALL)

/*******************************************************************************
* Macro Name  : SetEP_KIND / ClearEP_KIND
* Description : Set & clear EP_KIND bit.
* Input       : bEpNum: the physical endpoint number
* Output      : none
* Return      : none
*******************************************************************************/
#define _SetEP_KIND(bEpNum) \
  _SetENDPOINT(bEpNum, (EP_CTR_RX | EP_CTR_TX | \
                        ((_GetENDPOINT(bEpNum) | EP_KIND) & EPREG_MASK)))
#define _ClearEP_KIND(bEpNum) \
 _SetENDPOINT(bEpNum, (EP_CTR_RX | EP_CTR_TX | \
                       (_GetENDPOINT(bEpNum) & EPKIND_MASK)))

/*******************************************************************************
* Macro Name  : Set_Status_Out / Clear_Status_Out
* Description : Sets/clears directly STATUS_OUT bit in the endpoint register.
* Input       : bEpNum: the physical endpoint number
* Output      : none
* Return      : none
*******************************************************************************/
#define _Set_Status_Out(bEpNum)   _SetEP_KIND(bEpNum)
#define _Clear_Status_Out(bEpNum) _ClearEP_KIND(bEpNum)

/*******************************************************************************
* Macro Name  : SetEPDoubleBuff / ClearEPDoubleBuff
* Description : Sets/clears directly EP_KIND bit in the endpoint register.
* Input       : bEpNum: the physical endpoint number
* Output      : none
* Return      : none
*******************************************************************************/
#define _SetEPDoubleBuff(bEpNum)   _SetEP_KIND(bEpNum)
#define _ClearEPDoubleBuff(bEpNum) _ClearEP_KIND(bEpNum)

/*******************************************************************************
* Macro Name  : ClearEP_CTR_RX / ClearEP_CTR_TX
* Description : Clears bit CTR_RX / CTR_TX in the endpoint register.
* Input       : bEpNum: the physical endpoint number
* Output      : none
* Return      : none
*******************************************************************************/
#define _ClearEP_CTR_RX(bEpNum) \
  _SetENDPOINT(bEpNum, _GetENDPOINT(bEpNum) & (~EP_CTR_RX) & EPREG_MASK)
#define _ClearEP_CTR_TX(bEpNum) \
  _SetENDPOINT(bEpNum, _GetENDPOINT(bEpNum) & (~EP_CTR_TX) & EPREG_MASK)

/*******************************************************************************
* Macro Name  : ToggleDTOG_RX / ToggleDTOG_TX
* Description : Toggles DTOG_RX / DTOG_TX bit in the endpoint register.
* Input       : bEpNum: the physical endpoint number
* Output      : none
* Return      : none
*******************************************************************************/
#define _ToggleDTOG_RX(bEpNum) \
  _SetENDPOINT(bEpNum, EP_CTR_RX | EP_CTR_TX | EP_DTOG_RX | \
                       (_GetENDPOINT(bEpNum) & EPREG_MASK))
#define _ToggleDTOG_TX(bEpNum) \
  _SetENDPOINT(bEpNum, EP_CTR_RX | EP_CTR_TX | EP_DTOG_TX | \
                       (_GetENDPOINT(bEpNum) & EPREG_MASK))

/*******************************************************************************
* Macro Name  : ClearDTOG_RX / ClearDTOG_TX
* Description : Clears DTOG_RX / DTOG_TX bit in the endpoint register.
* Input       : bEpNum: the physical endpoint number
* Output      : none
* Return      : none
*******************************************************************************/
#define _ClearDTOG_RX(bEpNum) {                   \
  if ((_GetENDPOINT(bEpNum) & EP_DTOG_RX) != 0) { \
    _ToggleDTOG_RX(bEpNum);                       \
  }                                               \
}
#define _ClearDTOG_TX(bEpNum) {                   \
  if ((_GetENDPOINT(bEpNum) & EP_DTOG_TX) != 0) { \
    _ToggleDTOG_TX(bEpNum);                       \
  }                                               \
}

/*******************************************************************************
* Macro Name  : SetEPAddress
* Description : Sets address in an endpoint register.
* Input       : bEpNum: the physical endpoint number
*               bUSBNum: the USB endpoint number
* Output      : none
* Return      : none
*******************************************************************************/
#define _SetEPNumber(bEpNum, bUSBNum) \
  _SetENDPOINT(bEpNum, EP_CTR_RX | EP_CTR_TX | \
                       (_GetENDPOINT(bEpNum) & EPREG_MASK) | (bUSBNum))

/*******************************************************************************
* Macro Name  : GetEPAddress
* Description  Gets address in an endpoint register.
* Input       : bEpNum: the physical endpoint number
* Output      : none
* Return      : none
*******************************************************************************/
#define _GetEPAddress(bEpNum) ((uint8_t)(_GetENDPOINT(bEpNum) & EPADDR_FIELD))

#define _pEPTxAddr(bEpNum)  ((uint32_t *)((_GetBTABLE() + (bEpNum) * 8    ) * 2 + PMAAddr))
#define _pEPTxCount(bEpNum) ((uint32_t *)((_GetBTABLE() + (bEpNum) * 8 + 2) * 2 + PMAAddr))
#define _pEPRxAddr(bEpNum)  ((uint32_t *)((_GetBTABLE() + (bEpNum) * 8 + 4) * 2 + PMAAddr))
#define _pEPRxCount(bEpNum) ((uint32_t *)((_GetBTABLE() + (bEpNum) * 8 + 6) * 2 + PMAAddr))

/*******************************************************************************
* Macro Name  : SetEPTxAddr / SetEPRxAddr
* Description : Sets address of the tx/rx buffer.
* Input       : bEpNum: the physical endpoint number
*               wAddr: the address to be set (must be word aligned)
* Output      : none
* Return      : none
*******************************************************************************/
#define _SetEPTxAddr(bEpNum,wAddr) (*_pEPTxAddr(bEpNum) = (((wAddr) >> 1) << 1))
#define _SetEPRxAddr(bEpNum,wAddr) (*_pEPRxAddr(bEpNum) = (((wAddr) >> 1) << 1))

/*******************************************************************************
* Macro Name  : GetEPTxAddr / GetEPRxAddr
* Description : Gets address of the tx/rx buffer.
* Input       : bEpNum: the physical endpoint number
* Output      : none
* Return      : the address of the buffer
*******************************************************************************/
#define _GetEPTxAddr(bEpNum) ((uint16_t)*_pEPTxAddr(bEpNum))
#define _GetEPRxAddr(bEpNum) ((uint16_t)*_pEPRxAddr(bEpNum))

/*******************************************************************************
* Macro Name  : SetEPCountRxReg
* Description : Sets size of the rx buffer.
* Input       : pdwReg: pointer to the USB_COUNTn register
*               wCount: buffer size
* Output      : none
* Return      : none
*******************************************************************************/
/* MP: We call these macros with proper walue of wCount. */
#define _BlocksOf32(dwReg, wCount, wNBlocks) { \
  wNBlocks = ((wCount) >> 5) - 1;              \
  /*                                           \
  if(((wCount) & 0x1f) == 0)                   \
    (wNBlocks)--;                              \
  */                                           \
  *pdwReg = (wNBlocks << 10) | 0x8000;         \
}

#define _BlocksOf2(dwReg, wCount, wNBlocks) { \
  wNBlocks = (wCount) >> 1;                   \
  /*                                          \
  if(((wCount) & 0x1) != 0)                   \
    (wNBlocks)++;                             \
  */                                          \
  *pdwReg = wNBlocks << 10;                   \
}

#define _SetEPCountRxReg(dwReg, wCount) { \
  uint16_t wNBlocks;                      \
  if ((wCount) > 62) {                    \
    _BlocksOf32(dwReg, wCount, wNBlocks); \
  }                                       \
  else {                                  \
    _BlocksOf2(dwReg, wCount, wNBlocks);  \
  }                                       \
}

/*******************************************************************************
* Macro Name  : SetEPTxCount / SetEPRxCount
* Description : Sets size of the tx/rx buffer.
* Input       : bEpNum: the physical endpoint number
*               wCount: the size of the buffer
* Output      : none
* Return      : none
*******************************************************************************/
#define _SetEPTxCount(bEpNum, wCount) (*_pEPTxCount(bEpNum) = wCount)
#define _SetEPRxCount(bEpNum, wCount) {   \
  uint32_t *pdwReg = _pEPRxCount(bEpNum); \
  _SetEPCountRxReg(pdwReg, wCount);       \
}

/*******************************************************************************
* Macro Name     : GetEPTxCount / GetEPRxCount
* Description    : Gets counter of the tx buffer.
* Input          : bEpNum: the physical endpoint number
* Output         : none
* Return         : the counter value
*******************************************************************************/
#define _GetEPTxCount(bEpNum) ((uint16_t)(*_pEPTxCount(bEpNum)) & 0x3ff)
#define _GetEPRxCount(bEpNum) ((uint16_t)(*_pEPRxCount(bEpNum)) & 0x3ff)

/*******************************************************************************
* Macro Name  : SetEPDblBuf0Addr / SetEPDblBuf1Addr
* Description : Sets buffer 0/1 address in a double buffer endpoint.
* Input       : bEpNum: the physical endpoint number
*             : wBuf0Addr: buffer 0 address
* Output      : none
* Return      : none
*******************************************************************************/
#define _SetEPDblBuf0Addr(bEpNum, wBuf0Addr) _SetEPTxAddr(bEpNum, wBuf0Addr)
#define _SetEPDblBuf1Addr(bEpNum, wBuf1Addr) _SetEPRxAddr(bEpNum, wBuf1Addr)

/*******************************************************************************
* Macro Name  : SetEPDblBuffAddr
* Description : Sets addresses in a double buffer endpoint.
* Input       : bEpNum: the physical endpoint number
*               wBuf0Addr: buffer 0 address
*               wBuf1Addr: buffer 1 address
* Output      : none
* Return      : none
*******************************************************************************/
#define _SetEPDblBuffAddr(bEpNum, wBuf0Addr, wBuf1Addr) { \
  _SetEPDblBuf0Addr(bEpNum, wBuf0Addr);                   \
  _SetEPDblBuf1Addr(bEpNum, wBuf1Addr);                   \
}

/*******************************************************************************
* Macro Name  : GetEPDblBuf0Addr / GetEPDblBuf1Addr
* Description : Gets buffer 0/1 address of a double buffer endpoint.
* Input       : bEpNum: the physical endpoint number
* Output      : none
* Return      : none
*******************************************************************************/
#define _GetEPDblBuf0Addr(bEpNum) _GetEPTxAddr(bEpNum)
#define _GetEPDblBuf1Addr(bEpNum) _GetEPRxAddr(bEpNum)

/*******************************************************************************
* Macro Name  : SetEPDblBuffCount / SetEPDblBuf0Count / SetEPDblBuf1Count
* Description : Sets buffer 0/1 size of a double buffer endpoint.
* Input       : bEpNum: the physical endpoint number
*             : bDir: endpoint dir EP_DBUF_OUT = OUT
*                                  EP_DBUF_IN  = IN
*             : wCount: the size value
* Output      : none
* Return      : none
*******************************************************************************/
#define _SetEPRxDblBuf0Count(bEpNum, wCount) { \
  uint32_t *pdwReg = _pEPTxCount(bEpNum);      \
  _SetEPCountRxReg(pdwReg, wCount);            \
}

#define _SetEPDblBuf0Count(bEpNum, bDir, wCount) { \
  if (bDir == EP_DBUF_OUT) {                       \
    /* OUT endpoint */                             \
    _SetEPRxDblBuf0Count(bEpNum, wCount);          \
  }                                                \
  else if (bDir == EP_DBUF_IN) {                   \
    /* IN endpoint */                              \
    *_pEPTxCount(bEpNum) = (uint32_t)(wCount);     \
  }                                                \
}

#define _SetEPDblBuf1Count(bEpNum, bDir, wCount)  { \
  if (bDir == EP_DBUF_OUT) {                        \
    /* OUT endpoint */                              \
    _SetEPRxCount(bEpNum, wCount);                  \
  }                                                 \
  else if(bDir == EP_DBUF_IN) {                     \
    /* IN endpoint */                               \
    *_pEPRxCount(bEpNum) = (uint32_t)(wCount);      \
  }                                                 \
}

#define _SetEPDblBuffCount(bEpNum, bDir, wCount) { \
  _SetEPDblBuf0Count(bEpNum, bDir, wCount);        \
  _SetEPDblBuf1Count(bEpNum, bDir, wCount);        \
}

/*******************************************************************************
* Macro Name  : GetEPDblBuf0Count / GetEPDblBuf1Count
* Description : Gets buffer 0/1 rx/tx counter for double buffering.
* Input       : bEpNum: the physical endpoint number
* Output      : none
* Return      : none
*******************************************************************************/
#define _GetEPDblBuf0Count(bEpNum) _GetEPTxCount(bEpNum)
#define _GetEPDblBuf1Count(bEpNum) _GetEPRxCount(bEpNum)

#endif /* MP: micros with full speed USB device peripheral */

#ifdef __cplusplus
}
#endif

#endif /* _USB_REGS_H */

/******************* (C) COPYRIGHT 2010 STMicroelectronics *****END OF FILE****/
