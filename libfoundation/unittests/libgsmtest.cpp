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
#include <gsm/gsm_device.hpp>
#include <gsm/sms_message.hpp>
#include <cstring>

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

#if 0
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
#endif

class devctl : public gsm_modem::hw_control  {

	//! Enable or disable hardware modem device
	virtual void power_control( bool enable ) {
		dbprintf("Power control state change %i", enable );
	}
	//! Hardware reset device
	virtual void reset() {
		dbprintf("HW reset ");
	}
};


int libgsm_main( int /*argc*/, const char** /*  argv*/)
{
	fnd::serialport_unix m_ser( "/dev/ttyS0", 115200 );
	devctl m_ctl;
	gsm_modem::device modem( m_ser, m_ctl );
	if(1) {
		modem.enable( true );
		modem.register_to_network( "1234" );
	}
	if(0) {
		gsm_modem::oper_info oi;
		modem.get_current_op_info(oi);
		dbprintf("status %i mode %i short [%s] long [%s] nn %i", 
				(int)oi.status, (int)oi.mode, oi.desc_short, oi.desc_long, oi.numeric_name );
	}
	if(0) {
	//Get registration status
	dbprintf( "reg status %i", modem.get_registration_status() ) ;
	dbprintf( "signal strength %i", modem.get_signal_strength() );
	}
	gsm_modem::phbook_id ids;
	modem.get_phonebook().get_phonebooks_identifiers( ids );
		dbprintf( "phonebook ids %08x", ids.bits() );
	ids.mask( gsm_modem::phbook_id::sim );
	modem.get_phonebook().select_book( ids );
	gsm_modem::phbook_entry entry;
	if(0) {
		dbprintf("readpbres=%i", modem.get_phonebook().read_entry( 92, entry ) );
		dbprintf( "pb_name = [%s] pb_phone = [%s]", entry.name, entry.phone );
		entry.phone[0] = 0;
		dbprintf("findres=%i", modem.get_phonebook().find_entry( entry ) );
		dbprintf( "pb_name = [%s] pb_phone = [%s]", entry.name, entry.phone );
	}
	//Write entry test
	std::strcpy( entry.name, "piedzioch" );
	std::strcpy( entry.phone, "+486665554444" );
	dbprintf("writeentry=%i", modem.get_phonebook().write_entry( 15, entry ) );
	//Delete entry
	dbprintf("deletentry=%i", modem.get_phonebook().delete_entry( 15 ) );
	return 0;
}
