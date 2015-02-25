/*
 * =====================================================================================
 *
 *       Filename:  libgsmtest.cpp
 *
 *    Description:  GSM test
 *
 *        Version:  1.0
 *        Created:  20.02.2015 23:24:55
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Lucjan Bryndza (LB), lucck(at)boff(dot)pl
 *   Organization:  BoFF
 *
 * =====================================================================================
 */

#include <foundation/dbglog.h>
#include <serialport_unix.hpp>


void print_flags( int fl ) 
{
	if( fl < 0 ) {
		dbprintf("Fatal unable to print flags");
	}
	using sp = fnd::serial_port;
	static constexpr const char* sdesc[] = {
		"DSR",
		"DTR",
		"RTS",
		"CTS",
		"DCD",
		"RI."
	};
	for( unsigned b=sp::tiocm_dsr,i=0; b<=sp::tiocm_ri; b<<=1,++i ) {
		if( fl & b )
			printf("%s ", sdesc[i] );
	}
	printf("\n");
}

int libgsm_main( int /*argc*/, const char** /*  argv*/) {
	dbprintf("libgsm test welcome");
	fnd::serialport_unix m_ser( "/dev/ttyS0", 115200 );
	char buf[128] {};
	m_ser.puts("AT\r\n");
	int res = m_ser.get( buf, sizeof buf, 200, 7 );
	dbprintf("read %i = [%s]", res, buf );
	print_flags(m_ser.tiocm_get());
	return 0;
}
