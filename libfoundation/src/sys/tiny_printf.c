
/*
 * printf.h
 *
 *  Created on: 2009-08-08
 *      Author: lucck
 */

#include <stdarg.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stddef.h>
#include "foundation/sys/tiny_printf.h"
#include "foundation/sys/tiny_vaprintf.h"

//Function pointer to putchar
static int (*fn_putc)(int,void* ) = NULL;
static void * fn_putc_arg = NULL;
static void (*fn_lock)(void) = NULL;
static void (*fn_unlock)(void) = NULL;

static void printchar(char **str,int c)
{
	if (str)
	{
		**str = c;
		++(*str);
	}
	else if(fn_putc)
	{
		fn_putc(c,fn_putc_arg);
	}
}

int tiny_putchar( int ch )
{
	return fn_putc( ch, fn_putc_arg );
}

int tiny_puts( const char* str )
{
	while( *str ) {
		tiny_putchar( *str++ );
	}
	tiny_putchar( '\r' );
	tiny_putchar( '\n' );
	return 1;
}

#define PAD_RIGHT 1
#define PAD_ZERO 2

#define range() if(len>0 && pc>=len) return pc

static int prints(char **out, size_t len, const char *string, int width, int pad)
{
	register size_t pc = 0, padchar = ' ';

	if (width > 0)
	{
		register int len = 0;
		register const char *ptr;
		for (ptr = string; *ptr; ++ptr) ++len;
		if (len >= width) width = 0;
		else width -= len;
		if (pad & PAD_ZERO) padchar = '0';
	}
	if (!(pad & PAD_RIGHT))
	{
		for ( ; width > 0; --width)
		{
			range();
			printchar (out, padchar);
			++pc;
		}
	}
	for ( ; *string ; ++string)
	{
		range();
		printchar (out, *string);
		++pc;
	}
	for ( ; width > 0; --width)
	{
		range();
		printchar (out, padchar);
		++pc;
	}

	return pc;
}


/* the following should be enough for 32 bit int */
#define PRINT_BUF_LEN 12

static int printi(char **out, size_t len, int i, int b, int sg, int width, int pad, int letbase)
{
	char print_buf[PRINT_BUF_LEN];
	register char *s;
	register int t, neg = 0;
	size_t pc = 0;
	register unsigned int u = i;

	if (i == 0)
	{
		print_buf[0] = '0';
		print_buf[1] = '\0';
		return prints (out,len, print_buf, width, pad);
	}

	if (sg && b == 10 && i < 0)
	{
		neg = 1;
		u = -i;
	}

	s = print_buf + PRINT_BUF_LEN-1;
	*s = '\0';

	while (u)
	{
		t = u % b;
		if( t >= 10 )
			t += letbase - '0' - 10;
		*--s = t + '0';
		u /= b;
	}

	if (neg)
	{
		if( width && (pad & PAD_ZERO) )
		{
			range();
			printchar (out, '-');
			++pc;
			--width;
		}
		else
		{
			*--s = '-';
		}
	}

	return pc + prints (out, len ,s, width, pad);
}

#define unsigned_cast() \
	int val; \
	switch( modf ) \
	{ \
	case m_hh: val = (unsigned char)va_arg( args, int ); break; \
	case m_h:  val = (unsigned short)va_arg( args, int ); break; \
	case m_l:  val =  va_arg( args, unsigned long ); break; \
	default:   val =  va_arg( args, unsigned int ); break; \
	} do {} while(0)


int tiny_vaprintf(char **out, size_t len, const char *format, va_list args )
{
	enum { m_hh, m_h, m_n, m_l };
	int width, pad;
	size_t pc = 0;
	char scr[2];
	unsigned char modf = m_n;
	for (; *format != 0; ++format)
	{
		if (*format == '%')
		{
			++format;
			width = pad = 0;
			if (*format == '\0') break;
			if (*format == '%') goto out;
			if (*format == '-')
			{
				++format;
				pad = PAD_RIGHT;
			}
			while (*format == '0')
			{
				++format;
				pad |= PAD_ZERO;
			}
			for ( ; *format >= '0' && *format <= '9'; ++format)
			{
				width *= 10;
				width += *format - '0';
			}
			if( *format == 'h' )
			{
				modf = m_h;
				format++;
			}
			else if( *format == 'l' || *format == 'z' )
			{
				modf = m_l;
				format++;
			}
			if( *format == 'h' )
			{
				modf = m_hh;
				format++;
			}
			if( *format == 's' )
			{
				const char *s = va_arg( args, const char* );
				pc += prints (out,len, s?s:"(null)", width, pad);
				continue;
			}
			if( *format == 'd' || *format == 'i' )
			{
				int val;
				switch( modf )
				{
				case m_hh: val = (signed char)va_arg( args, int ); break;
				case m_h:  val = (signed short)va_arg( args, int ); break;
				case m_l:  val =  va_arg( args, long ); break;
				default:   val =  va_arg( args, int ); break;
				}
				pc += printi (out,len, val, 10, 1, width, pad, 'a');
				continue;
			}
			if( *format == 'x' )
			{
				unsigned_cast();
				pc += printi (out,len, val, 16, 0, width, pad, 'a');
				continue;
			}
			if( *format == 'X' )
			{
				unsigned_cast();
				pc += printi (out,len, val, 16, 0, width, pad, 'A');
				continue;
			}
			if( *format == 'u' )
			{
				unsigned_cast();
				pc += printi (out,len, val, 10, 0, width, pad, 'a');
				continue;
			}
			else if( *format == 'p')
			{
				pc += printi (out,len, va_arg( args, int ), 16, 0, sizeof(void*)*2, PAD_ZERO, 'A');
			}
			if( *format == 'c' )
			{
				/* char are converted to int then pushed on the stack */
				scr[0] = (char)va_arg( args, int );
				scr[1] = '\0';
				pc += prints (out,len, scr, width, pad);
				continue;
			}
		}
		else
		{
		out:
			range();
			printchar (out, *format);
			++pc;
		}
	}
	if (out) **out = '\0';
	va_end( args );
	return pc;
}



int tiny_printf(const char *format, ...)
{
	int result;
	va_list args;
	va_start( args, format );
	if(fn_lock && fn_unlock) fn_lock();
    result = tiny_vaprintf( NULL,0, format, args );
    if(fn_lock && fn_unlock) fn_unlock();
	va_end( args );
    return  result;
}



int tiny_snprintf(char *out, unsigned long max_len, const char *format, ...)
{
    va_list args;
    va_start( args, format );
    int ret =  tiny_vaprintf( &out, max_len, format, args );
	va_end( args );
	return ret;
}


void register_printf_putc_handler_syslock(int (*fputc)(int,void*),void *arg,
		void (*lock)(void),void (*unlock)(void))
{
    fn_putc = fputc;
    fn_putc_arg = arg;
    fn_lock = lock;
    fn_unlock = unlock;
}



