/*
 * =====================================================================================
 *
 *       Filename:  phonebook.hpp
 *
 *    Description: GSM library phonebook implementation 
 *
 *        Version:  1.0
 *        Created:  01.03.2015 19:39:32
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
#include <gsm/phbook_id.hpp>

namespace gsm_modem {

	//! Phonebook entry
	struct phbook_entry {
		char name[21] {};	//! Phonebook name
		char phone[15] {};	//! Phone number
	};


	//! forward declaration
	class device;
	class at_parser;

	//! Class which represent device phonebook
	class phonebook {
	public:
		
		//! Phonebook device class
		explicit phonebook( device& owner )
			: m_dev( owner ) {

		}
		/** Select phonebook return error code or
		 *  phonebook identifer
		 *  @param[in] Phonebook id
		 *  @param[out] Error code
		 */
		int select_book( const phbook_id& id );

		/** Get phonebook indentifiers and return its representation
		 * @param[out] ids Output identifer
		 */
		int get_phonebooks_identifiers( phbook_id& ids );

		/** Read entry from the phone book
		 * @param[in] input index
		 * @param[out] filled structure
		 * @return Error code */
		int read_entry( int index, phbook_entry& entry ); 
		
		/** Write entry at selected position 
		 * @param[in] index Input index if -1 write to first empty index
		 * @param[in] entry Phonebook entry
		 * @return Error code 
		 */
	    int write_entry( int index, const phbook_entry& entry ) {
			return write_or_delete_entry( index, &entry );
		}
	
		/** Find phonebook entry by name return number 
		 * @param[in,out] Entry for find
		 * @return index or error code
		 */
		int find_entry( phbook_entry& entry );

		/** Delete phonebook entry at selected index 
		 * @param[in] Index to delete
		 * @return error code of index 
		 */
		int delete_entry( int index ) {
			return write_or_delete_entry( index, nullptr );	
		}
		
		/** Find first phonebook empty entry 
			@return error or index found
		*/
		int find_empty_entry();
	private:
		//! Parse phonebook entry
		int parse_phonebook_entry( char* buf, phbook_entry& entry );

		//! Internal version write or delete entry
		int write_or_delete_entry( int index, const phbook_entry* phb );

		//! Get AT parser
		at_parser& at();
	private:
		device& m_dev;	//! Owner device class
		unsigned m_curr_book {};	//Current phonebook
	};
}
