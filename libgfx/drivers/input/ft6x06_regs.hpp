/*
 * =====================================================================================
 *
 *       Filename:  ft6x06_regs.hpp
 *
 *    Description:  Touchscreen registers defs
 *
 *        Version:  1.0
 *        Created:  2019.09.03 21:53:03
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Lucjan Bryndza (LB), lbryndza.p@boff.pl
 *   Organization:  BoFF
 *
 * =====================================================================================
 */
#pragma once

namespace gfx::drv::detail::regs::ft6x06 {


  /* Current mode register of the FT6206 (R/W) */
static constexpr auto DEV_MODE_REG = 0x00;

  /* Possible values of DEV_MODE_REG */
static constexpr auto DEV_MODE_WORKING = 0x00;
static constexpr auto DEV_MODE_FACTORY = 0x04;

static constexpr auto DEV_MODE_MASK = 0x7;
static constexpr auto DEV_MODE_SHIFT = 4;

  /* Gesture ID register */
static constexpr auto GEST_ID_REG = 0x01;

  /* Possible values of GEST_ID_REG */
static constexpr auto GEST_ID_NO_GESTURE = 0x00;
static constexpr auto GEST_ID_MOVE_UP = 0x10;
static constexpr auto GEST_ID_MOVE_RIGHT = 0x14;
static constexpr auto GEST_ID_MOVE_DOWN = 0x18;
static constexpr auto GEST_ID_MOVE_LEFT = 0x1C;
static constexpr auto GEST_ID_ZOOM_IN = 0x48;
static constexpr auto GEST_ID_ZOOM_OUT = 0x49;

  /* Touch Data Status register : gives number of active touch points (0..2) */
static constexpr auto TD_STAT_REG = 0x02;

  /* Values related to TD_STAT_REG */
static constexpr auto TD_STAT_MASK = 0x0F;
static constexpr auto TD_STAT_SHIFT = 0x00;

  /* Values Pn_XH and Pn_YH related */
static constexpr auto TOUCH_EVT_FLAG_PRESS_DOWN = 0x00;
static constexpr auto TOUCH_EVT_FLAG_LIFT_UP = 0x01;
static constexpr auto TOUCH_EVT_FLAG_CONTACT = 0x02;
static constexpr auto TOUCH_EVT_FLAG_NO_EVENT = 0x03;

static constexpr auto TOUCH_EVT_FLAG_SHIFT = 6;
static constexpr auto TOUCH_EVT_FLAG_MASK = (3 << TOUCH_EVT_FLAG_SHIFT);

static constexpr auto MSB_MASK = 0x0F;
static constexpr auto MSB_SHIFT = 0;

  /* Values Pn_XL and Pn_YL related */
static constexpr auto LSB_MASK = 0xFF;
static constexpr auto LSB_SHIFT = 0;

static constexpr auto P1_XH_REG = 0x03;
static constexpr auto P1_XL_REG = 0x04;
static constexpr auto P1_YH_REG = 0x05;
static constexpr auto P1_YL_REG = 0x06;

  /* Touch Pressure register value (R) */
static constexpr auto P1_WEIGHT_REG = 0x07;

  /* Values Pn_WEIGHT related  */
static constexpr auto TOUCH_WEIGHT_MASK = 0xFF;
static constexpr auto TOUCH_WEIGHT_SHIFT = 0;

  /* Touch area register */
static constexpr auto P1_MISC_REG = 0x08;

  /* Values related to Pn_MISC_REG */
static constexpr auto TOUCH_AREA_MASK = (0x04 << 4);
static constexpr auto TOUCH_AREA_SHIFT = 0x04;

static constexpr auto P2_XH_REG = 0x09;
static constexpr auto P2_XL_REG = 0x0A;
static constexpr auto P2_YH_REG = 0x0B;
static constexpr auto P2_YL_REG = 0x0C;
static constexpr auto P2_WEIGHT_REG = 0x0D;
static constexpr auto P2_MISC_REG = 0x0E;

  /* Threshold for touch detection */
static constexpr auto TH_GROUP_REG = 0x80;

  /* Values TH_GROUP_REG : threshold related  */
static constexpr auto THRESHOLD_MASK = 0xFF;
static constexpr auto THRESHOLD_SHIFT = 0;

  /* Filter function coefficients */
static constexpr auto TH_DIFF_REG = 0x85;

  /* Control register */
static constexpr auto CTRL_REG = 0x86;

  /* Values related to CTRL_REG */

  /* Will keep the Active mode when there is no touching */
static constexpr auto CTRL_KEEP_ACTIVE_MODE = 0x00;

  /* Switching from Active mode to Monitor mode automatically when there is no touching */
static constexpr auto CTRL_KEEP_AUTO_SWITCH_MONITOR_MODE = 0x01;

  /* The time period of switching from Active mode to Monitor mode when there is no touching */
static constexpr auto TIMEENTERMONITOR_REG = 0x87;

  /* Report rate in Active mode */
static constexpr auto PERIODACTIVE_REG = 0x88;

  /* Report rate in Monitor mode */
static constexpr auto PERIODMONITOR_REG = 0x89;

  /* The value of the minimum allowed angle while Rotating gesture mode */
static constexpr auto RADIAN_VALUE_REG = 0x91;

  /* Maximum offset while Moving Left and Moving Right gesture */
static constexpr auto OFFSET_LEFT_RIGHT_REG = 0x92;

  /* Maximum offset while Moving Up and Moving Down gesture */
static constexpr auto OFFSET_UP_DOWN_REG = 0x93;

  /* Minimum distance while Moving Left and Moving Right gesture */
static constexpr auto DISTANCE_LEFT_RIGHT_REG = 0x94;

  /* Minimum distance while Moving Up and Moving Down gesture */
static constexpr auto DISTANCE_UP_DOWN_REG = 0x95;

  /* Maximum distance while Zoom In and Zoom Out gesture */
static constexpr auto DISTANCE_ZOOM_REG = 0x96;

  /* High 8-bit of LIB Version info */
static constexpr auto LIB_VER_H_REG = 0xA1;

  /* Low 8-bit of LIB Version info */
static constexpr auto LIB_VER_L_REG = 0xA2;

  /* Chip Selecting */
static constexpr auto CIPHER_REG = 0xA3;

  /* Interrupt mode register (used when in interrupt mode) */
static constexpr auto GMODE_REG = 0xA4;

static constexpr auto G_MODE_INTERRUPT_MASK = 0x03;
static constexpr auto G_MODE_INTERRUPT_SHIFT = 0x00;

  /* Possible values of GMODE_REG */
static constexpr auto G_MODE_INTERRUPT_POLLING = 0x00;
static constexpr auto G_MODE_INTERRUPT_TRIGGER = 0x01;

  /* Current power mode the FT6206 system is in (R) */
static constexpr auto PWR_MODE_REG = 0xA5;

  /* FT6206 firmware version */
static constexpr auto FIRMID_REG = 0xA6;

  /* FT6206 Chip identification register */
static constexpr auto CHIP_ID_REG = 0xA8;

  /*  Possible values of CHIP_ID_REG */
static constexpr auto ID_VALUE = 0x11;
static constexpr auto FT6x36_ID_VALUE = 0xCD;

  /* Release code version */
static constexpr auto RELEASE_CODE_ID_REG = 0xAF;

  /* Current operating mode the FT6206 system is in (R) */
static constexpr auto STATE_REG = 0xBC;


static constexpr auto FT6206_DEV_MODE_WORKING   = 0;
static constexpr auto FT6206_DEV_MODE_FACTORY   = 0x04;
static constexpr auto FT6206_DEV_MODE_MASK      = 0x7;
static constexpr auto FT6206_DEV_MODE_SHIFT     = 4;
static constexpr auto FT6206_DEV_MODE_REG       = 0x00;
/* Touch Data Status register : gives number of active touch points (0..2) */
static constexpr auto FT6206_TD_STAT_REG             = 0x02;

  /* Interrupt mode register (used when in interrupt mode) */
static constexpr auto FT6206_GMODE_REG                = 0xA4;

static constexpr auto FT6206_G_MODE_INTERRUPT_MASK    = 0x03;
static constexpr auto FT6206_G_MODE_INTERRUPT_SHIFT   = 0x00;

  /* Possible values of FT6206_GMODE_REG */
static constexpr auto FT6206_G_MODE_INTERRUPT_POLLING = 0x00;
static constexpr auto FT6206_G_MODE_INTERRUPT_TRIGGER = 0x01;


/* Gesture ID register */
static constexpr auto FT6206_GEST_ID_REG             = 0x01;

  /* Possible values of FT6206_GEST_ID_REG */
static constexpr auto FT6206_GEST_ID_NO_GESTURE = 0x00; 
static constexpr auto FT6206_GEST_ID_MOVE_UP = 0x10; 
static constexpr auto FT6206_GEST_ID_MOVE_RIGHT = 0x14; 
static constexpr auto FT6206_GEST_ID_MOVE_DOWN = 0x18; 
static constexpr auto FT6206_GEST_ID_MOVE_LEFT = 0x1C; 
static constexpr auto FT6206_GEST_ID_ZOOM_IN = 0x48; 
static constexpr auto FT6206_GEST_ID_ZOOM_OUT = 0x49;


 /* Values related to FT6206_TD_STAT_REG */
static constexpr auto FT6206_TD_STAT_MASK = 0x0F; 
static constexpr auto FT6206_TD_STAT_SHIFT = 0x00; 

  /* Values Pn_XL and Pn_YL related */
static constexpr auto FT6206_LSB_MASK = 0xFF; 
static constexpr auto FT6206_LSB_SHIFT = 0; 

static constexpr auto FT6206_P1_XH_REG = 0x03; 
static constexpr auto FT6206_P1_XL_REG = 0x04; 
static constexpr auto FT6206_P1_YH_REG = 0x05; 
static constexpr auto FT6206_P1_YL_REG = 0x06; 

  /* Values related to FT6206_Pn_MISC_REG */
static constexpr auto FT6206_TOUCH_AREA_MASK = (0x04 << 4); 
static constexpr auto FT6206_TOUCH_AREA_SHIFT = 0x04; 

static constexpr auto FT6206_P2_XH_REG = 0x09; 
static constexpr auto FT6206_P2_XL_REG = 0x0A; 
static constexpr auto FT6206_P2_YH_REG = 0x0B; 
static constexpr auto FT6206_P2_YL_REG = 0x0C; 
static constexpr auto FT6206_P2_WEIGHT_REG = 0x0D; 
static constexpr auto FT6206_P2_MISC_REG = 0x0E; 
static constexpr auto FT6206_MSB_MASK = 0x0F; 
static constexpr auto FT6206_MSB_SHIFT = 0; 
  /* Maximum border values of the touchscreen pad */
static constexpr uint16_t FT_6206_MAX_WIDTH = 800; /* Touchscreen pad max width   */
static constexpr uint16_t FT_6206_MAX_HEIGHT = 480; /* Touchscreen pad max height  */
  /* Values Pn_XH and Pn_YH related */
static constexpr auto FT6206_TOUCH_EVT_FLAG_PRESS_DOWN = 0x00; 
static constexpr auto FT6206_TOUCH_EVT_FLAG_LIFT_UP = 0x01; 
static constexpr auto FT6206_TOUCH_EVT_FLAG_CONTACT = 0x02; 
static constexpr auto FT6206_TOUCH_EVT_FLAG_NO_EVENT = 0x03; 
/* Touch Pressure register value (R) */
static constexpr auto FT6206_P1_WEIGHT_REG = 0x07; 
  /* Values Pn_WEIGHT related  */
static constexpr auto FT6206_TOUCH_WEIGHT_MASK = 0xFF; 
static constexpr auto FT6206_TOUCH_WEIGHT_SHIFT = 0; 
 /* Values Pn_XH and Pn_YH related */

static constexpr auto FT6206_TOUCH_EVT_FLAG_SHIFT = 6; 
static constexpr auto FT6206_TOUCH_EVT_FLAG_MASK = (3 << FT6206_TOUCH_EVT_FLAG_SHIFT); 
}
