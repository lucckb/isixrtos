/*
 * ssd1306_cmds.hpp
 *
 *  Created on: 26 gru 2017
 *      Author: lucck
 */

#pragma once

namespace fnd {
namespace drv {
namespace lcd {


namespace {
namespace ns1306 {
namespace cmd {
//! Command description
//!
	constexpr uint8_t ADDR_MODE				=	0x20;
	constexpr uint8_t SET_COLUMN_ADDR		=	0x21;
	constexpr uint8_t SET_PAGE_ADDR			=	0x22;
	constexpr uint8_t SET_START_LINE		=	0x40;
	constexpr uint8_t SET_CONTRAST			=	0x81;
	constexpr uint8_t SET_CHARGEPUMP		=	0x8D;
	constexpr uint8_t SET_SEGMENT_REMAP		=	0xA1;
	constexpr uint8_t DISPLAY_ALL_ON_RES	=	0xA4;
	constexpr uint8_t NORMAL				=	0xA6;
	constexpr uint8_t SET_MUX				=	0xA8;
	constexpr uint8_t DISPLAY_OFF			=	0xAE;
	constexpr uint8_t DISPLAY_ON			=	0xAF;
	constexpr uint8_t SET_COM_SCAN_INC		=	0xC0;
	constexpr uint8_t SET_COM_SCAN_DEC		=	0xC8;
	constexpr uint8_t SET_OFFSET			=	0xD3;
	constexpr uint8_t SET_CLK_DIV			=	0xD5;
	constexpr uint8_t SET_PRECHARGE			=	0xD9;
	constexpr uint8_t SET_COM_PINS			=	0xDA;
	constexpr uint8_t SET_VCOM_DESELECT		=	0xDB;
	constexpr uint8_t PAGE_START_ADDR		=	0xB0;
	constexpr uint8_t COLUMN_LOW_ADDR		=	0x00;
	constexpr uint8_t COLUMN_HIGH_ADDR		=	0x10;
	constexpr uint8_t MODE_HORIZ			=	0;
}
namespace addr {
	constexpr uint8_t MODE_VERT			=	1;
	constexpr uint8_t MODE_PAGE			=	2;
}

}}



}}}
