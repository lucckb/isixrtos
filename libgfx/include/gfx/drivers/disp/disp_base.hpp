/*
 * disp_base.hpp
 *
 *  Created on: 10-06-2013
 *      Author: lucck
 */

 
#ifndef ISIX_GFX_DISP_BASE_HPP_
#define ISIX_GFX_DISP_BASE_HPP_
 
#include <gfx/types.hpp>
#include <utility>
#include <cstddef>
 
namespace gfx {
namespace drv {
 
//Display power control
enum class power_ctl_t : short
{
	unknown,
	off,
	on,
	sleep
};
 
enum class rotation_t : short
{
	rot_0,
	rot_90,
	rot_180,
	rot_270
};
 
class disp_base
{
	//Make object noncopyable
	disp_base(const disp_base&) = delete;
	disp_base& operator=(const disp_base&) = delete;
	static constexpr auto invalid_value = -1;
public:
	disp_base( coord_t width, coord_t height )
		: m_width( width ), m_height( height ) {
	}
	virtual ~disp_base() {
	}
	/* Set PIXEL */
	virtual void set_pixel( coord_t x, coord_t y, color_t color ) = 0;
	/* Get PIXEL */
	virtual color_t get_pixel( coord_t x, coord_t y ) = 0;
	/* Set PIXEL */
	virtual void clear( color_t color ) = 0;
	/* Fill area */
	virtual void fill( coord_t x, coord_t y, coord_t cx, coord_t cy, color_t color ) = 0;

	/* Set blit area (viewport) */
	virtual void ll_blit( coord_t x, coord_t y, coord_t cx, coord_t cy ) = 0;

	/* Push into the memory */
	virtual void ll_blit( const color_t *buf, size_t len ) = 0;

	/* Blit area */
	virtual void blit( coord_t x, coord_t y, coord_t cx, coord_t cy,
	        coord_t src_y, const color_t *buf ) = 0;
	/* Vertical scroll */
	virtual void vert_scroll( coord_t /*x*/, coord_t /*y*/, coord_t /*cx*/, coord_t /*cy*/, int /*lines*/, color_t /*bgcolor*/ )
	{
	}
	/* Power ctl */
	virtual bool power_ctl( power_ctl_t mode ) = 0;
	/* Rotate screen */
	virtual void rotate( rotation_t /*rot*/ )
	{
	}
	/* Set backlight percent */
	virtual void backlight( int /*percent*/ )
	{
	}
	/* Get backlight percent */
	virtual int backlight() 
	{
		return invalid_value;
	}
	//Optional rendeer buff (can be used if vert scrool is not required )
	virtual std::pair<color_t*,size_t> get_rbuf() = 0;
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
protected:
	void set_screen_size( coord_t w, coord_t h )
	{
		m_width =  w;
		m_height = h;
	}
private:
	coord_t m_width {}, m_height {};	//Disp width and height
};

 
}}
 

#endif /* ISIX_GFX_DISP_BASE_HPP_ */

 
