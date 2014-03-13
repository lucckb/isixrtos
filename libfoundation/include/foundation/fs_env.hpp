/*
 * =====================================================================================
 *
 *       Filename:  fs_env.hpp
 *
 *    Description:  Filesystem envinronment implementation
 *
 *        Version:  1.0
 *        Created:  03/07/2014 18:59:52
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Lucjan Bryndza (lb)
 *   Organization:
 *
 * =====================================================================================
 */
#pragma once
#include <foundation/iflash_mem.hpp>
/* ------------------------------------------------------------------ */
/**  Filesystem implementation */
namespace fnd {
namespace filesystem {
/* ------------------------------------------------------------------ */
namespace detail {
	struct fnode_0;
}
/* ------------------------------------------------------------------ */ 
//! Env FS class implementation
class fs_env {
	enum error_internal {
		err_hdr_not_found = 3,
		err_hdr_second = 2,
		err_hdr_first = 1
	};
public:
	//! Error codes
	enum error {
		err_success = 0,
		err_invalid_id = -8192,		/** Identifier not found  */
		err_range_id  = -8193,		/** Ivalid range */
		err_fs_full =  -8195,		/** File system full */
		err_fs_fmt = -8196,			/**  Filesystem format error reformat required */
		err_internal = -8197		/** Internal error (critical) */
	};
	/** Environment fs constructor 
	 * @param[in] flash_mem Flash memory controller
	 * @param[in] n_page number of pages used 0 all from beginning
       					 negative n from end */
	fs_env( iflash_mem& flash_mem , unsigned n_pg = 0 ) 
		: m_flash( flash_mem ), m_npages( calc_npages(n_pg) ),
		 m_pg_base( calc_page(n_pg) ), m_pg_alt( calc_alt_page(n_pg) )
	{}
	/**  Store data in non volatile memory
	 *   @param[in] env_id Environment identifier
	 *   @param[in] buf Pointer to buffer for store data
	 *   @param[in] len Buffer length
	 *   @return Error code on failed
	 */
	int set( unsigned env_id, const void* buf, size_t buf_len );

	/**  Get data from non volatile memory
	 *   @param[in] env_id Environment identifier
	 *   @param[in] buf Pointer to buffer for store data
	 *   @param[in] len Buffer length
	 *   @return Error code on failed
	 */
	int get( unsigned env_id, void* buf, size_t buf_len );

	/** Unset environment variable
	 *   @param[in] env_id Environment identifier
	 *   @return Error code on failed
	 */
	int unset( unsigned env_id );

	/** Format the whole storage memory on demand 
	 *  @return error status code
	 */
	int format( );
private:
	//! Get first page address
	iflash_mem::paddr_t calc_page( int n_pg ) const;
	//! Calculate alternate page
	iflash_mem::paddr_t calc_alt_page( int n_pg ) const {
		if( !(m_flash.get_capabilities()&iflash_mem::cap_pg_no_erase) ) {
			return calc_page( n_pg ) + m_npages;
		} else {
			return calc_page( n_pg );
		}
	}
	//! Can random erase
	bool can_random_access() const {
		return m_pg_base==m_pg_alt;
	}
	//! Init fs
	int init_fs( unsigned& pg, unsigned& csize );
	//! Check if storage memory is formated
	int find_valid_page( unsigned& clust_size );
	//! Get pages avail
	iflash_mem::paddr_t calc_npages( int n_pg ) const;
	//! Find first entry by ID
	int find_first( unsigned id, unsigned pg, unsigned cs, detail::fnode_0* node = nullptr );
	//! Flash read cluster 
	int flash_read( unsigned fpg, unsigned clust, unsigned csize, void *buf, size_t len );
	//! Flash read cluster 
	int flash_write( unsigned fpg, unsigned clust, unsigned csize, const void *buf, size_t len );
	//! Reclaim eeprom memory
	int reclaim_random();
	//! Reclaim flash memory
	int reclaim_nonrandom();
	//! Reclaim the memory
	int reclaim() {
		if( can_random_access() ) {
			return reclaim_random();
		} else {
			return reclaim_nonrandom();
		}
	}
	//! Find free node
	int find_free_cluster( unsigned pg, unsigned csize, unsigned sclust );
	//! Delete active inode chain
	int delete_chain( unsigned pg, unsigned csize, unsigned cclu );
	//! Check fre chain
	int check_chains( unsigned pg, unsigned csize, unsigned rclu );
	//!Erase all eeprom
	int erase_all_random( unsigned pg );
	//Erase all flashmem
	int erase_all_nonrandom( unsigned pg );
	//! Erase all pages beginning from startup pg
	int erase_all_pages( unsigned pg ) {
		if( can_random_access() ) {
			return erase_all_random(pg);
		} else {
			return erase_all_nonrandom(pg);
		}
	}
	//! Get cluster size
	unsigned get_clust_size() const {
		unsigned avail_mem = m_npages * m_flash.page_size();
		return (avail_mem>8192)?(64):(32);
	}
private:
	iflash_mem& m_flash;	       			//! Flash memory private data
	const unsigned m_npages;				//! Number of pages
	const iflash_mem::paddr_t m_pg_base;	//! Base page
	const iflash_mem::paddr_t m_pg_alt;		//! Alternate page
};
/* ------------------------------------------------------------------ */ 
}
}
