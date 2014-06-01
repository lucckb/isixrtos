#ifndef _USB_OTG_REGS_H
#define _USB_OTG_REGS_H 1

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#if defined STM32F10X_CL

  #define _USB_OTG_BASE_ADDR  0x50000000

  /* Core supports 4 device endpoints and 8 host channels. */
  #define _USB_EP_MAX_COUNT     4
  #define _USB_CHNNL_MAX_COUNT  8

  #define CONST_EP_MAX_COUNT     _USB_EP_MAX_COUNT
  #define CONST_CHNNL_MAX_COUNT  _USB_CHNNL_MAX_COUNT

  #define FIFO_DWORDS_SIZE  320

  #define OTG_FS_REGS_USED  1
  #define OTG_HS_REGS_USED  0

#elif defined STM32F2XX || defined STM32F4XX

  #define USB_OTG_FS_BASE_ADDR  0x50000000
  #define USB_OTG_HS_BASE_ADDR  0x40040000

  /* FS core supports 4 device endpoints and 8 host channels. */
  #define FS_EP_MAX_COUNT     4
  #define FS_CHNNL_MAX_COUNT  8

  /* HS core supports 6 device endpoints and 12 host channels. */
  #define HS_EP_MAX_COUNT     6
  #define HS_CHNNL_MAX_COUNT  12

  #if HS_EP_MAX_COUNT > FS_EP_MAX_COUNT
    #define CONST_EP_MAX_COUNT  HS_EP_MAX_COUNT
  #else
    #define CONST_EP_MAX_COUNT  FS_EP_MAX_COUNT
  #endif

  #if HS_CHNNL_MAX_COUNT > FS_CHNNL_MAX_COUNT
    #define CONST_CHNNL_MAX_COUNT  HS_CHNNL_MAX_COUNT
  #else
    #define CONST_CHNNL_MAX_COUNT  FS_CHNNL_MAX_COUNT
  #endif

  #define FIFO_DWORDS_SIZE  1024

  extern uint32_t _USB_OTG_BASE_ADDR;
  extern unsigned _USB_EP_MAX_COUNT;
  extern unsigned _USB_CHNNL_MAX_COUNT;

  #define USE_OTG_FS_REGS()                   \
    _USB_OTG_BASE_ADDR = USB_OTG_FS_BASE_ADDR, \
    _USB_EP_MAX_COUNT      = FS_EP_MAX_COUNT,      \
    _USB_CHNNL_MAX_COUNT   = FS_CHNNL_MAX_COUNT

  #define USE_OTG_HS_REGS()                   \
    _USB_OTG_BASE_ADDR = USB_OTG_HS_BASE_ADDR, \
    _USB_EP_MAX_COUNT      = HS_EP_MAX_COUNT,      \
    _USB_CHNNL_MAX_COUNT   = HS_CHNNL_MAX_COUNT

  #define OTG_FS_REGS_USED (_USB_OTG_BASE_ADDR == USB_OTG_FS_BASE_ADDR)
  #define OTG_HS_REGS_USED (_USB_OTG_BASE_ADDR == USB_OTG_HS_BASE_ADDR)

#endif

#if defined STM32F10X_CL || defined STM32F2XX || defined STM32F4XX

#define USB_MAX_HOST_PACKET_COUNT  1023

#define USB_OTG_CORE_GLOBAL_REG_OFFSET   0x0000
#define USB_OTG_HOST_GLOBAL_REG_OFFSET   0x0400
#define USB_OTG_HOST_CHANNEL_REG_OFFSET  0x0500
#define USB_OTG_DEV_GLOBAL_REG_OFFSET    0x0800
#define USB_OTG_DEV_IN_EP_REG_OFFSET     0x0900
#define USB_OTG_DEV_OUT_EP_REG_OFFSET    0x0B00
#define USB_OTG_PCGCCTL_OFFSET           0x0E00
#define USB_OTG_DATA_FIFO_OFFSET         0x1000
#define USB_OTG_DATA_FIFO_SIZE           0x1000

/*********************************************************************
* The USB_OTG_GREGS structure defines the relative field offsets for
* the core global control and status registers.
*********************************************************************/
typedef struct {
  volatile uint32_t GOTGCTL;       /* 000 control and status */
  volatile uint32_t GOTGINT;       /* 004 interrupt */
  volatile uint32_t GAHBCFG;       /* 008 AHB configuration */
  volatile uint32_t GUSBCFG;       /* 00C USB configuration */
  volatile uint32_t GRSTCTL;       /* 010 reset */
  volatile uint32_t GINTSTS;       /* 014 core interrupt */
  volatile uint32_t GINTMSK;       /* 018 interrupt mask */
  volatile uint32_t GRXSTSR;       /* 01C receive status debug read */
  volatile uint32_t GRXSTSP;       /* 020 receive status pop */
  volatile uint32_t GRXFSIZ;       /* 024 receive FIFO size */
  volatile uint32_t HNPTXFSIZ;     /* 028 host non-periodic transmit FIFO size */
  volatile uint32_t HNPTXSTS;      /* 02C host non-periodic transmit FIFO status */
  volatile uint32_t GI2CCTL;       /* 030 I2C access */
  volatile uint32_t reserved34;    /* 034 reserved */
  volatile uint32_t GCCFG;         /* 038 general core configuration */
  volatile uint32_t CID;           /* 03C core ID */
  volatile uint32_t reserved4[48]; /* 040 reserved */
  volatile uint32_t HPTXFSIZ;      /* 100 host periodic transmit FIFO size */
  volatile uint32_t DIEPTXFx[11];  /* 104 IN EP transmit FIFO size */
} USB_OTG_GREGS;

/* 028 device endpoint 0 transmit FIFO size */
#define DIEPTXF0  HNPTXFSIZ

#define P_USB_OTG_GREGS \
  ((USB_OTG_GREGS *)(_USB_OTG_BASE_ADDR + USB_OTG_CORE_GLOBAL_REG_OFFSET))

typedef union { /* offset 0x0008 */
  uint32_t d32;
  struct {
    uint32_t gintmsk      :  1; /* gint - do not change */
    uint32_t hbstlen      :  4;
    uint32_t dmaen        :  1;
    uint32_t reserved6    :  1;
    uint32_t txfelvl      :  1;
    uint32_t ptxfelvl     :  1;
    uint32_t reserved9_31 : 23;
  } b;
} USB_OTG_GAHBCFG_TypeDef;

typedef union { /* offset 0x000C */
  uint32_t d32;
  struct {
    uint32_t tocal         :  3;
    uint32_t phyif         :  1; /* undocumented in RM0008 and RM0033 */
    uint32_t ulpisel       :  1; /* undocumented in RM0008 and RM0033 */
    uint32_t fsintf        :  1; /* undocumented in RM0008 and RM0033 */
    uint32_t physel        :  1; /* undocumented in RM0008 and RM0033 */
    uint32_t ddrsel        :  1; /* undocumented in RM0008 and RM0033 */
    uint32_t srpcap        :  1;
    uint32_t hnpcap        :  1;
    uint32_t trdt          :  4; /* badly documented in RM0008 and RM0033 */
    uint32_t nptxfrwnden   :  1; /* undocumented in RM0008 and RM0033 */
    uint32_t phylpcs       :  1;
    uint32_t otgutmifssel  :  1; /* undocumented in RM0008 and RM0033 */
    uint32_t ulpifsls      :  1;
    uint32_t ulpiar        :  1;
    uint32_t ulpicsm       :  1;
    uint32_t ulpievbusd    :  1;
    uint32_t ulpievbusi    :  1;
    uint32_t tsdps         :  1;
    uint32_t pcci          :  1;
    uint32_t ptci          :  1;
    uint32_t ulpiipd       :  1;
    uint32_t reserved26_28 :  3;
    uint32_t fhmod         :  1;
    uint32_t fdmod         :  1;
    uint32_t ctxpkt        :  1;
  } b;
} USB_OTG_GUSBCFG_TypeDef;

typedef union { /* offset 0x0010 */
  uint32_t d32;
  struct {
    uint32_t csrst         :  1;
    uint32_t hsrst         :  1;
    uint32_t fcrst         :  1;
    uint32_t reserved3     :  1;
    uint32_t rxfflsh       :  1;
    uint32_t txfflsh       :  1;
    uint32_t txfnum        :  5;
    uint32_t reserved11_29 : 19;
    uint32_t dmareq        :  1;
    uint32_t ahbidl        :  1;
  } b;
} USB_OTG_GRSTCTL_TypeDef;

typedef union { /* offset 0x0014 */
  uint32_t d32;
  struct {
    uint32_t cmod          :  1;
    uint32_t mmis          :  1;
    uint32_t otgint        :  1;
    uint32_t sof           :  1;
    uint32_t rxflvl        :  1;
    uint32_t nptxfe        :  1;
    uint32_t ginakeff      :  1;
    uint32_t gonakeff      :  1; /* boutnakeff - do not change */
    uint32_t reserved8_9   :  2;
    uint32_t esusp         :  1;
    uint32_t usbsusp       :  1;
    uint32_t usbrst        :  1;
    uint32_t enumdne       :  1;
    uint32_t isoodrp       :  1;
    uint32_t eopf          :  1;
    uint32_t reserved16_17 :  2;
    uint32_t iepint        :  1;
    uint32_t oepint        :  1;
    uint32_t iisoixfr      :  1;
    uint32_t iisooxfr      :  1; /* ipxfr_incompisoout - do not change */
    uint32_t datafsusp     :  1;
    uint32_t reserved23    :  1;
    uint32_t hprtint       :  1;
    uint32_t hcint         :  1;
    uint32_t ptxfe         :  1;
    uint32_t reserved27    :  1;
    uint32_t cidschg       :  1;
    uint32_t discint       :  1;
    uint32_t srqint        :  1;
    uint32_t wkuint        :  1;
  } b;
} USB_OTG_GINTSTS_TypeDef;

typedef union { /* offset 0x0018 */
  uint32_t d32;
  struct {
    uint32_t reserved0   :  1;
    uint32_t mmism       :  1;
    uint32_t otgm        :  1; /* otgint - do not change */
    uint32_t sofm        :  1;
    uint32_t rxflvlm     :  1;
    uint32_t nptxfem     :  1;
    uint32_t ginakeffm   :  1;
    uint32_t gonakeffm   :  1;
    uint32_t reserved8_9 :  2;
    uint32_t esuspm      :  1;
    uint32_t usbsuspm    :  1;
    uint32_t usbrstm     :  1; /* usbrst - do not change */
    uint32_t enumdnem    :  1;
    uint32_t isoodrpm    :  1;
    uint32_t eopfm       :  1;
    uint32_t reserved16  :  1;
    uint32_t epmism      :  1;
    uint32_t iepim       :  1; /* iepint - do not change */
    uint32_t oepim       :  1; /* oepint - do not change */
    uint32_t iisoixfrm   :  1;
    uint32_t iisooxfrm   :  1; /* ipxfrm_iisooxfrm - do not change */
    uint32_t fsuspm      :  1;
    uint32_t reserved23  :  1;
    uint32_t hprtim      :  1; /* prtim - do not change */
    uint32_t hcim        :  1;
    uint32_t ptxfem      :  1;
    uint32_t reserved27  :  1;
    uint32_t cidschgm    :  1;
    uint32_t discim      :  1; /* discint - do not change */
    uint32_t srqim       :  1;
    uint32_t wkuim       :  1; /* wuim - do not change */
  } b;
} USB_OTG_GINTMSK_TypeDef;

typedef union { /* offset 0x0020 */
  uint32_t d32;
  struct {
    uint32_t ch_ep_num     :  4;
    uint32_t bcnt          : 11;
    uint32_t dpid          :  2;
    uint32_t pktsts        :  4;
    uint32_t frmnum        :  4;
    uint32_t reserved25_31 :  7;
  } b;
} USB_OTG_GRXSTS_TypeDef;

#define GRXSTS_DPID_DATA0  0
#define GRXSTS_DPID_DATA2  1
#define GRXSTS_DPID_DATA1  2
#define GRXSTS_DPID_MDATA  3

#define GRXSTS_PKTSTS_IN_RECEIVED        2
#define GRXSTS_PKTSTS_IN_COMPLETED       3
#define GRXSTS_PKTSTS_DATA_TOGGLE_ERROR  5
#define GRXSTS_PKTSTS_CHANNEL_HALTED     7

#define GRXSTS_PKTSTS_GLOBAL_OUT_NAK   1
#define GRXSTS_PKTSTS_OUT_RECEIVED     2
#define GRXSTS_PKTSTS_OUT_COMPLETED    3
#define GRXSTS_PKTSTS_SETUP_COMPLETED  4
#define GRXSTS_PKTSTS_SETUP_RECEIVED   6

typedef union { /* offset 0x002C */
  uint32_t d32;
  struct {
    uint32_t nptxfsav           : 16;
    uint32_t nptxqsav           :  8; /* nptqxsav - do not change */
    uint32_t nptxqtop_terminate :  1;
    uint32_t nptxqtop_timer     :  2;
    uint32_t nptxqtop_chnum     :  4;
    uint32_t reserved31         :  1;
  } b;
} USB_OTG_HNPTXSTS_TypeDef;

/* DIEPTXF0 offset 0x0028
   DIEPTXFx offset 0x0104 + 4 * (fifo_num - 1) */
typedef union {
  uint32_t d32;
  struct {
    uint32_t startaddr : 16;
    uint32_t depth     : 16;
  } b;
} USB_OTG_FIFOSIZE_TypeDef;

typedef union { /* offset 0x0030h */
  uint32_t d32;
  struct {
    uint32_t rwdata     :  8;
    uint32_t regaddr    :  8;
    uint32_t addr       :  7;
    uint32_t i2cen      :  1;
    uint32_t ack        :  1;
    uint32_t reserved25 :  1;
    uint32_t i2cdevaddr :  2; /* i2cdevadr - do not change */
    uint32_t i2cdatse0  :  1;
    uint32_t reserved29 :  1;
    uint32_t rw         :  1;
    uint32_t bsyndne    :  1;
  } b;
} USB_OTG_GI2CCTL_TypeDef;

typedef union { /* offset 0x0038h */
  uint32_t d32;
  struct {
    uint32_t reserved0_15  : 16;
    uint32_t pwrdwn        :  1;
    uint32_t i2cpaden      :  1;
    uint32_t vbusasen      :  1;
    uint32_t vbusbsen      :  1;
    uint32_t sofouten      :  1;
    uint32_t novbussens    :  1;
    uint32_t reserved22_31 : 10;
  } b;
} USB_OTG_GCCFG_TypeDef;

/*********************************************************************
* The USB_OTG_HOST structure defines the relative field offsets for
* the host-mode control and status global registers.
* These registers are visible only in the host mode and must not be
* accessed in the device mode, as the results are unknown.
*********************************************************************/
typedef struct {
  volatile uint32_t HCFG;          /* 400 configuration */
  volatile uint32_t HFIR;          /* 404 frame interval */
  volatile uint32_t HFNUM;         /* 408 frame number / frame time remaining */
  volatile uint32_t reserved0C;    /* 40C reserved */
  volatile uint32_t HPTXSTS;       /* 410 periodic transmit FIFO status */
  volatile uint32_t HAINT;         /* 414 all channels interrupt */
  volatile uint32_t HAINTMSK;      /* 418 all channels interrupt mask */
  volatile uint32_t reserved1C[9]; /* 41C reserved */
  volatile uint32_t HPRT;          /* 440 port control and status */
} USB_OTG_HREGS;

#define P_USB_OTG_HREGS \
  ((USB_OTG_HREGS *)(_USB_OTG_BASE_ADDR + USB_OTG_HOST_GLOBAL_REG_OFFSET))

typedef union { /* offset 0x0400 */
  uint32_t d32;
  struct {
    uint32_t fslspcs      :  2; /* the same coding as usb_speed_t */
    uint32_t fslss        :  1;
    uint32_t reserved3_31 : 29;
  } b;
} USB_OTG_HCFG_TypeDef;

/*
#define HCFG_FSLSPCS_30_60_MHZ  0
#define HCFG_FSLSPCS_48_MHZ     1
#define HCFG_FSLSPCS_6_MHZ      2
*/

typedef union { /* offset 0x0404 */
  uint32_t d32;
  struct {
    uint32_t frivl         : 16;
    uint32_t reserved16_31 : 16;
  } b;
} USB_OTG_HFIR_TypeDef;

typedef union { /* offset 0x0408 */
  uint32_t d32;
  struct {
    uint32_t frnum : 16;
    uint32_t ftrem : 16;
  } b;
} USB_OTG_HFNUM_TypeDef;

typedef union { /* offset 0x0410 */
  uint32_t d32;
  struct {
    uint32_t ptxfsav           : 16;
    uint32_t ptxqsav           :  8;
    uint32_t ptxqtop_terminate :  1;
    uint32_t ptxqtop_type      :  2;
    uint32_t ptxqtop_chnum     :  4;
    uint32_t ptxqtop_odd       :  1;
  } b;
} USB_OTG_HPTXSTS_TypeDef;

typedef union { /* offset 0x0414 */
  uint32_t d32;
  struct {
    uint32_t haint         : 16;
    uint32_t reserved16_31 : 16;
  } b;
} USB_OTG_HAINT_TypeDef;

typedef union { /* offset 0x0418 */
  uint32_t d32;
  struct {
    uint32_t haintm        : 16;
    uint32_t reserved16_31 : 16;
  } b;
} USB_OTG_HAINTMSK_TypeDef;

typedef union { /* offset 0x0440 */
  uint32_t d32;
  struct {
    uint32_t pcsts         :  1;
    uint32_t pcdet         :  1;
    uint32_t pena          :  1;
    uint32_t penchng       :  1;
    uint32_t poca          :  1;
    uint32_t pocchng       :  1;
    uint32_t pres          :  1;
    uint32_t psusp         :  1;
    uint32_t prst          :  1;
    uint32_t reserved9     :  1;
    uint32_t plsts         :  2;
    uint32_t ppwr          :  1;
    uint32_t ptctl         :  4;
    uint32_t pspd          :  2; /* the same coding as usb_speed_t */
    uint32_t reserved19_31 : 13;
  } b;
} USB_OTG_HPRT_TypeDef;

/*********************************************************************
* The USB_OTG_HC_REGS structure defines the relative field offsets for
* the host-mode channel control and status global registers.
* These registers are visible only in the host mode and must not be
* accessed in the device mode, as the results are unknown.
*********************************************************************/
typedef struct {
  volatile uint32_t HCCHARx;    /* 500 + 20 * ch_num, channel characteristics */
  volatile uint32_t HCSPLTx;    /* 504 + 20 * ch_num, channel split control */
  volatile uint32_t HCINTx;     /* 508 + 20 * ch_num, channel interrupt */
  volatile uint32_t HCINTMSKx;  /* 50C + 20 * ch_num, channel interrupt mask */
  volatile uint32_t HCTSIZx;    /* 510 + 20 * ch_num, transfer size */
  volatile uint32_t HCDMAx;     /* 514 + 20 * ch_num, channel DMA address */
  volatile uint32_t reserved18; /* 518 + 20 * ch_num, reserved */
  volatile uint32_t reserved1C; /* 51C + 20 * ch_num, reserved */
} USB_OTG_HCHNNLS;

#define P_USB_OTG_HCHNNLS \
  ((USB_OTG_HCHNNLS *)(_USB_OTG_BASE_ADDR + USB_OTG_HOST_CHANNEL_REG_OFFSET))

typedef union { /* offset 0x0500 + 0x0020 * ch_num */
  uint32_t d32;
  struct {
    uint32_t mpsiz      : 11;
    uint32_t epnum      :  4;
    uint32_t epdir      :  1;
    uint32_t reserved16 :  1;
    uint32_t lsdev      :  1;
    uint32_t eptyp      :  2;
    uint32_t mcnt       :  2; /* mc - do not change */
    uint32_t dad        :  7;
    uint32_t oddfrm     :  1;
    uint32_t chdis      :  1;
    uint32_t chena      :  1;
  } b;
} USB_OTG_HCCHARx_TypeDef;

typedef union { /* offset 0x0508 + 0x0020 * ch_num */
  uint32_t d32;
  struct {
    uint32_t xfrc          :  1;
    uint32_t chh           :  1;
    uint32_t ahberr        :  1;
    uint32_t stall         :  1;
    uint32_t nak           :  1;
    uint32_t ack           :  1;
    uint32_t nyet          :  1;
    uint32_t txerr         :  1;
    uint32_t bberr         :  1;
    uint32_t frmor         :  1;
    uint32_t dterr         :  1;
    uint32_t reserved11_31 : 21;
  } b;
} USB_OTG_HCINTx_TypeDef;

typedef union { /* offset 0x050C + 0x0020 * ch_num */
  uint32_t d32;
  struct {
    uint32_t xfrcm         :  1;
    uint32_t chhm          :  1;
    uint32_t ahberrm       :  1; /* ahberr - do not change */
    uint32_t stallm        :  1;
    uint32_t nakm          :  1;
    uint32_t ackm          :  1;
    uint32_t nyetm         :  1; /* nyet - do not change */
    uint32_t txerrm        :  1;
    uint32_t bberrm        :  1;
    uint32_t frmorm        :  1;
    uint32_t dterrm        :  1;
    uint32_t reserved11_31 : 21;
  } b;
} USB_OTG_HCINTMSKx_TypeDef;

typedef union { /* offset 0x0510 + 0x0020 * ch_num */
  uint32_t d32;
  struct {
    uint32_t xfrsiz      : 19;
    uint32_t pktcnt      : 10;
    uint32_t pid         :  2;
    uint32_t rerserved31 :  1;
  } b;
} USB_OTG_HCTSIZx_TypeDef;

#define HCTSIZ_PKTCNT_MAX  1023
#define HCTSIZ_DPID_DATA0  0
#define HCTSIZ_DPID_DATA2  1
#define HCTSIZ_DPID_DATA1  2
#define HCTSIZ_DPID_MDATA  3
#define HCTSIZ_DPID_SETUP  3

/*********************************************************************
* The USB_OTG_DEV structure defines the relative field offsets for the
* device mode control and status registers.
* These registers are visible only in the device mode and must not be
* accessed in the host mode, as the results are unknown.
*********************************************************************/
typedef struct {
  volatile uint32_t DCFG;         /* 800 configuration */
  volatile uint32_t DCTL;         /* 804 control */
  volatile uint32_t DSTS;         /* 808 status */
  volatile uint32_t reserved0C;   /* 80C reserved */
  volatile uint32_t DIEPMSK;      /* 810 IN endpoint interrupt mask */
  volatile uint32_t DOEPMSK;      /* 814 OUT endpoint interrupt mask */
  volatile uint32_t DAINT;        /* 818 all endpoints interrupt */
  volatile uint32_t DAINTMSK;     /* 81C all endpoints interrupt mask */
  volatile uint32_t reserved20;   /* 820 reserved */
  volatile uint32_t reserved24;   /* 824 reserved */
  volatile uint32_t DVBUSDIS;     /* 828 VBUS discharge time */
  volatile uint32_t DVBUSPULSE;   /* 82C VBUS pulsing time */
  volatile uint32_t DTHRCTL;      /* 830 threshold control */
  volatile uint32_t DIEPEMPMSK;   /* 834 IN endpoint FIFO empty interrupt mask */
  volatile uint32_t DEACHINT;     /* 838 each endpoint interrupt */
  volatile uint32_t DEACHINTMSK;  /* 83C each endpoint interrupt mask */
  volatile uint32_t DIEPEACHMSK1; /* 840 each endpoint 1 interrupt */
} USB_OTG_DREGS;

#define P_USB_OTG_DREGS \
  ((USB_OTG_DREGS *)(_USB_OTG_BASE_ADDR + USB_OTG_DEV_GLOBAL_REG_OFFSET))

typedef union { /* offset 0x0800 */
  uint32_t d32;
  struct {
    uint32_t dspd          :  2; /* the same coding as usb_speed_t */
    uint32_t nzlsohsk      :  1;
    uint32_t reserved3     :  1;
    uint32_t dad           :  7;
    uint32_t pfivl         :  2;
    uint32_t reserved13_13 : 11;
    uint32_t perschivl     :  2;
    uint32_t reserved26_31 :  6;
  } b;
} USB_OTG_DCFG_TypeDef;

typedef union { /* offset 0x0804 */
  uint32_t d32;
  struct {
    uint32_t rwusig        :  1;
    uint32_t sdis          :  1;
    uint32_t ginsts        :  1;
    uint32_t gonsts        :  1;
    uint32_t tctl          :  3;
    uint32_t sginak        :  1;
    uint32_t cginak        :  1;
    uint32_t sgonak        :  1;
    uint32_t cgonak        :  1;
    uint32_t poprgdne      :  1;
    uint32_t reserved12_31 : 20;
  } b;
} USB_OTG_DCTL_TypeDef;

typedef union { /* offset 0x0808 */
  uint32_t d32;
  struct {
    uint32_t suspsts       :  1;
    uint32_t enumspd       :  2; /* the same coding as usb_speed_t */
    uint32_t eerr          :  1;
    uint32_t reserved4_7   :  4;
    uint32_t fnsof         : 14;
    uint32_t reserved22_31 : 10;
  } b;
} USB_OTG_DSTS_TypeDef;

typedef union { /* offset 0x0810 */
  uint32_t d32;
  struct {
    uint32_t xfrcm         :  1;
    uint32_t epdm          :  1;
    uint32_t reserved2     :  1;
    uint32_t tom           :  1;
    uint32_t ittxfemsk     :  1;
    uint32_t inepnmm       :  1;
    uint32_t inepnem       :  1;
    uint32_t reserved7     :  1;
    uint32_t txfurm        :  1;
    uint32_t bim           :  1;
    uint32_t reserved10_31 : 22;
  } b;
} USB_OTG_DIEPMSK_TypeDef;

typedef union { /* offset 0x0814 */
  uint32_t d32;
  struct {
    uint32_t xfrcm         :  1;
    uint32_t epdm          :  1;
    uint32_t reserved2     :  1;
    uint32_t stupm         :  1;
    uint32_t otepdm        :  1;
    uint32_t reserved5     :  1;
    uint32_t b2bstupm      :  1; /* b2bstup -do not change */
    uint32_t reserved7     :  1;
    uint32_t opem          :  1;
    uint32_t boim          :  1;
    uint32_t reserved10_31 : 22;
  } b;
} USB_OTG_DOEPMSK_TypeDef;

typedef union { /* offset 0x0818 */
  uint32_t d32;
  struct {
    uint32_t iepint : 16;
    uint32_t oepint : 16;
  } b;
} USB_OTG_DAINT_TypeDef;

typedef union { /* offset 0x081C */
  uint32_t d32;
  struct {
    uint32_t iepm : 16;
    uint32_t oepm : 16;
  } b;
} USB_OTG_DAINTMSK_TypeDef;

/*********************************************************************
* The USB_OTG_DINEPS structure defines one set of the IN endpoint
* specific registers per implemented logical endpoint.
* These registers are visible only in the device mode and must not be
* accessed in the host mode, as the results are unknown.
*********************************************************************/
typedef struct {
  volatile uint32_t DIEPCTLx;   /* 900 + 20 * ep_num, IN EP control */
  volatile uint32_t reserved04; /* 904 + 20 * ep_num, reserved */
  volatile uint32_t DIEPINTx;   /* 908 + 20 * ep_num, IN EP interrupt */
  volatile uint32_t reserved0C; /* 90C + 20 * ep_num, reserved */
  volatile uint32_t DIEPTSIZx;  /* 910 + 20 * ep_num, IN EP FIFO size */
  volatile uint32_t DIEPDMAx;   /* 914 + 20 * ep_num, DMA address */
  volatile uint32_t DTXFSTSx;   /* 918 + 20 * ep_num, IN EP FIFO status */
  volatile uint32_t reserved1C; /* 91C + 20 * ep_num, reserved */
} USB_OTG_DINEPS;

#define P_USB_OTG_DINEPS \
  ((USB_OTG_DINEPS *)(_USB_OTG_BASE_ADDR + USB_OTG_DEV_IN_EP_REG_OFFSET))

/*********************************************************************
* The USB_OTG_DOUTEPS structure defines one set of the OUT endpoint
* specific registers per implemented logical endpoint.
* These registers are visible only in the device mode and must not be
* accessed in the host mode, as the results are unknown.
*********************************************************************/
typedef struct {
  volatile uint32_t DOEPCTLx;   /* B00 + 20 * ep_num, OUT EP control */
  volatile uint32_t reserved04; /* B04 + 20 * ep_num, reserved */
  volatile uint32_t DOEPINTx;   /* B08 + 20 * ep_num, OUT EP interrupt */
  volatile uint32_t reserved0C; /* B0C + 20 * ep_num, reserved */
  volatile uint32_t DOEPTSIZx;  /* B10 + 20 * ep_num, OUT EP FIFO size */
  volatile uint32_t DOEPDMAx;   /* B14 + 20 * ep_num, DMA address */
  volatile uint32_t reserved18; /* B18 + 20 * ep_num, reserved */
  volatile uint32_t reserved1C; /* B1C + 20 * ep_num, reserved */
} USB_OTG_DOUTEPS;

#define P_USB_OTG_DOUTEPS \
  ((USB_OTG_DOUTEPS *)(_USB_OTG_BASE_ADDR + USB_OTG_DEV_OUT_EP_REG_OFFSET))

/* DIEPCTL0 offset 0x0900
   DIEPCTLx offset 0x0900 + 0x20 * ep_num
   DOEPCTL0 offset 0x0B00
   DOEPCTLx offset 0x0B00 + 0x20 * ep_num */
typedef union {
  uint32_t d32;
  struct {
    uint32_t mpsiz          : 11;
    uint32_t reserved11_14  :  4;
    uint32_t usbaep         :  1;
    uint32_t dpid_eonum     :  1;
    uint32_t naksts         :  1;
    uint32_t eptyp          :  2;
    uint32_t snpm           :  1;
    uint32_t stall          :  1;
    uint32_t txfnum         :  4;
    uint32_t cnak           :  1;
    uint32_t snak           :  1;
    uint32_t sd0pid_sevnfrm :  1;
    uint32_t sd1pid_soddfrm :  1;
    uint32_t epdis          :  1;
    uint32_t epena          :  1;
  } b;
} USB_OTG_DEPCTLx_TypeDef;

#define MPSIZ_64  0
#define MPSIZ_32  1
#define MPSIZ_16  2
#define MPSIZ_8   3

typedef union { /* offset 0x0908 + 0x20 * ep_num */
  uint32_t d32;
  struct {
    uint32_t xfrc          :  1;
    uint32_t epdisd        :  1;
    uint32_t reserved2     :  1;
    uint32_t toc           :  1;
    uint32_t inixfe        :  1;
    uint32_t reserved5     :  1;
    uint32_t inepne        :  1;
    uint32_t txfe          :  1;
    uint32_t txfifoudrn    :  1;
    uint32_t bna           :  1;
    uint32_t reserved10    :  1;
    uint32_t pktdrpsts     :  1;
    uint32_t berr          :  1;
    uint32_t nak           :  1;
    uint32_t reserved14_31 : 18;
  } b;
} USB_OTG_DIEPINTx_TypeDef;

typedef union { /* offset 0x0B08 + 0x20 * ep_num */
  uint32_t d32;
  struct {
    uint32_t xfrc          :  1;
    uint32_t epdisd        :  1;
    uint32_t reserved2     :  1;
    uint32_t stup          :  1;
    uint32_t otepdis       :  1;
    uint32_t reserved5     :  1;
    uint32_t b2bstup       :  1;
    uint32_t reserved7_13  :  7;
    uint32_t nyet          :  1;
    uint32_t reserved15_31 : 17;
  } b;
} USB_OTG_DOEPINTx_TypeDef;

/* DIEPTSIZ0 offset 0x0910
   DOEPTSIZ0 offset 0x0B10
   DIEPTSIZx offset 0x0910 + 0x20 * ep_num
   DOEPTSIZx offset 0x0B10 + 0x20 * ep_num */
typedef union {
  uint32_t d32;
  struct {
    uint32_t xfrsiz             : 19;
    uint32_t pktcnt             : 10;
    uint32_t stupcnt_mcnt_rxpid :  2;
    uint32_t reserved31         :  1;
  } b;
} USB_OTG_DEPTSIZx_TypeDef;

typedef union { /* offset 0x0918 + 0x20 * ep_num */
  uint32_t d32;
  struct {
    uint32_t ineptfsav     : 16;
    uint32_t reserved16_31 : 16;
  } b;
} USB_OTG_DTXFSTSx_TypeDef;

/*********************************************************************
* The USB_OTG_PREGS structure defines the relative field offsets for
* the power and clock gate control registers.
*********************************************************************/
typedef struct {
  volatile uint32_t PCGCCTL; /* E00 power and clock gate control */
} USB_OTG_PREGS;

#define P_USB_OTG_PREGS \
  ((USB_OTG_PREGS *)(_USB_OTG_BASE_ADDR + USB_OTG_PCGCCTL_OFFSET))

typedef union { /* offset 0x0E00 */
  uint32_t d32;
  struct {
    uint32_t stppclk      :  1;
    uint32_t gatehclk     :  1;
    uint32_t reserved2_3  :  2;
    uint32_t physusp      :  1;
    uint32_t reserved5_31 : 27;
  } b;
} USB_OTG_PCGCCTL_TypeDef;


/*********************************************************************
* The USB_OTG_FIFO structure defines the FIFO registers.
*********************************************************************/
typedef volatile uint32_t USB_OTG_FIFO[USB_OTG_DATA_FIFO_SIZE >> 2];

#define P_USB_OTG_FIFO \
  ((USB_OTG_FIFO *)(_USB_OTG_BASE_ADDR + USB_OTG_DATA_FIFO_OFFSET))

#endif /* STM32F10X_CL || STM32F2XX || STM32F4XX */

#ifdef __cplusplus
}
#endif

#endif /* _USB_OTG_REGS_H */
