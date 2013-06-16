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
void gdi::draw_text(coord_t x, coord_t y, int ch )
{
	if( ch < 0 || ch >= m_font->size )
		ch = m_font->defaultchar;
	const auto buf = m_gdev.get_rbuf();
	const auto width = (m_font->width)?(m_font->width[ch]):(m_font->maxwidth);
	if( x + width > m_gdev.get_width() )
		return;
	const auto bmpch = m_font->offset[ch];
	const auto rows_in_buf = buf.second / width;
	coord_t p_blit_cy = 0;
	auto blit_rows = m_font->height<rows_in_buf?m_font->height:rows_in_buf;
	for( unsigned row=0,bi=0; row<m_font->height; ++row )
	{
		const bit bits( m_font->bits + bmpch + row );
		for( int col=0; col<width; ++col,++bi )
		{
			buf.first[bi] = bits[col]?m_color:m_bg_color;
		}
		if( row+1 >= (p_blit_cy+blit_rows) )
		{
			m_gdev.blit( x, p_blit_cy + y, width, blit_rows, 0, buf.first );
			bi = 0;
			p_blit_cy += blit_rows;
			blit_rows = (m_font->height-row)<rows_in_buf?(m_font->height-row):rows_in_buf;
		}
	}
}
/* ------------------------------------------------------------------ */
void gdi::draw_text( coord_t x, coord_t y, const char* str )
{
	const auto dwidth = m_gdev.get_width();
	for( coord_t px=x; *str; ++str )
	{
		const int ch = ( *str >= m_font->size )?(m_font->defaultchar):(*str);
		const auto width = (m_font->width)?(m_font->width[ch]):(m_font->maxwidth);
		draw_text( px, y, ch );
		px += width;
		if( px > dwidth ) break;
	}
}
/* ------------------------------------------------------------------ */
}}


/* ------------------------------------------------------------------ */
