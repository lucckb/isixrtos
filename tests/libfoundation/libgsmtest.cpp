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

#include <foundation/sys/dbglog.h>
#include <serialport_unix.hpp>
#include <gsm/device.hpp>
#include <gsm/sms_message.hpp>
#include <gsm/sms_store.hpp>
#include <cstring>
#include <tuple>

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

#define TEST_ERROR( x ) do { \
	if( x < 0 ) { \
		printf("Execute failed at line %i %s", __LINE__, #x ); \
		exit(-1); } \
	} while(0)

int libgsm_main( int /*argc*/, const char** /*  argv*/)
{
	int ret;
	fnd::serialport_unix m_ser( "/dev/ttyS0", 115200 );
	devctl m_ctl;
	using cap = gsm_modem::device::cap;
	gsm_modem::device modem( m_ser, m_ctl, cap::hw_flow );
	if(1) {
		TEST_ERROR( modem.enable( true ) );
		modem.register_to_network( "1234" );
	}
	if(0) {
		gsm_modem::oper_info oi;
		TEST_ERROR( modem.get_current_op_info(oi) );
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
	if(0) {
		//Write entry test
		std::strcpy( entry.name, "piedzioch" );
		std::strcpy( entry.phone, "+486665554444" );
		dbprintf("writeentry=%i", modem.get_phonebook().write_entry( 15, entry ) );
		//Delete entry
		dbprintf("deletentry=%i", modem.get_phonebook().delete_entry( 15 ) );
	}
	if(1) {
		TEST_ERROR( modem.set_sms_routing_to_ta( true, false, true, false ) );
	}
	if(0) {
		char msg[161] {};
		memset(msg, 'z', 160 );
		gsm_modem::sms_submit sms( "+48660428360", "Proba SMSA");
		sms.report_request( true );
		dbprintf("SMSerr=%i", modem.send_sms( sms ) );
	}
	if(1) {
		auto& ssms = modem.get_sms_store();
		gsm_modem::smsmem_id ids;
		dbprintf( "store flags %i", (ret=ssms.get_store_identifiers(ids)) );
		TEST_ERROR(ret);
		dbprintf( "select store %i", (ret=ssms.select_store(ids)) );
		TEST_ERROR(ret);
		dbprintf( "number of entries %lu", ssms.capacity() );
	}
	if(0) {
		dbprintf( "Delete entry %i", modem.get_phonebook().find_empty_entry() );
	}
	if(0) {
		int err;
		gsm_modem::sms_type_ptr_t sms;
		std::tie( err, sms ) = modem.get_sms_store().read_entry(3);
		dbprintf( "Read entry stat %i %p", err, sms );
		TEST_ERROR( ret );
		if( err >= 0 && sms->type() == gsm_modem::sms::t_deliver ) {
			const auto it = dynamic_cast<gsm_modem::sms_deliver*>( sms );
			dbprintf("TSTAMP %s ORIGIN_ADDR %s PID %i REPORT_INDIC %i",
				it->service_tstamp(), it->origin_address(), it->pid(), it->report_indication() );
			dbprintf("Content %s", it->message() );
					
		} else if( err >=0 && sms->type() == gsm_modem::sms::t_submit ) {
			dbprintf("Submit message type");
			const auto it = dynamic_cast<gsm_modem::sms_submit*>( sms );
			dbprintf("VALIDITY %i REP_REQ %i FL_MSG %i DEST_ADDR %s, PID %i",
				it->validity_period(), it->report_request(), it->flash_message(),
				it->dest_address() , it->pid() );
			dbprintf("Content %s", it->message() );
		}
	}
	if( 0 ) {
		dbprintf("Delete entry %i", modem.get_sms_store().erase_entry( 2 ) );
	}
	if( 1 ) {
		gsm_modem::imei_number imei;
		gsm_modem::imsi_number imsi;
		TEST_ERROR( modem.get_imei( imei ) );
		dbprintf("IMEI=%s", imei.value );
		TEST_ERROR( modem.get_imsi( imsi ) );
		dbprintf("IMSI=%s", imsi.value );
	}
	if( 1 ) {
		for(;;) {
			dbprintf("Wait=%i", modem.wait_event( 5000 ) );
		}
	}
	return 0;
}
