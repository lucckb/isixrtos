/**
 * =====================================================================================
 * 	File: fbdev.cpp
 *  Description: Framebuffer device driver based on STM32 LTDC
 * 	Created Date: Sunday, August 18th 2019, 7:51:42 pm
 * 	Author: Lucjan Bryndza
 * 	Copyright (c) 2019 BoFF
 * 
 * 	GPL v2/3
 * =====================================================================================
 */
#include <periph/drivers/display/rgb/fbdev.hpp>
#include <foundation/sys/dbglog.h>
#include <periph/core/error.hpp>
#include <periph/dt/dts.hpp>
#include <periph/clock/clocks.hpp>
#include <periph/drivers/display/rgb/display_config.hpp>
#include <stm32_ll_rcc.h>
#include "ltdc_regs.hpp"

#ifdef LTDC
namespace periph::display {
//! Hw acccess
inline void fbdev::iow(uint32_t reg, uint32_t val) const noexcept
{
     *reinterpret_cast<volatile uint32_t*>(m_base+reg) = val;
}
//! HW access
inline uint32_t fbdev::ior(uint32_t reg) const noexcept
{
    return *reinterpret_cast<volatile uint32_t*>(m_base+reg);
}


//! Default constructor
fbdev::fbdev(const char dev_name[])
     : m_base(dt::get_periph_base_address(dev_name))
{
     if(!m_base) {
        error::expose<error::bus_exception>(error::invstate);
    }
    const dt::device_conf_base* base;
    int ret = dt::get_periph_devconf(dev_name,base);
    error::expose<error::bus_exception>(ret);
    m_fbinfo = reinterpret_cast<const fb_info*>(base);
    if(m_fbinfo->n_layers<1) {
        error::expose<error::bus_exception>(error::inval);
    }
}

/** Open and enable framebuffer device
 * @return error code;
 */
int fbdev::open()
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
        //Configure controller
        for(auto i=0U;i<m_fbinfo->n_layers;++i) {
            hardware_setup(m_fbinfo->layers[i],i);   
        }
    } while(0);
    return ret;
}
/** Close and disable framebuffer device
 * @return error code
 */
int fbdev::close()
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
    } while(0);
    return ret;
}
    
//! Framebuffer hardware setup
void fbdev::hardware_setup(const layer_info& info, int i) noexcept
{
	uint32_t reg;
	uint32_t val;
	reg = ((info.hsync - 1) << SSCR_HSW_S);
	reg |= ((info.vsync - 1) << SSCR_VSW_S);
	iow( LTDC_SSCR, reg);

	reg = ((info.hsync + info.hbp - 1) << BPCR_AHBP_S);
	reg |= ((info.vsync + info.vbp - 1) << BPCR_AVBP_S);
	iow( LTDC_BPCR, reg);

	reg = ((info.vsync + info.vbp + info.height - 1) << AWCR_AAH_S);
	reg |= ((info.hsync + info.hbp + info.width - 1) << AWCR_AAW_S);
	iow( LTDC_AWCR, reg);

	val = info.vsync + info.vbp + info.height + info.vfp - 1;
	reg = val << TWCR_TOTALH_S;
	val = info.hsync + info.hbp + info.width + info.hfp - 1;
	reg |= val << TWCR_TOTALW_S;
	iow( LTDC_TWCR, reg);

	/* Background color */
	iow( LTDC_BCCR, 0xeeffee);

	layer(info, i);

	iow( LTDC_SRCR, SRCR_IMR);

	reg = ior( LTDC_GCR);
	//reg |= (GCR_HSPOL | GCR_VSPOL | GCR_DEPOL | GCR_PCPOL);
	iow( LTDC_GCR, reg);

	reg = ior( LTDC_GCR);
	reg |= (GCR_LTDCEN);
	iow( LTDC_GCR, reg);

	dbg_info("ltdc initialized");

}

//! Single layer config
void fbdev::layer(const layer_info& info, int i) noexcept
{
    int nbytes_per_pixel;
	uint32_t reg;
	dbg_info("layer->base %x", info.base);
	reg = (info.hsync + info.hbp) |
	    ((info.hsync + info.hbp + info.width - 1) << 16);
	iow( LTDC_LWHPCR(i), reg);

	reg = (info.vsync + info.vbp) |
	    ((info.vsync + info.vbp + info.height - 1) << 16);
	iow( LTDC_LWVPCR(i), reg);

	if (info.bpp == 16)
		iow( LTDC_LPFCR(i), 2); /* 010: RGB565 */
	else if (info.bpp == 24)
		iow( LTDC_LPFCR(i), 1); /* 001: RGB888 */
	else if (info.bpp == 32)
		iow( LTDC_LPFCR(i), 0); /* 000: ARGB8888 */
	iow( LTDC_LCFBAR(i), info.base);

	nbytes_per_pixel = info.bpp / 8;
	reg = ((info.width * nbytes_per_pixel) << 16);
	reg |= ((info.width * nbytes_per_pixel) + 3);
	iow( LTDC_LCFBLR(i), reg);

	/* Enable the layer */
	reg = ior(LTDC_LCR(i));
	reg |= (LCR_LEN);
	iow( LTDC_LCR(i), reg);
}

//! Get FB memory buffer
void* fbdev::fbmem(size_t idx) const noexcept
{
	return reinterpret_cast<void*>(m_fbinfo->layers[idx].base);
}

}
#endif /* ifdef LTDC */

