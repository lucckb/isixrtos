/**
 * =====================================================================================
 * 	File: dsi.cpp
 * 	Created Date: Saturday, August 17th 2019, 8:56:21 pm
 * 	Author: Lucjan Bryndza
 * 	Copyright (c) 2019 BoFF
 * 
 * 	GPL v2/3
 * =====================================================================================
 */

#include <periph/drivers/display/bus/dsi.hpp>
#include <periph/core/error.hpp>
#include <periph/dt/dts.hpp>
#include <periph/clock/clocks.hpp>
#include <periph/gpio/gpio.hpp>
#include <periph/drivers/display/rgb/display_config.hpp>
#include <foundation/sys/dbglog.h>
#include <stm32_ll_rcc.h>
#include <isix/ostime.h>
#undef DSI_VR
#include "dsi_regs.hpp"

#ifdef DSI
namespace periph::display::bus {


//! Default constructor
dsi::dsi(const char dsi_name[])
    : ibus(ibus::type::dsi)
	, m_base(dt::get_periph_base_address(dsi_name))
{
    if(!m_base) {
        error::expose<error::bus_exception>(error::invstate);
    }
    const dt::device_conf_base* base;
    int ret = dt::get_periph_devconf(dsi_name,base);
    error::expose<error::bus_exception>(ret);
    const auto entry = reinterpret_cast<const fb_info*>(base);
    if(entry->n_layers != 1) {
        error::expose<error::bus_exception>(error::inval);
    }
    m_fbinfo = entry->layers;
}

/** Open DSI device using default display name */
int dsi::open()
{
    int ret {};
    do {    
        //Get periph clock config
        dt::clk_periph pclk;
        ret = dt::get_periph_clock(io(), pclk); 
        if(ret) break;
        //Enable device clock
        ret = clock::device_enable(pclk); 
        if(ret) break;
        // Configure GPIOS
        ret = gpio_conf(true); 
        if(ret) break;
        //Configure controller
        hardware_setup();
    } while(0);
    return ret;
}

/** Close already opened DSI device */
int dsi::close()
{
    int ret {};
    do {    
        //Get periph clock config
        dt::clk_periph pclk;
        ret = dt::get_periph_clock(io(), pclk); 
        if(ret) break;
        //Reset device
        ret = clock::device_reset(pclk); 
        if(ret) break;
        //Disable device clock
        ret = clock::device_disable(pclk); 
        if(ret) break;
        // Configure GPIOS
        ret = gpio_conf(false); 
        if(ret) break;
    } while(0);
    return ret;
}

/** DSI short command operator */
int dsi::raw_write(vch_t vchid, data_t data_type, 
            param_t data0, param_t data1)
{
    using namespace detail;
    if(!m_base) return error::noinit;
	uint32_t reg;
	/* Wait FIFO empty flag */
	while ((ior(DSI_GPSR) & GPSR_CMDFE) != 1);
	reg = (uint32_t(data_type) << GHCR_DT_S);
	reg |= (uint32_t(vchid) << GHCR_VCID_S);
	reg |= (uint32_t(data0) << GHCR_WCLSB_S);
	reg |= (uint32_t(data1) << GHCR_WCMSB_S);
	iow(DSI_GHCR, reg);
    return error::success;
}
        
/** DSI long command operator */
int dsi::raw_write(vch_t vchid, datal_t data_type, 
        const data_t* params, size_t nparams)
{
    using namespace detail;
    if(!m_base) return error::noinit;
	uint32_t data0;
	uint32_t data1;
	uint32_t reg;
	unsigned i;
	/* Wait FIFO empty flag */
	while ((ior(DSI_GPSR) & GPSR_CMDFE) != 1);
	for (i = 0; i < nparams; i += 4) {
		reg = params[i];
		if ((i + 1) < nparams)
			reg |= (uint32_t(params[i + 1]) << 8);
		if ((i + 2) < nparams)
			reg |= (uint32_t(params[i + 2]) << 16);
		if ((i + 3) < nparams)
			reg |= (uint32_t(params[i + 3]) << 24);
		iow(DSI_GPDR, reg);
	}
	data0 = nparams & 0xff;
	data1 = (nparams >> 8) & 0xff;

	reg = (data_type << GHCR_DT_S);
	reg |= (vchid << GHCR_VCID_S);
	reg |= (data0 << GHCR_WCLSB_S);
	reg |= (data1 << GHCR_WCMSB_S);
	iow(DSI_GHCR, reg);
    return error::success;
}

//! GPIO configuration
int dsi::gpio_conf(bool en) noexcept
{   
    const auto mux = dt::get_periph_pin_mux(io());
    if(mux<0) return mux;
    auto pin = dt::get_periph_pin(io(),dt::pinfunc::dsi_te);
    if(pin<0) return pin;
    if(en)
        gpio::setup(pin, gpio::mode::alt{gpio::outtype::pushpull, mux, gpio::speed::high});
    else
        gpio::setup(pin, gpio::mode::in{gpio::pulltype::floating});
    return error::success;
}

//! Hw acccess
inline void dsi::iow(uint32_t reg, uint32_t val) const noexcept
{
     *reinterpret_cast<volatile uint32_t*>(m_base+reg) = val;
}
//! HW access
inline uint32_t dsi::ior(uint32_t reg) const noexcept
{
    return *reinterpret_cast<volatile uint32_t*>(m_base+reg);
}

//! Hardware setup controller
void dsi::hardware_setup() noexcept 
{
    using namespace detail;
    uint32_t HorizontalSyncActive;
	uint32_t HorizontalBackPorch;
	uint32_t HorizontalLine;
	uint32_t lane_byte_clk_khz;
	uint32_t lcd_clock;
	uint32_t val;
	int	reg;

	dbg_info("DSI version: %x", ior(DSI_VR));
 
	/* Enable the regulator */
	reg = ior(DSI_WRPCR);
	reg |= (WRPCR_REGEN);
	iow(DSI_WRPCR, reg);

	/* Wait ready */
	/* TODO: timeout ? */
	while ((ior(DSI_WISR) & WISR_RRS) == 0)
		continue;

	/* Configure PLL */
	reg = ior(DSI_WRPCR);
	reg &= ~(WRPCR_NDIV_M << WRPCR_NDIV_S);
	reg |= (125 << WRPCR_NDIV_S);
	reg &= ~(WRPCR_ODF_M << WRPCR_ODF_S);
	reg |= (0 << WRPCR_ODF_S);
	reg &= ~(WRPCR_IDF_M << WRPCR_IDF_S);
	reg |= (2 << WRPCR_IDF_S);
	iow(DSI_WRPCR, reg);

	/* Enable the PLL */
	reg = ior(DSI_WRPCR);
	reg |= (WRPCR_PLLEN);
	iow(DSI_WRPCR, reg);

	/* Wait the PLL to be ready */
	/* TODO: timeout ? */
	while ((ior(DSI_WISR) & WISR_PLLLS) == 0)
		continue;

	/* Enable PHY clock and digital data */
	reg = (PCTLR_CKE | PCTLR_DEN);
	iow(DSI_PCTLR, reg);

	/* Lanes configuration */
	reg = (CLCR_DPCC | CLCR_ACR); /* High speed, automatic lane control */
	iow(DSI_CLCR, reg);

	/* 2 lanes */
	reg = (PCONFR_NL_2 << PCONFR_NL_S);
	iow(DSI_PCONFR, reg);

	reg = ior(DSI_CCR);
	reg &= ~(CCR_TXECKDIV_M << CCR_TXECKDIV_S);
	reg |= (4 << CCR_TXECKDIV_S);
	iow(DSI_CCR, reg);

	/*
	 * Formula:
	 * tempIDF = (PLLIDF > 0) ? PLLIDF : 1;
	 * (4000000 * tempIDF * (1 << PLLODF)) / ((HSE_VALUE/1000) * PLLNDIV);
	 */

	val = (4000000 * 2 * (1 << 0)) / ((8000000/1000) * 125);

	reg = ior(DSI_WPCR0);
	reg &= ~(WPCR0_UIX4_M << WPCR0_UIX4_S);
	reg |= (val << WPCR0_UIX4_S);
	iow(DSI_WPCR0, reg);

	/* Video mode */
	reg = ior(DSI_MCR);
	reg &= ~(MCR_CMDM);	/* Video mode */
	iow(DSI_MCR, reg);

	reg = ior(DSI_WCFGR);
	reg &= ~(WCFGR_DSIM);	/* Video mode */
	iow(DSI_WCFGR, reg);

	reg = ior(DSI_VMCR);
	reg &= ~(VMCR_VMT_M << VMCR_VMT_S);
	reg |= (VMCR_VMT_BRST << VMCR_VMT_S);
	iow(DSI_VMCR, reg);

	reg = (m_fbinfo->width << VPCR_VPSIZE_S);
	iow(DSI_VPCR, reg);

	reg = (0 << VCCR_NUMC_S); /* Number of Chunks */
	iow(DSI_VCCR, reg);

	reg = (0xfff << VNPCR_NPSIZE_S);
	iow(DSI_VNPCR, reg);

	reg = (0 << LVCIDR_VCID_S); /* Virtual Channel ID */
	iow(DSI_LVCIDR, reg);

	reg = 0; /* All active high */
	iow(DSI_LPCR, reg);

	switch (m_fbinfo->bpp) {
	case 24:
		reg = (LCOLCR_COLC_24 << LCOLCR_COLC_S);
		break;
	case 16:
		reg = (LCOLCR_COLC_16_1 << LCOLCR_COLC_S);
		break;
	default:
		dbg_err("Error: can't configure LCOLCR");
	}
	iow(DSI_LCOLCR, reg);

	reg = ior(DSI_WCFGR);
	reg &= ~(WCFGR_COLMUX_M << WCFGR_COLMUX_S);
	switch (m_fbinfo->bpp) {
	case 24:
		reg |= (COLMUX_24 << WCFGR_COLMUX_S);
		break;
	case 16:
		reg |= (COLMUX_16_1 << WCFGR_COLMUX_S);
		break;
	default:
		dbg_err("Error: can't configure COLMUX");
	}
	iow(DSI_WCFGR, reg);

	/* TODO: calculate this */
	lane_byte_clk_khz = 62500;
	lcd_clock = 4000;

	HorizontalLine = ((m_fbinfo->width + m_fbinfo->hsync + m_fbinfo->hbp + m_fbinfo->hfp)
	    * lane_byte_clk_khz) / lcd_clock;
	HorizontalBackPorch = (m_fbinfo->hbp * lane_byte_clk_khz) / lcd_clock;
	HorizontalSyncActive = (m_fbinfo->hsync * lane_byte_clk_khz) / lcd_clock;

	iow(DSI_VHBPCR, HorizontalBackPorch);
	iow(DSI_VHSACR, HorizontalSyncActive);
	iow(DSI_VLCR, HorizontalLine);

	iow(DSI_VVSACR, m_fbinfo->vsync);	/* Vertical sync active */
	iow(DSI_VVBPCR, m_fbinfo->vbp);		/* Vertical back porch */
	iow(DSI_VVFPCR, m_fbinfo->vfp);		/* Vertical Front Porch */
	iow(DSI_VVACR, m_fbinfo->height);	/* Vertical Active */

	/* Low power configuration */
	reg = ior(DSI_VMCR);
	reg |= VMCR_LPCE;
	iow(DSI_VMCR, reg);

	reg = (64 << LPMCR_VLPSIZE_S);
	reg |= (64 << LPMCR_LPSIZE_S);
	iow(DSI_LPMCR, reg);

	reg = ior(DSI_VMCR);
	reg |= VMCR_LPHFPE;
	reg |= VMCR_LPHBPE;
	reg |= VMCR_LPVAE;
	reg |= VMCR_LPVFPE;
	reg |= VMCR_LPVBPE;
	reg |= VMCR_LPVSAE;
	/* reg |= VMCR_FBTAAE; */
	iow(DSI_VMCR, reg);

	/* Enable the DSI */
	ior(DSI_CR);
	reg |= CR_EN;
	iow(DSI_CR, reg);

	reg = ior(DSI_WCR);
	reg |= WCR_DSIEN;
	/* reg |= (1 << 0); */ /* 8 color mode */
	iow(DSI_WCR, reg);

	dbg_info("DSI configuration done");

	reg = ior(DSI_WCR);
	reg |= WCR_LTDCEN;
	iow(DSI_WCR, reg);

}

//! Write operation
int dsi::write(int addr, const uint8_t* buf, size_t siz) noexcept
{
    if (siz < 2)
		return raw_write(addr, DCS_SHORT_PKT_WRITE_P1,buf[0], buf[1]); 
	else
		return raw_write(addr, DCS_LONG_PKT_WRITE, buf, siz); 
}

}
#endif /* ifdef DSI */
