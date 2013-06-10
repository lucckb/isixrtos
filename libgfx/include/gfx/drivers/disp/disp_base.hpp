/*
 * disp_base.hpp
 *
 *  Created on: 10-06-2013
 *      Author: lucck
 */

/* ------------------------------------------------------------------ */
#ifndef ISIX_GFX_DISP_BASE_HPP_
#define ISIX_GFX_DISP_BASE_HPP_
/* ------------------------------------------------------------------ */
#include <gfx/types.hpp>

/* ------------------------------------------------------------------ */
namespace gfx {
namespace drv {
/* ------------------------------------------------------------------ */
//Display power control
enum class power_ctl_t : short
{
	off,
	on,
	sleep
};
/* ------------------------------------------------------------------ */
enum class rotation_t : short
{
	rot_0,
	rot_90,
	rot_180,
	rot_270
};
/* ------------------------------------------------------------------ */
class disp_base
{
	//Make object noncopyable
	disp_base(const disp_base&) = delete;
	disp_base& operator=(const disp_base&) = delete;
public:
	disp_base( coord_t width, coord_t height )
		: m_width( width ), m_height( height )
	{
	}
	virtual ~disp_base()
	{
	}
	/* Set PIXEL */
	virtual int set_pixel( coord_t x, coord_t y, color_t color ) = 0;
	/* Get PIXEL */
	virtual color_t get_pixel()
	{
		//TODO: Fixme library error codes
		return -1;
	}
	/* Set PIXEL */
	virtual int clear() = 0;
	/* Fill area */
	virtual int fill( coord_t x, coord_t y, coord_t cx, coord_t cy ) = 0;
	/* Blit area */
	virtual int blit( coord_t x, coord_t y, coord_t cx, coord_t cy,
					   coord_t src_x, coord_t src_y, coord_t src_cx, const color_t *buf ) = 0;
	/* Vertical scroll */
	virtual int vert_scroll( coord_t x, coord_t y, coord_t cx, coord_t cy, int lines, color_t bgcolor ) = 0;
	/* Power ctl */
	virtual int power_ctl( power_ctl_t mode ) = 0;
	/* Rotate screen */
	virtual int rotate( rotation_t rot )
	{
		//TODO: Fixme library error codes
		return -1;
	}
	/* Set backlight percent */
	virtual int backlight( int percent )
	{
		//TODO: Fixme library error codes
		return -1;
	}
	/* Get width */
	coord_t get_width() const
	{
		return m_width;
	}
	/* Get width */
	coord_t get_height() const
	{
		return m_height;
	}
private:
	coord_t m_width {}, m_height {};	//Disp width and height
};

/* ------------------------------------------------------------------ */
}}
/* ------------------------------------------------------------------ */

#endif /* ISIX_GFX_DISP_BASE_HPP_ */

/* ------------------------------------------------------------------ */
