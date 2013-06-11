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
#include <utility>
/* ------------------------------------------------------------------ */
namespace gfx {
namespace drv {
/* ------------------------------------------------------------------ */
/* ILI 9341 hw driver */
class ili9341 : public disp_base
{
	static constexpr auto SCREEN_WIDTH = 240;
	static constexpr auto SCREEN_HEIGHT = 320;
	typedef uint8_t cmd_t;
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
	virtual int clear();
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
	static constexpr auto CSL_BIT_CMD = 0;
	static constexpr auto RS_BIT_CMD = 1;
	static constexpr auto RST_BIT_CMD = 2;
	//Initialize display
	int init_display();
	//Command 
	void command( cmd_t cmd )
	{
		m_bus.set_ctlbits( CSL_BIT_CMD, false );
		m_bus.set_ctlbits( RS_BIT_CMD,  false );
		m_bus.write( &cmd, sizeof(cmd) );
		m_bus.set_ctlbits( CSL_BIT_CMD, true );
	}
	template <typename T>
		void command( cmd_t cmd, T arg )
	{
		command( cmd );
		for(size_t s=0; s<sizeof(T); ++s)
		{
				
		}
	}		
private:
	disp_bus &m_bus;
};

/* ------------------------------------------------------------------ */
}}
/* ------------------------------------------------------------------ */
#endif /* ILI9341_HPP_ */
