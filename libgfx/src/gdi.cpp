/*
 * gdi.cpp
 *
 *  Created on: 15-06-2013
 *      Author: lucck
 */
#include <gfx/disp/gdi.hpp>
#include <dbglog.h>

/* ------------------------------------------------------------------ */
namespace gfx {
namespace disp {
/* ------------------------------------------------------------------ */
namespace {
	class bit
	{
	public:
		bit( const unsigned short bits[])
			: m_bits( bits )
		{
		}
		bool operator[](size_t idx) const
		{
			return m_bits[idx/16] & (1<<(15-idx));
		}
	private:
		const unsigned short *m_bits;
	};
}
/* ------------------------------------------------------------------ */
//Draw char
int gdi::draw_char(coord_t x, coord_t y, int ch, const font_t &font, color_t color, color_t bg_color )
{
	if( ch < 0 || ch >= font.size )
		ch = font.defaultchar;
	const auto buf = m_gdev.get_rbuf();
	const auto width = (font.width)?(font.width[ch]):(font.maxwidth);
	const auto bmpch = font.offset[ch];
	const auto rows_in_buf = buf.second / width;
	const auto blit_rows = font.height<rows_in_buf?font.height:rows_in_buf;
	auto p_blit_y = y;
	for( unsigned row=0,bi=0; row<font.height; ++row )
	{
		const bit bits( font.bits + bmpch + row );
		for( int col=0; col<width; ++col,++bi )
		{
			buf.first[bi] = bits[col]?color:bg_color;
			//if( bits[col] )
			//	m_gdev.set_pixel( x+col,y+row, color );
		}
		if( row+1 >= blit_rows )
		{
			m_gdev.blit( x, p_blit_y, width, blit_rows, 0, buf.first );
			bi = 0;
			p_blit_y += blit_rows;
		}
	}
	return 0;
}
/* ------------------------------------------------------------------ */
}}


/* ------------------------------------------------------------------ */



