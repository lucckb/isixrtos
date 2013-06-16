/*
 * gdi.cpp
 *
 *  Created on: 15-06-2013
 *      Author: lucck
 */
#include <gfx/disp/gdi.hpp>
#include <dbglog.h>
#include <cstring>
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
	if( ch < m_font->firstchar || ch >= m_font->size )
		ch = m_font->defaultchar;
	const auto buf = m_gdev.get_rbuf();
	const auto width = (m_font->width)?(m_font->width[ch]):(m_font->maxwidth);
	if( x + width > m_gdev.get_width() )
		return;
	const auto bmpch = m_font->offset[ ch - m_font->firstchar ];
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
//Draw text
void gdi::draw_text( coord_t x, coord_t y, const char* str )
{
	const auto dwidth = m_gdev.get_width();
	for( coord_t px=x; *str; ++str )
	{
		const int ch = ( *str < m_font->firstchar || *str >= m_font->size )?(m_font->defaultchar):(*str);
		const auto width = (m_font->width)?(m_font->width[ch]):(m_font->maxwidth);
		draw_text( px, y, ch );
		px += width;
		if( px > dwidth ) break;
	}
}

/* ------------------------------------------------------------------ */
/** Get text width */
coord_t gdi::get_text_width( const char *str )
{
	if( !m_font->width )
		return std::strlen( str ) * m_font->maxwidth;
	else
	{
		coord_t width;
		for(width=0;*str; ++str )
		{
			const int ch = ( *str < m_font->firstchar || *str >= m_font->size )?(m_font->defaultchar):(*str);
			width += m_font->width[ch];
		}
		return width;
	}
}
/* ------------------------------------------------------------------ */
//Draw line
void gdi::draw_line( coord_t x0, coord_t y0, coord_t x1, coord_t y1 )
{

	// Fill vert and horiz lines
	if (x0 == x1)
	{
		if (y1 > y0)
			m_gdev.fill(x0, y0, 1, y1-y0+1, m_color);
		else
			m_gdev.fill(x0, y1, 1, y0-y1+1, m_color);
		return;
	}
	if (y0 == y1)
	{
		if (x1 > x0)
			m_gdev.fill(x0, y0, x1-x0+1, 1,  m_color);
		else
			m_gdev.fill(x0, y1, x0-x1+1, 1,  m_color);
		return;
	}
	int dx, addx;
	int dy, addy;
	if (x1 >= x0)
	{
		dx = x1 - x0;
		addx = 1;
	}
	else
	{
		dx = x0 - x1;
		addx = -1;
	}
	if (y1 >= y0)
	{
		dy = y1 - y0;
		addy = 1;
	}
	else
	{
		dy = y0 - y1;
		addy = -1;
	}

	if (dx >= dy)
	{
		dy *= 2;
		auto p = dy - dx;
		auto diff = p - dx;

		for(int i=0; i<=dx; ++i)
		{
			m_gdev.set_pixel(x0, y0, m_color);
			if (p < 0)
			{
				p  += dy;
				x0 += addx;
			}
			else
			{
				p  += diff;
				x0 += addx;
				y0 += addy;
			}
		}
	}
	else
	{
		dx *= 2;
		auto p = dx - dy;
		auto diff = p - dy;

		for(int i=0; i<=dy; ++i)
		{
			m_gdev.set_pixel(x0, y0, m_color);
			if(p < 0)
			{
				p  += dx;
				y0 += addy;
			}
			else
			{
				p  += diff;
				x0 += addx;
				y0 += addy;
			}
		}
	}
}

/* ------------------------------------------------------------------ */
/** Draw circle */
void gdi::draw_circle( coord_t x, coord_t y, coord_t radius )
{

	int a = 0;
	int b = radius;
	int p = 1 - radius;
	do
	{
		if( !m_bg_fill )
		{
			set_pixel(x+a, y+b);
			set_pixel(x+b, y+a);
			set_pixel(x-a, y+b);
			set_pixel(x-b, y+a);
			set_pixel(x+b, y-a);
			set_pixel(x+a, y-b);
			set_pixel(x-a, y-b);
			set_pixel(x-b, y-a);
		}
		else
		{
			draw_line(x-a, y+b, x+a, y+b);
			draw_line(x-a, y-b, x+a, y-b);
			draw_line(x-b, y+a, x+b, y+a);
			draw_line(x-b, y-a, x+b, y-a);
		}
		if (p < 0)
			p += 3 + 2*a++;
		else
			p += 5 + 2*(a++ - b--);
	}
	while(a <= b);
}

/* ------------------------------------------------------------------ */
/** Draw ellipse */
void gdi::draw_ellipse( coord_t x, coord_t y, coord_t a, coord_t b )
{
	int  dx = 0, dy = b;
	int a2 = a*a, b2 = b*b;
	int err = b2-(2*b-1)*a2, e2;
	do
	{
			if( !m_bg_fill )
			{
				set_pixel(x+dx, y+dy); /* I. Quadrant */
				set_pixel(x-dx, y+dy); /* II. Quadrant */
				set_pixel(x-dx, y-dy); /* III. Quadrant */
				set_pixel(x+dx, y-dy); /* IV. Quadrant */
			}
			else
			{
				draw_line(x-dx,y+dy,x+dx,y+dy);
				draw_line(x-dx,y-dy,x+dx,y-dy);
			}
			e2 = 2*err;
			if(e2 <  (2*dx+1)*b2) {
				dx++;
				err += (2*dx+1)*b2;
			}
			if(e2 > -(2*dy-1)*a2) {
				dy--;
				err -= (2*dy-1)*a2;
			}
	}
	while(dy >= 0);

	while(dx++ < a)
	{
		set_pixel(x+dx, y);
		set_pixel(x-dx, y);
	}
}
/* ------------------------------------------------------------------ */
}}


/* ------------------------------------------------------------------ */
