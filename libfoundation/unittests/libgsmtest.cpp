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
#include <gsm/at_parser.hpp>

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
#if 0
	char buf[128] {};
	m_ser.puts("AT\r\n");
	int res = m_ser.get( buf, sizeof buf, 200, 7 );
	dbprintf("read %i = [%s]", res, buf );
	print_flags(m_ser.tiocm_get());
#endif
	//Check AT parser
	gsm_modem::at_parser at( m_ser );
	char* r;
#if 0
	r = at.chat("+CMEE=1", nullptr, true, true );
	dbprintf( "ret [%s] err [%i]", r, at.error() );
	r = at.chat("+CMGF=1");
	dbprintf( "ret [%s] err [%i]", r, at.error() );
	r = at.chat("+CSMS?","+CSMS:");
	dbprintf( "ret [%s] err [%i]", r, at.error() );
#endif
	r = at.chat("+CPIN?","+CPIN:");
	dbprintf( "ret [%s] err [%i]", r, at.error() );
	
	gsm_modem::resp_vec vec;
	int ri = at.chatv(vec, "+CPBR=1,250", "+CPBR:", false );
	dbprintf( "chatv err=%i len %u", ri, vec.size() );
	if( !ri ) {
		for( const auto& v : vec ) {
			dbprintf("entry->[%s]", v );
		}
	}
	return 0;
}
