/*
 * display_operators.hpp
 *
 *  Created on: 23 paź 2013
 *      Author: lucck
 */

#ifndef FOUNDATION_DISPLAY_OPERATORS_HPP_
#define FOUNDATION_DISPLAY_OPERATORS_HPP_

#include <foundation/utils.h>

namespace fnd {
namespace lcd {
/*----------------------------------------------------------*/
	class lfmt
	{
	public:
		enum
		{
			dec = 10,
			hex = 16
		};
		lfmt(unsigned val_, int fmt_, char fmtch_='0', short base_=dec)
		: m_val(val_),m_fmt(fmt_),m_fmtch(fmtch_) , m_base(base_) {}
		unsigned val() const { return m_val; }
		int fmt() const { return m_fmt; }
		unsigned fmtch() const { return m_fmtch; }
		unsigned base() const { return m_base; }
	private:
		const unsigned m_val;
		const int m_fmt;
		const char m_fmtch;
		const short m_base;
	};
	/* ------------------------------------------------------------------ */
	struct icon
	{
	public:
		virtual const char* get_data() const = 0;
		virtual char get_char() const = 0;
	};
	/* ------------------------------------------------------------------ */
	class pos
	{
	public:
		pos(int x_, int y_)
		:m_x(x_), m_y(y_) {}
		unsigned short x() const { return m_x; }
		unsigned short y() const { return m_y; }
	private:
		const unsigned short m_x;
		const unsigned short m_y;
	};
	/* ------------------------------------------------------------------ */
	template <typename D>
		D& operator<<( D &o,const char *str)
	{

		while(*str)
			o.putc(*str++);
		return o;
	}

	/* ------------------------------------------------------------------ */
	template <typename D>
		D& operator<<(D &o,unsigned value)
	{
		char buf[11];
		fnd::fnd_uitoa(buf,value,1,'0',10);
		o << buf;
		return o;
	}
	/* ------------------------------------------------------------------ */
	template <typename D>
		D& operator<<(D &o,int value)
	{
		char buf[12];
		fnd::fnd_itoa(buf,value,1,'0');
		o << buf;
		return o;
	}
	/* ------------------------------------------------------------------ */
	template <typename D>
		D& operator<<(D &o,const lfmt &fmt)
	{
		char buf[11];
		fnd::fnd_uitoa(buf,fmt.val(),fmt.fmt(),fmt.fmtch(),fmt.base());
		o << buf;
		return o;
	}

	/* ------------------------------------------------------------------ */
	template <typename D>
		D& operator<<(D &o,const icon &fmt)
	{
		o.show_icon(fmt.get_char(),fmt.get_data());
		return o;
	}

	/* ------------------------------------------------------------------ */
	template <typename D>
		D& operator<<(D &o,const pos &fmt)
	{
		o.setpos(fmt.x,fmt.y);
		return o;
	}
	/* ------------------------------------------------------------------ */


}}
/*----------------------------------------------------------*/
#endif /* DISPLAY_OPERATORS_HPP_ */
/*----------------------------------------------------------*/
