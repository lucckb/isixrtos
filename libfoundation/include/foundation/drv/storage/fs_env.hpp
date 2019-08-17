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
#include <foundation/drv/storage/iflash_mem.hpp>
#include <foundation/algo/lrucache.hpp>
 
/**  Filesystem implementation */
namespace fnd {
namespace filesystem {
 
namespace detail {
	struct fnode_0;
}
  
//! Env FS class implementation
class fs_env 
{
	enum error_internal {
		err_hdr_not_found = 3,
		err_hdr_second = 2,
		err_hdr_first = 1
	};
	static constexpr auto c_first_cluster = 1U;
	static constexpr auto c_lru_size = 16;
public:
	//! Prevent copy
	fs_env( const fs_env& ) = delete;
	fs_env& operator=( const fs_env& ) = delete;
	//! Error codes
	enum error {
		err_success = 0,
		err_invalid_id = -8192,		/** Identifier not found  */
		err_range_id  = -8193,		/** Ivalid range */
		err_fs_full =  -8195,		/** File system full */
		err_fs_fmt = -8196,			/**  Filesystem format error reformat required */
		err_internal = -8197,		/** Internal error (critical) */
		err_buf_ovrflow = -8198	    /** Buffer is to small to handle variable */
	};
	/** Environment fs constructor 
	 * @param[in] flash_mem Flash memory controller
	 * @param[in] n_page number of pages used 0 all from beginning
       					 negative n from end */
	fs_env( iflash_mem& flash_mem , bool wear_leveling = true, unsigned n_pg = 0 ) 
		: m_flash( flash_mem ), m_npages( calc_npages(n_pg) ),
		 m_pg_base( calc_page(n_pg) ), m_pg_alt( calc_alt_page(n_pg) ),
		 m_wear_leveling( wear_leveling | !can_random_access() ),
		 m_lru( c_lru_size )
	{}
	/**  Store data in non volatile memory
	 *   @param[in] env_id Environment identifier
	 *   @param[in] buf Pointer to buffer for store data
	 *   @param[in] len Buffer length
	 *   @return Error code
	 */
	int set( unsigned env_id, const void* buf, size_t buf_len );

	/**  Get data from non volatile memory
	 *   @param[in] env_id Environment identifier
	 *   @param[in] buf Pointer to buffer for store data
	 *   @param[in] len Buffer length
	 *   @return Return number of bytes read or negative error code
	 */
	int get( unsigned env_id, void* buf, size_t buf_len );

	/** Unset environment variable
	 *   @param[in] env_id Environment identifier
	 *   @return Error code
	 */
	int unset( unsigned env_id );

	/** Format the whole storage memory on demand 
	 *  @return Error code
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
	int init_fs();
	//! Check if storage memory is formated
	int find_valid_page( unsigned& clust_size );
	//! Get pages avail
	iflash_mem::paddr_t calc_npages( int n_pg ) const;
	//! Find first entry by ID
	int find_first( unsigned id, detail::fnode_0* node = nullptr );
	//! Find next cluster
	int find_next_cluster( unsigned cluster );
	//! Flash read cluster 
	int flash_read( unsigned fpg, unsigned clust, unsigned csize, void *buf, size_t len );
	//! Flash read cluster 
	int flash_write( unsigned fpg, unsigned clust, unsigned csize, const void *buf, size_t len );
	//! Reclaim eeprom memory
	int reclaim_random();
	//! Reclaim flash memory
	int reclaim_nonrandom();
	//! Reclaim the memory
	int reclaim() 
	{
		m_lru.clear();
		m_last_free_clust = c_first_cluster;
		if( can_random_access() ) {
			return reclaim_random();
		} else {
			return reclaim_nonrandom();
		}
	}
	//! Calculate required cluster for buffer usage
	size_t buf_len_to_n_clust( size_t buf_len );
	//! Find free node
	int find_free_cluster( unsigned sclust );
	//! Delete active inode chain
	int delete_chain( unsigned cclu );
	//! Check fre chain
	int check_chains( unsigned rclu );
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
	unsigned get_initial_clust_size() const {
		unsigned avail_mem = m_npages * m_flash.page_size();
		return (avail_mem>8192)?(64):(32);
	}
	//! Get current used page
	unsigned get_page() const {
		return m_alt_page_in_use?m_pg_alt:m_pg_base;
	}
	//! Get cluster size
	unsigned get_clust_size() const {
		return m_clust_size;
	}
	//! Get first cluster
	unsigned get_first_cluster() const {
		return m_wear_leveling?m_last_free_clust:c_first_cluster;
	}
	//! Set chain without wear leveling
	int write_non_existing( unsigned env_id, const void* buf, size_t buf_len, 
			unsigned short& lru_cache_elem );
	//! Set chain without wear leveling
	int write_existing( unsigned env_id, const void* buf, size_t buf_len );
private:
	iflash_mem& m_flash;	       			//! Flash memory private data
	const unsigned m_npages;				//! Number of pages
	const iflash_mem::paddr_t m_pg_base;	//! Base page
	const iflash_mem::paddr_t m_pg_alt;		//! Alternate page
	unsigned m_clust_size {};				//! Current cluster size
	bool m_alt_page_in_use {};				//! Use second header
	const bool m_wear_leveling {};		//! Disable wear leveling
	unsigned m_last_free_clust { c_first_cluster };			//! Last free cluster
	fnd::lru_cache<unsigned, unsigned short> m_lru;	//! LRU cache
};
  
}
}
