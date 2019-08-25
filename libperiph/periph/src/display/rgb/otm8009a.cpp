/**
 * =====================================================================================
 * 	File: otm8009a.cpp
 * 	Created Date: Sunday, August 18th 2019, 8:59:26 pm
 * 	Author: Lucjan Bryndza
 * 	Copyright (c) 2019 BoFF
 * 
 * 	GPL v2/3
 * =====================================================================================
 */

#include <periph/drivers/display/rgb/otm8009a.hpp>
#include <periph/drivers/display/bus/dsi.hpp>
#include <periph/gpio/gpio.hpp>
#include <isix/ostime.h>
#include <periph/dt/dts.hpp>
#include <foundation/sys/dbglog.h>
#include "otm8009a_regs.hpp"

namespace periph::display {

//! Constructor
otm8009a::otm8009a(bus::ibus& dsi, const char name[])
    : idisplay(dsi,name), m_dev_name(name)
{
}


//! Open device
int otm8009a::open() noexcept
{
	int ret {};
	do {
		using namespace detail;
		// Configure gpio
		ret = gpio_conf(true);
		if(ret) break;
		// Open bus driver
		ret = bus().open();
		if(ret) break;
		/* Enter CMD2 */
		ret = write_cmd( MCS_CMD2_ENA1, 0x80, 0x09, 0x01);
		if(ret) break;

		/* Enter ORISE CMD2 */
		ret = write_cmd( MCS_CMD2_ENA2, 0x80, 0x09);
		if(ret) break;

		ret = write_cmd( MCS_SD_PCH_CTRL, 0x30);
		if(ret) break;
		isix::wait_ms(10);

		ret = write_cmd( MCS_NO_DOC1, 0x40);
		if(ret) break;
		isix::wait_ms(10);

		ret = write_cmd( MCS_PWR_CTRL4 + 1, 0xA9);
		if(ret) break;
		ret = write_cmd( MCS_PWR_CTRL2 + 1, 0x34);
		if(ret) break;
		ret = write_cmd( MCS_P_DRV_M, 0x50);
		if(ret) break;
		ret = write_cmd( MCS_VCOMDC, 0x4E);
		if(ret) break;
		ret = write_cmd( MCS_OSC_ADJ, 0x66); /* 65Hz */
		if(ret) break;
		ret = write_cmd( MCS_PWR_CTRL2 + 2, 0x01);
		if(ret) break;
		ret = write_cmd( MCS_PWR_CTRL2 + 5, 0x34);
		if(ret) break;
		ret = write_cmd( MCS_PWR_CTRL2 + 4, 0x33);
		if(ret) break;
		ret = write_cmd( MCS_GVDDSET, 0x79, 0x79);
		if(ret) break;
		ret = write_cmd( MCS_SD_CTRL + 1, 0x1B);
		if(ret) break;
		ret = write_cmd( MCS_PWR_CTRL1 + 2, 0x83);
		if(ret) break;
		ret = write_cmd( MCS_SD_PCH_CTRL + 1, 0x83);
		if(ret) break;
		ret = write_cmd( MCS_RGB_VIDEO_SET, 0x0E);
		if(ret) break;
		ret = write_cmd( MCS_PANSET, 0x00, 0x01);
		if(ret) break;

		ret = write_cmd( MCS_GOAVST, 0x85, 0x01, 0x00, 0x84, 0x01, 0x00);
		if(ret) break;
		ret = write_cmd( MCS_GOACLKA1, 0x18, 0x04, 0x03, 0x39, 0x00, 0x00,
			0x00, 0x18, 0x03, 0x03, 0x3A, 0x00, 0x00, 0x00);
		if(ret) break;
		ret = write_cmd( MCS_GOACLKA3, 0x18, 0x02, 0x03, 0x3B, 0x00, 0x00,
			0x00, 0x18, 0x01, 0x03, 0x3C, 0x00, 0x00, 0x00);
		if(ret) break;
		ret = write_cmd( MCS_GOAECLK, 0x01, 0x01, 0x20, 0x20, 0x00, 0x00,
			0x01, 0x02, 0x00, 0x00);
		if(ret) break;

		ret = write_cmd( MCS_NO_DOC2, 0x00);
		if(ret) break;

		ret = write_cmd( MCS_PANCTRLSET1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
		if(ret) break;
		ret = write_cmd( MCS_PANCTRLSET2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0);
		if(ret) break;
		ret = write_cmd( MCS_PANCTRLSET3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0);
		if(ret) break;
		ret = write_cmd( MCS_PANCTRLSET4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
		if(ret) break;
		ret = write_cmd( MCS_PANCTRLSET5, 0, 4, 4, 4, 4, 4, 0, 0, 0, 0,
			0, 0, 0, 0, 0);
		if(ret) break;
		ret = write_cmd( MCS_PANCTRLSET6, 0, 0, 0, 0, 0, 0, 4, 4, 4, 4,
			4, 0, 0, 0, 0);
		if(ret) break;
		ret = write_cmd( MCS_PANCTRLSET7, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
		if(ret) break;
		ret = write_cmd( MCS_PANCTRLSET8, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
			0xFF, 0xFF, 0xFF, 0xFF, 0xFF);
		if(ret) break;

		ret = write_cmd( MCS_PANU2D1, 0x00, 0x26, 0x09, 0x0B, 0x01, 0x25,
			0x00, 0x00, 0x00, 0x00);
		if(ret) break;

		ret = write_cmd( MCS_PANU2D2, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x26, 0x0A, 0x0C, 0x02);
		if(ret) break;
		ret = write_cmd( MCS_PANU2D3, 0x25, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
		if(ret) break;
		ret = write_cmd( MCS_PAND2U1, 0x00, 0x25, 0x0C, 0x0A, 0x02, 0x26,
			0x00, 0x00, 0x00, 0x00);
		if(ret) break;
		ret = write_cmd( MCS_PAND2U2, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x25, 0x0B, 0x09, 0x01);
		if(ret) break;
		ret = write_cmd( MCS_PAND2U3, 0x26, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
		if(ret) break;

		ret = write_cmd( MCS_PWR_CTRL1 + 1, 0x66);
		if(ret) break;

		ret = write_cmd( MCS_NO_DOC3, 0x06);
		if(ret) break;

		ret = write_cmd( MCS_GMCT22P, 0x00, 0x09, 0x0F, 0x0E, 0x07, 0x10,
			0x0B, 0x0A, 0x04, 0x07, 0x0B, 0x08, 0x0F, 0x10, 0x0A, 0x01);
		if(ret) break;
		ret = write_cmd( MCS_GMCT22N, 0x00, 0x09, 0x0F, 0x0E, 0x07, 0x10,
			0x0B, 0x0A, 0x04, 0x07, 0x0B, 0x08, 0x0F, 0x10, 0x0A, 0x01);
		if(ret) break;

		/* Exit CMD2 */
		ret = write_cmd( MCS_CMD2_ENA1, 0xFF, 0xFF, 0xFF);
		if(ret) break;

		ret = write_seq(OTM_CMD_NOP, 0x00);
		if(ret) break;

		ret = write_seq(OTM_CMD_SLPOUT);
		if(ret) break;
	
		/* Wait for sleep out */
		isix::wait_ms(120);
		switch (bpp()) {
		case 24:
		case 32:
			ret = write_seq(OTM_CMD_COLMOD, (COLMOD_VIPF_24_1 | COLMOD_IFPF_24));
			break;
		case 16:
			ret = write_seq(OTM_CMD_COLMOD, (COLMOD_VIPF_16 | COLMOD_IFPF_16));
		default:
			break;
		}
		if(ret) break;
		
		/* Content Adaptive Backlight Control */
		ret = write_seq(OTM_CMD_WRDISBV, 0x7F);
		if(ret) break;
		ret = write_seq(OTM_CMD_WRCTRLD, 0x2C);
		if(ret) break;
		ret = write_seq(OTM_CMD_WRCABC, 0x02);
		if(ret) break;
		ret = write_seq(OTM_CMD_WRCABCMB, 0xFF);
		if(ret) break;

		ret = write_seq(OTM_CMD_DISPON, 0x00);
		if(ret) break;
		ret = write_seq(OTM_CMD_NOP, 0x00);
		if(ret) break;
		ret = write_seq(OTM_CMD_RAMWR, 0x00);
		if(ret) break;
	} while(0);
	return ret;
}


//! Change display orientation
int otm8009a::orientation(orientation_t orient) noexcept
{
	using namespace detail;
	if(orient==orientation_t::landscape) {
		auto ret = write_seq(OTM_CMD_MADCTL, (MADCTL_MV | MADCTL_MX));
		if(ret) return ret;
		ret = write_seq(OTM_CMD_CASET, 0x00, 0x00, 0x03, 0x1F);
		if(ret) return ret;
		ret = write_seq(OTM_CMD_PASET, 0x00, 0x00, 0x01, 0xDF);
		if(ret) return ret;
	} else if(orient==orientation_t::portrait) {
		auto ret = write_seq(OTM_CMD_MADCTL, 0);
		if(ret) return ret;
		ret = write_seq(OTM_CMD_CASET, 0x00, 0x00, 0x01, 0xDF);
		if(ret) return ret;
		ret = write_seq(OTM_CMD_PASET, 0x00, 0x00, 0x03, 0x1F);
		if(ret) return ret;
	}
	return error::success;
}

//! Close device
int otm8009a::close() noexcept
{
	const auto r1 = write_cmd(detail::OTM_CMD_SWRESET, 0);
	const auto r2 = bus().close();
	gpio_conf(false);
	return r1?r1:r2;
}



// Configure GPIO
int otm8009a::gpio_conf(bool en) noexcept
{
	//Configure and reset display first
	int pin = dt::get_periph_pin(m_dev_name,dt::pinfunc::lcd_reset);
	if(pin<0) return pin;
	dbg_info("otm reset pin %i",pin);
	if(en) {
		gpio::setup(pin, gpio::mode::out{gpio::outtype::pushpull,gpio::speed::medium} );
		gpio::set(pin, false);
		isix::wait_ms(10);
		gpio::set(pin, true);
	} else {
		gpio::setup(pin, gpio::mode::in{gpio::pulltype::floating});
	}
	pin = dt::get_periph_pin(m_dev_name,dt::pinfunc::lcd_backlight);
	if(pin<0) return pin;
	dbg_info("otm backligh pin %i",pin);
	if(en) {
		gpio::setup(pin, gpio::mode::out{gpio::outtype::pushpull,gpio::speed::medium} );
		m_blpin = pin;
	} else {
		gpio::setup(pin, gpio::mode::in{gpio::pulltype::floating});
		m_blpin = -1;
	}
	return error::success;
}

//! Setup backlight mode
void otm8009a::backlight( int percent ) noexcept
{
	if(m_blpin>=0) {
		gpio::set(m_blpin,percent);
	} else {
		dbg_warn("Backlight gpio not configured");
	}
}

}