/*
 * ili9341.hpp
 *
 *  Created on: 10-06-2013
 *      Author: lucck
 */

#ifndef ISIX_GFX_ILI9341_HPP_
#define ISIX_GFX_ILI9341_HPP_
/* ------------------------------------------------------------------ */
#include "disp_base.hpp"
#include "disp_bus.hpp"
#include <cstdint>
#include <initializer_list>
/* ------------------------------------------------------------------ */
namespace gfx {
namespace drv {
/* ------------------------------------------------------------------ */
/* ILI 9341 hw driver */
class ili9341 : public disp_base
{
	static constexpr auto SCREEN_WIDTH = 240;
	static constexpr auto SCREEN_HEIGHT = 320;
public:
	//Constructor
	ili9341( disp_bus &bus );
	//Destructor
	virtual ~ili9341()
	{
	}
	/* Get PIXEL */
	virtual color_t get_pixel( coord_t x, coord_t y );
	/* Set PIXEL */
	virtual void set_pixel( coord_t x, coord_t y, color_t color );
	/* Clear the screen */
	virtual void clear( color_t color );
	/* Blit area */
	virtual void blit( coord_t x, coord_t y, coord_t cx, coord_t cy,
	        coord_t src_y, const color_t *buf );

	/* Fill area */
	virtual void fill( coord_t x, coord_t y, coord_t cx, coord_t cy, color_t color );

	/* Vertical scroll */
	virtual void vert_scroll( coord_t x, coord_t y, coord_t cx, coord_t cy, int lines, color_t bgcolor );
	/* Power ctl */
	virtual void power_ctl( power_ctl_t mode );
	/* Rotate screen */
	virtual void rotate( rotation_t rot );
	static constexpr color_t rgb( uint8_t r, uint8_t g, uint8_t b )
	{
		return ((b>>3)<<(16-5)) | ((g>>2)<<(16-5-6) | (r>>3) );
	}
private:
	//Colorspace
	static constexpr auto MADREG_RGB = 0;	//(1<<3)
	//Display commands
	enum class dcmd : uint8_t
	{
		POWERCTLB 	= 0xCF,
		POWERONSEQCTL = 0xED,
		DIVTIMCTLA 	= 0xE8,
		POWERCTLA 	= 0xC8,
		PUMPRATIOCTL = 0xF7,
		DIVTIMCTLB 	= 0xEA,
		FRAMECTLNOR = 0xB1,
		POWERCTL1	= 0xC0,
		POWERCTL2 	= 0xC1,
		VCOMCTL1	= 0xC5,
		VCOMCTL2	= 0xC7,
		MEMACCESS	= 0x36,
		FUNCTIONCTL = 0xB6,
		ENABLE3GREG = 0xF2,
		GAMMASET = 	  0x26,
		POSGAMMACORRECTION  = 0xE0,
		NEGGAMMACORRECTION  = 0xE1,
		PIXFORMATSET = 0x3A,
		INTERFCTL =   0xF6,
		DISPLAYOFF =  0x28,
		DISPLAYON =   0x29,
		SLEEPENTER	  =   0x10,
		SLEEPOUT  =   0x11,
		COLADDRSET =  0x2A,
		PAGEADDRSET = 0x2B,
		MEMORYWRITE = 0x2C,
		MEMORYREAD  = 0x2E,
		READSELFDIAG = 0x0F,
		MADCTL = 0x36,
		VERTSCROLLDEF = 0x33,
		VERTSCROLLSTART = 0x37,
		NORMALMODEON    = 0x13

	};
private:
	static constexpr auto CSL_BIT_CMD = 0;
	static constexpr auto RS_BIT_CMD = 1;
	static constexpr auto RST_BIT_CMD = 2;
	//Initialize display
	void init_display();
	//Command 
	void command( dcmd cmd )
	{
		m_bus.set_ctlbits( RS_BIT_CMD,  false );
		m_bus.write( &cmd, sizeof cmd );
		m_bus.set_ctlbits( RS_BIT_CMD, true );
	}
	class bus_lock
	{
	public:
		~bus_lock()
		{
			m_bus.set_ctlbits( CSL_BIT_CMD, true );
		}
		bus_lock( disp_bus &bus )
			: m_bus( bus )
		{
			m_bus.set_ctlbits( CSL_BIT_CMD, false );
		}
	private:
		disp_bus &m_bus;
	};
	template <typename ...ARGS>
		void command( dcmd cmd, ARGS... args )
	{
		command( cmd );
		const uint8_t cmd_tab[ ] { uint8_t(args)... };
		m_bus.write( cmd_tab, sizeof cmd_tab );
	}
	//Reset device
	void reset();
	//Set viewport
	void set_viewport( coord_t x, coord_t y, coord_t cx, coord_t cy );
private:
	disp_bus &m_bus;
	rotation_t m_orient { rotation_t::rot_0 };
	power_ctl_t m_pwrstate {};
};

/* ------------------------------------------------------------------ */
}}
/* ------------------------------------------------------------------ */
#endif /* ILI9341_HPP_ */
