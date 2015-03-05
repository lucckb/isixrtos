/*
 * =====================================================================================
 *
 *       Filename:  sms_store.hpp
 *
 *    Description:  SMS store class
 *
 *        Version:  1.0
 *        Created:  05.03.2015 17:48:58
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
#include <gsm/containers.hpp>
#include <gsm/stringbit_id.hpp>
#include <array>

namespace gsm_modem {

	//! Forward declaration
	class device;
	class at_parser;
	
	// sms store ids
	class smsmem_id : public detail::stringbit_id {
		static constexpr const char* const smsstorenames[] = {
			"SM", "ME", "SR", "BM", nullptr
		};
	public:
		//! SMS store identifiers
		enum sms_store_id_ {
			sim = 1 << 0,	//! Simcard
			me =  1 << 1, 	//! Me phonebook
			sr =  1 << 2, 	//! Status report message storage
			bm =  1 << 3 	//! CBM message storage
		};
		static constexpr auto last_bit_no = 3;
		//! Constructor
		smsmem_id( unsigned bb = 0 )
			: stringbit_id( bb ) {

			}
		//Set bits using name
		void set_bit( const char *name ) {
			stringbit_id::set_bit( name, smsstorenames );
		}
		//Get name 
		const char* get_name(bool limit=false) const {
			return stringbit_id::get_name( smsstorenames, last_bit_no, limit?me:0 );
		}

		//! Has sim
		bool has_sim() const {
			return m_bits&sim;
		}
		//! Has ME
		bool has_me() const {
			return m_bits&me;
		}
		//! Has Sr
		bool has_sr() const {
			return m_bits&sr;
		}
		//! Has ME
		bool has_bm() const {
			return m_bits&bm;
		}
	};

	//! Sms mem array
	using smsmem_array = std::array< smsmem_id,3>;

	//! Class for representing SMS store
	class sms_store {
	public:
		//! Sms store device class
		explicit sms_store( device& owner ) 
			: m_dev( owner ) {

			}

		/** Set sms store phonebook name
		 * @param[in] id SMS store name 
		 * @return error code
		 */
		int select_store( const smsmem_id& id );
		
		/** Get sms store indentifiers 
			@param[in] id Store identifiers bitflag class
			@return error code for storage
		*/
		int get_store_identifiers( smsmem_id& id );
	private:
		//Get at parser
		at_parser& at();
	private:
		device& m_dev;	//! Device owner
		unsigned m_store_flags {};//! Stored elem
	};
}
