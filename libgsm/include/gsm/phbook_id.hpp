/*
 * =====================================================================================
 *
 *       Filename:  phbook_id.hpp
 *
 *    Description:  Phonebook identification class
 *
 *        Version:  1.0
 *        Created:  01.03.2015 21:44:13
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Lucjan Bryndza (LB), lucck(at)boff(dot)pl
 *   Organization:  BoFF
 *
 * =====================================================================================
 */
#pragma once
#include <gsm/stringbit_id.hpp>

namespace gsm_modem {
	//!Phonebook text identifier
	class phbook_id : public detail::stringbit_id {
		static constexpr const char* const bookids[] = {
			"SM", "FD", "ON", "EN", "LD", "MC", "ME", "MT", "RC", "SN", nullptr 
		};
	public:
		enum phonebook_id_ {
			sim = 		1<<0,	//! SM Sim phonebook
			fdn = 		1<<1,	//! FD SIM fixed dialing numbers
			msisdn = 	1<<2,	//! ON Sim own numbers MSISDN
			ecc = 		1<<3,	//! EN ECC emberency call codes
			lnd = 		1<<4,	//! LD Last Number dialed
			mc = 		1<<5,	//! MC misssed unanswered received call list
			me =		1<<6,	//! ME phonebook
			mesim = 	1<<7,	//! MT combined ME and sim phonebook
			rc = 		1<<8,	//! RC received call list
			sn = 		1<<9	//! SN SDN service dialing numbers (Sim special service num)
		};
		static constexpr auto last_bit_no = 9L;
		phbook_id( unsigned bb = 0 ) 
			: stringbit_id( bb )
		{};
		/** Get the name of the bit
		 * only one bit must be set
		 * in other case it return error*/
		const char* get_name() const {
			return stringbit_id::get_name( bookids, last_bit_no );
		}

		/** Set bit related to phonebook name
		 * @param[in] phonebook name
		 */
		void set_bit( const char* name ) {
			stringbit_id::set_bit( name, bookids );
		}

		bool has_sim() const {
			return m_bits&sim;
		}
		bool has_fdn() const {
			return m_bits&fdn;
		}
		bool has_msisdn() const {
			return m_bits&msisdn;
		}
		bool has_ecc() const {
			return m_bits&ecc;
		}
		bool has_lnd() const {
			return m_bits&lnd;
		}
		bool has_mc() const {
			return m_bits&mc;
		}
		bool has_me() const {
			return m_bits&me;
		}
		bool has_mesim() const {
			return m_bits&mesim;
		}
		bool has_rc() const {
			return m_bits&rc;
		}
		bool has_sn() const {
			return m_bits&sn;
		}
	};
}
