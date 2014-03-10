/*
 * =====================================================================================
 *
 *       Filename:  fs_env.hpp
 *
 *    Description:  Filesystem envinronment implementation
 *
 *        Version:  1.0
 *        Created:  03/07/2014 12:59:52 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Lucjan Bryndza (lb), Lucjan_B1@verifone.com
 *   Organization:  VERIFONE
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
//! Env FS class implementation
class fs_env {
public:
	//! Error codes
	enum error {
		err_no_clusters = 4,
		err_hdr_not_found = 3,
		err_hdr_second = 2,
		err_hdr_first = 1,
		err_success = 0,
		err_invalid_id = -8192,
		err_no_id	  =  -8193,
		err_range_id  = -8194,
		err_fs_full =  -8195,
	};
	/** Environment fs constructor 
	 * @param[in] flash_mem Flash memory controller
	 * @param[in] n_page number of pages used 0 all from beginning
       					 negative n from end */
	fs_env( iflash_mem& flash_mem , unsigned n_pg = 0 ) 
		: m_flash( flash_mem ), m_pg_base( calc_page(n_pg) ),
		  m_pg_alt( calc_alt_page(n_pg) ), m_npages( calc_npages(n_pg) )
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
	int get( unsigned env_id, const void* buf, size_t buf_len );

	/** Unset environment variable
	 *   @param[in] env_id Environment identifier
	 *   @return Error code on failed
	 */
	int unset( unsigned env_id );

private:
	//! Get first page address
	iflash_mem::paddr_t calc_page( int n_pg ) const;
	//! Calculate alternate page
	iflash_mem::paddr_t calc_alt_page( int n_pg ) const {
		if( !(m_flash.get_capabilities()&iflash_mem::cap_pg_no_erase) )
			return calc_page( n_pg ) + n_pg / 2;
		else 
			return calc_page( n_pg );
	}
	//! Can random erase
	bool can_random_access() const {
		return m_pg_base==m_pg_alt;
	}
	//! Init fs
	int init_fs( unsigned& pg, unsigned& csize );
	//! Check if storage memory is formated
	int find_valid_page( unsigned& clust_size );
	//! Format flash and return cluster size
	int format( unsigned& clust_size );
	//! Get pages avail
	iflash_mem::paddr_t calc_npages( int n_pg ) const;
	//! Find first entry by ID
	int find_first( unsigned id, unsigned pg, unsigned cs );
	//! Flash read cluster 
	int flash_read( unsigned fpg, unsigned clust, unsigned csize, void *buf, size_t len );
	//! Flash read cluster 
	int flash_write( unsigned fpg, unsigned clust, unsigned csize, const void *buf, size_t len );
	//! Reclaim the memory
	int reclaim();
	//! Find free node
	int find_free_cluster( unsigned pg, unsigned csize, unsigned sclust );
	//! Delete active inode chain
	int delete_chain( unsigned pg, unsigned csize, unsigned cclu );
private:
	iflash_mem& m_flash;	       			//! Flash memory private data
	const iflash_mem::paddr_t m_pg_base;	//! Base page
	const iflash_mem::paddr_t m_pg_alt;		//! Alternate page
	const unsigned m_npages;				//! Number of pages
};
/* ------------------------------------------------------------------ */ 
}
}
