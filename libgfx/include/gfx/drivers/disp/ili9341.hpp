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
	/* Set PIXEL */
	virtual int set_pixel( coord_t x, coord_t y, color_t color );
	/* Get PIXEL */
	virtual color_t get_pixel();
	/* Set PIXEL */
	virtual int clear( color_t color );
	/* Fill area */
	virtual int fill( coord_t x, coord_t y, coord_t cx, coord_t cy );
	/* Blit area */
	virtual int blit( coord_t x, coord_t y, coord_t cx, coord_t cy,
					   coord_t src_x, coord_t src_y, coord_t src_cx, const color_t *buf );
	/* Vertical scroll */
	virtual int vert_scroll( coord_t x, coord_t y, coord_t cx, coord_t cy, int lines, color_t bgcolor );
	/* Power ctl */
	virtual int power_ctl( power_ctl_t mode );
	/* Rotate screen */
	virtual int rotate( rotation_t rot );
private:
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
		DISPLAYON =   0x29,
		SLEEPOUT  =   0x11,
		COLADDRSET =  0x2A,
		PAGEADDRSET = 0x2B,
		MEMORYWRITE = 0x2C,
	};
private:
	static constexpr auto CSL_BIT_CMD = 0;
	static constexpr auto RS_BIT_CMD = 1;
	static constexpr auto RST_BIT_CMD = 2;
	//Initialize display
	int init_display();
	//Command 
	void command( dcmd cmd )
	{
		m_bus.set_ctlbits( CSL_BIT_CMD, false );
		m_bus.set_ctlbits( RS_BIT_CMD,  false );
		m_bus.write( &cmd, sizeof cmd );
		m_bus.set_ctlbits( RS_BIT_CMD, true );
	}
	void command_()
	{
		m_bus.set_ctlbits( CSL_BIT_CMD, true );
	}
	template <typename ...ARGS>
		void command_( uint8_t first, ARGS... args )
	{
		m_bus.write( &first, sizeof(first) );
		command_( args...);
	}
	template <typename ...ARGS>
		void command( dcmd cmd, ARGS... args )
	{
		command( cmd );
		command_( args... );
	}
private:
	disp_bus &m_bus;
};

/* ------------------------------------------------------------------ */
}}
/* ------------------------------------------------------------------ */
#endif /* ILI9341_HPP_ */
