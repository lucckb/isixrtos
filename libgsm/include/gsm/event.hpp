/*
 * =====================================================================================
 *
 *       Filename:  gsm_event.hpp
 *
 *    Description:  GSM event dispatcher class
 *
 *        Version:  1.0
 *        Created:  25.02.2015 19:51:30
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Lucjan Bryndza (LB), lucck(at)boff(dot)pl
 *   Organization:  BoFF
 *
 * =====================================================================================
 */

#pragma once

#include <gsm/datadefs.hpp>

namespace gsm_modem {
	class sms_store;
	class smsmem_id;
	class at_parser;
	class event {
	public:
		//Noncopyable
		event& operator=(event&) = delete;
		event(event&) = delete;
		//Default constructor
		event( sms_store& dev );
		//Callback dispatcher called from AT parser
		void dispatch( at_parser& , char* str );
protected:
		//Callback functions
		virtual void sms_reception( const sms_type_ptr_t /*sms*/);
		//SMS reception indication
		virtual void sms_reception_indication(const smsmem_id& /*storage */,int /*index*/);
		// number, subaddr, alpha
		virtual void caller_line_id( const char* /*number*/, const char* /*alpha*/);
		//Ring indicatior
		virtual void ring_indication() {
		}
		//No answer
		virtual void no_answer() {
		}
	private:
		sms_store& m_store;
	};
}
