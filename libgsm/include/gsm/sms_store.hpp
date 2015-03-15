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
#include <gsm/sms_message.hpp>
#include <array>

namespace gsm_modem {

	//! Forward declaration
	class device;
	class at_parser;
	//! NOTE: It can be easy replaced with shared ptr if non placement new impl
	using sms_store_result_t = std::pair<int,sms_type_ptr_t>;

	struct smsstore_message_type {
	enum smsstore_message_type_ {
		rec_unread = 0,
		rec_read = 1,
		sto_unsent = 2,
		sto_sent = 3,
		all = 4
	}; };

	// sms store ids
	class smsmem_id : public detail::stringbit_id {
		static constexpr const char* const smsstorenames[] = {
			"SM", "ME", "SR", "BM", nullptr
		};
	public:
		//! SMS store identifiers
		enum sms_store_id {
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

	

	//! Class for representing SMS store
	class sms_store {
	public:
		//! Copy constructor and assign to delete
		sms_store& operator=(sms_store&)=delete;
		sms_store( sms_store& ) = delete;

		//! Sms store device class
		explicit sms_store( device& owner ) 
			: m_dev( owner ) 
		{
		}
		//! Delete flags
		enum class del {
			index = 0,
			read = 1,
			read_sent = 2,
			read_sent_unsent = 3,
			all = 4
		};
		/** Set sms store phonebook name
		 * @param[in] id SMS store name 
		 * @return Number of entries in selected book
		 */
		int select_store( const smsmem_id& id );
		
		/** Get sms store indentifiers 
			@param[in] id Store identifiers bitflag class
			@return error code for storage
		*/
		int get_store_identifiers( smsmem_id& id );

		/** Read message from phonebook 
		 * @param[in] Input index of the message
		 * @return Pointer to read SMS with error code
		 */
		sms_store_result_t read_entry( int index ) ;
		
		/** Erase entry using selected index name
		 * @param[in] index Index of message to delete
		 * @param[in] flags Flags of deleted messages
		 * @return error code of erase message
		 * */
		int erase_entry( int index, del flags = del::index );

		/** Return number of entries in the store
		 */
		size_t capacity() const {
			return m_total_entries;
		}
		// Interators
	private:
		//Get at parser
		at_parser& at();
	private:
		device& m_dev;				//! Device owner
		unsigned m_store_flags {};	//! Stored elem
		short m_total_entries {};
		//! Placement new message creating
		char m_storage alignas(8) [sms_placement_size]; //! Used placement new don't use new/del
		template<typename T, typename... A> T* create_message( A&&... args )
		{
			if( m_message ) {
				m_message->~sms();
			}
			T* ptr = new(m_storage)T(args...);
			m_message = ptr;
			return ptr;
		}
		sms *m_message {};		   //! SMS message definition
	};
}
