/*
 * =====================================================================================
 *
 *       Filename:  env_fs.cpp
 *
 *    Description:  Environment FS implementation
 *
 *        Version:  1.0
 *        Created:  08.03.2014 18:25:34
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Lucjan Bryndza (LB), lucck(at)boff(dot)pl
 *   Organization:  BoFF
 *
 * =====================================================================================
 */

#include <foundation/fs_env.hpp>
#include <cstdint>
#include <foundation/dbglog.h>
#include <limits>
#include <cstring>
/* ------------------------------------------------------------------ */ 
namespace fnd {
namespace filesystem {
/* ------------------------------------------------------------------ */
namespace {
/* ------------------------------------------------------------------ */ 
//! Page header
struct pg_hdr {
	static constexpr uint32_t id_empty = 0xffff;
	static constexpr uint32_t id_valid = 0xFA197908;
	bool ok() const {
		return id == id_valid;
	}
	uint32_t id;			//! Partition ID
	uint16_t clust_len;		//! Cluster size
} __attribute__((packed));
/* ------------------------------------------------------------------ */
//! Node defs
static constexpr uint16_t node_unused = 0x7FFF;
static constexpr uint16_t node_dirty = 0;
static constexpr uint16_t node_end = node_unused - 1;

//! Node type 0
struct fnode_0 {
	uint16_t type : 1;
	uint16_t id_next: 15;
	uint16_t next;
	uint16_t len;
	uint16_t crc;
	uint8_t data[];
} __attribute__((packed));
//! Node type 1
struct fnode_1 {
	uint16_t type : 1;
	uint16_t next: 15;
	uint8_t data[];
} __attribute__((packed));
/* ------------------------------------------------------------------ */ 
//! Short CRC8 implementation
class crc16 {
	static constexpr uint16_t lo_tbl[] = {
        0x00, 0x10, 0x20, 0x30, 0x40, 0x50, 0x60, 0x70,
        0x81, 0x91, 0xA1, 0xB1, 0xC1, 0xD1, 0xE1, 0xF1
	};
	static constexpr uint16_t hi_tbl[] = {
        0x00, 0x21, 0x42, 0x63, 0x84, 0xA5, 0xC6, 0xE7,
        0x08, 0x29, 0x4A, 0x6B, 0x8C, 0xAD, 0xCE, 0xEF
	};
private:
	void update4( uint8_t value ) {
		uint8_t t;
		// Step one, extract the Most significant 4 bits of the CRC register
		t = hi_acc >> 4;

		// XOR in the Message Data into the extracted bits
		t = t ^ value;

		// Shift the CRC Register left 4 bits
		hi_acc = (hi_acc << 4) | (lo_acc >> 4);
		lo_acc = lo_acc << 4;

		// Do the table lookups and XOR the result into the CRC Tables
		hi_acc = hi_acc ^ hi_tbl[t];
		lo_acc = lo_acc ^ lo_tbl[t];
	}
public:
	void operator()( uint8_t val ) {
		update4( val >> 4 );
		update4( val & 0x0F );
	}
	void operator()( const void *buf, size_t len ) {
		for( size_t s = 0; s < len; ++s ) {
			auto p = reinterpret_cast<const uint8_t*>(buf);
			(*this)( p[s] );
		}
	}
	uint16_t operator()() const {
		return (uint16_t(hi_acc)<<8)|uint16_t(lo_acc);
	}
private:
	uint8_t lo_acc { 0xFF };
	uint8_t hi_acc { 0xFF };
};
	//Declare constexpr table
	constexpr uint16_t crc16::lo_tbl[];
	constexpr uint16_t crc16::hi_tbl[];
/* ------------------------------------------------------------------ */
} // ns
/* ------------------------------------------------------------------ */
/**  Store data in non volatile memory
	*   @param[in] env_id Environment identifier
	*   @param[in] buf Pointer to buffer for store data
	*   @param[in] len Buffer length
	*   @return Error code on failed
	*/
int fs_env::set( unsigned env_id, const void* buf, size_t buf_len )
{
	unsigned csize, pg;
	++env_id;
	if( env_id >= node_end ) {
		dbprintf("Invalid input identifier");
		return err_invalid_id;
	}
	if( buf_len > std::numeric_limits<decltype(fnode_0::id_next)>::max() ) {
		return err_range_id;
	}
	auto ret = init_fs( pg, csize );
	if( ret >= 0 ) {
		ret = find_first( env_id, pg, csize );
		if( ret == err_invalid_id ) {
			ret = err_success;
		} else {
			ret = delete_chain( pg, csize, ret );
		}
	}
	if( ret == 0 ) {
		const unsigned nclu = buf_len/csize + (buf_len%csize?(1):(0));
		unsigned fnd_clu = 0;
		for( int retries = 0; retries < 2; ++retries )
		{
			for( unsigned c = 0, fc =0; c<nclu; ++c ) {
				fc = find_free_cluster( pg, csize, fc + 1 );
				if( fc > 0 ) {
					if( ++fnd_clu == nclu ) {
						break;
					}
				} else if( fc == err_no_clusters )  {
					ret = reclaim();
					if( ! ret ) 
						ret = init_fs( pg, csize );
				    fnd_clu = 0;
					break;
		
				} else {
					ret = fc;
					break;
				}
				if( ret ) break;
				if( fnd_clu == nclu ) break;
			}
			if( fnd_clu == nclu ) {
				break;
			}
		}
		if( fnd_clu < nclu ) {
			ret  = err_fs_full;
		}
		if( !ret ) {
			crc16 crcc;
			crcc( buf, buf_len );
			auto fc1 = find_free_cluster( pg, csize, 1 );
			int fc2;
			for( unsigned c=0; c<nclu; ++c ) {
				fc2 = find_free_cluster( pg, csize, fc1 + 1 );
				char ibuf[ csize ];
				unsigned twlen;
				if( c == 0 ) {
					auto hdr = reinterpret_cast<fnode_0*>( ibuf );
					hdr->id_next = env_id;
					hdr->len = buf_len;
					hdr->crc = crcc();
					hdr->type = 0;
					hdr->next = (buf_len<=(csize-sizeof(fnode_0)))?(node_end):(fc2);
					const int wlen = (buf_len<=(csize-sizeof(fnode_0)))?(buf_len):
								 	 (csize-sizeof(fnode_0));
					std::memcpy( hdr->data, buf, wlen );
					buf = reinterpret_cast<const char*>(buf) + wlen;
					buf_len -= wlen;
					twlen = sizeof(fnode_0) + wlen;
				} else {
					auto hdr = reinterpret_cast<fnode_1*>( ibuf );
					hdr->type = 1;
					hdr->next = (buf_len<=(csize-sizeof(fnode_1)))?(node_end):(fc2);
					const int wlen = (buf_len<=(csize-sizeof(fnode_1)))?(buf_len):
								 	 (csize-sizeof(fnode_1));
					std::memcpy( hdr->data, buf, wlen );
					buf = reinterpret_cast<const char*>(buf) + wlen;
					buf_len -= wlen;
					twlen = sizeof(fnode_1) + wlen;
				}
				ret = flash_write( pg, fc1, csize, ibuf, twlen );
				fc1 = fc2;
				if( ret ) break;
			}
		}
	}
	return ret;
}
/* ------------------------------------------------------------------ */
/**  Get data from non volatile memory
	*   @param[in] env_id Environment identifier
	*   @param[in] buf Pointer to buffer for store data
	*   @param[in] len Buffer length
	*   @return Error code on failed
	*/
int fs_env::get( unsigned env_id, const void* buf, size_t buf_len )
{
	++env_id;
	if( env_id >= node_end ) {
		dbprintf("Invalid input identifier");
		return err_invalid_id;
	}
	return 0;
}
/* ------------------------------------------------------------------ */
/** Unset environment variable
	*   @param[in] env_id Environment identifier
	*   @return Error code on failed
	*/
int fs_env::unset( unsigned env_id )
{
	++env_id;
	if( env_id >= node_end ) {
		dbprintf("Invalid input identifier");
		return err_invalid_id;
	}
	unsigned csize, pg;
	auto ret = init_fs( pg, csize );
	if( ret >= 0 ) {
		int ret = find_first( env_id, pg, csize );
		if( ret == err_invalid_id ) {
			ret = err_no_id;
		} else if( ret > 0 ) {
			ret = delete_chain( pg, csize, ret );
		}
	}
	return ret;
}
/* ------------------------------------------------------------------ */ 
//!Unset internal witohout mod
int fs_env::delete_chain( unsigned pg, unsigned csize, unsigned cclu )
{
	int ret;
	//Read node
	for(unsigned pclu ;cclu!=node_end;) {
		fnode_0 node;
		dbprintf("unset() next %i", cclu );
		ret = flash_read( pg, cclu, csize, &node, sizeof node );
		if( ret ) break;
		pclu = cclu;
		if( node.type == 0 ) {
			cclu = node.next;
		} else {
			cclu = node.id_next;
		}
		node.next = node_dirty;
		node.id_next = node_dirty;
		ret = flash_write( pg, pclu, csize, &node, sizeof node );
		if( ret ) break;
	}
	return ret;
}
/* ------------------------------------------------------------------ */
//! Find free node
int fs_env::find_free_cluster( unsigned pg, unsigned csize, unsigned sclust )
{	
	const auto pg_size = m_flash.page_size();
	auto ncs = (m_npages * pg_size)/csize;
	fnode_0 node;
	bool found = false;
	int ret;
	for( unsigned c=sclust; c<ncs; ++c ) {
		ret = flash_read( pg, c, csize, &node, sizeof node );
		if( ret ) break;
		if( node.id_next == node_unused ) {
			found = true; ret = c;
			break;
		}
	}
	if( !ret && !found ) {
		 ret = err_no_clusters;
	}
	return ret;
}
/* ------------------------------------------------------------------ */
//Init fs  cal on every set and get env
int fs_env::init_fs( unsigned& pg, unsigned& csize )
{
	auto ret = find_valid_page(csize);
	if( ret == err_hdr_not_found ) {
		ret = format(csize);
		pg = m_pg_base;
		dbprintf("Page not found format requied CS %i RET %i", csize, ret );
	} else if( ret == err_hdr_first ) {
		pg = m_pg_base;
	} else if( ret == err_hdr_second ) {
		pg = m_pg_alt;
	}
	return ret;
}
/* ------------------------------------------------------------------ */
//! Find first entry by ID
int fs_env::find_first( unsigned id , unsigned pg, unsigned cs )
{
	const auto pg_size = m_flash.page_size();
	auto ncs = (m_npages * pg_size)/cs;
	///dbprintf("find_first() ncs: %i npgs: %i pg_size %i cs: %i", ncs, m_npages, pg_size, cs );
	int ret;
	fnode_0 node;
	bool found = false;
	for( unsigned c=1; c<ncs; ++c ) {
		ret = flash_read( pg, c, cs, &node, sizeof node );
		if( ret ) break;
		if( node.type == 0 && node.id_next == id ) {
			ret = c;
			found = true;
			break;
		}
	}
	if( !ret && !found ) {
		ret = err_invalid_id;
	}
	dbprintf("Find first ret %i", ret );
	return ret;
}
/* ------------------------------------------------------------------ */
//! Format memory headers
int fs_env::format( unsigned& clust_size ) 
{
	unsigned avail_mem = m_npages * m_flash.page_size();
	if( avail_mem > 8192 ) {
		clust_size = 64;
	} else {
		clust_size = 32;
	}
	const pg_hdr hdr { pg_hdr::id_valid, uint16_t(clust_size) };
	auto ret = m_flash.write( m_pg_base, 0, &hdr, sizeof hdr );
	if( !ret && !can_random_access() ) {
		ret = m_flash.page_erase( m_pg_alt );
		dbprintf("Page erase %i", ret );
	}
	return ret;
}
/* ------------------------------------------------------------------ */
//! Get first page address
iflash_mem::paddr_t fs_env::calc_page( int n_pg ) const
{
	iflash_mem::paddr_t ret;
	if( n_pg == 0 ) {
		ret = 0;
	} else {
		ret = m_flash.num_pages() - n_pg;
	}
	return ret;
}
/* ------------------------------------------------------------------ */
//! Get pages avail
iflash_mem::paddr_t fs_env::calc_npages( int n_pg ) const
{
	int ret;
	if( n_pg == 0 ) {
		ret = m_flash.num_pages();
	} else {
		ret = n_pg;
	}
	if( !(m_flash.get_capabilities()&iflash_mem::cap_pg_no_erase) ) {
		ret /= 2;
	}
	return ret;
}
/* ------------------------------------------------------------------ */
//! Flash read cluster 
int fs_env::flash_read( unsigned fpg, unsigned clust, unsigned csize,  void *buf, size_t len )
{
	const auto pg_size = m_flash.page_size();
	const unsigned n_wr = csize/pg_size + (csize%pg_size?(1):(0));
	int ret;
	for( unsigned n=0; n<n_wr; ++n ) {
		const auto paddr = fpg + (clust * csize)/pg_size + n;
		const auto poffs = (clust * csize)%pg_size;
		const auto rll = (len>pg_size)?(pg_size):(len%pg_size);
		if( rll <= 0 ) break;
		//dbprintf("flash_read(paddr: %i poffs %i, len %i)", paddr, poffs, len );
		ret=m_flash.read( paddr, poffs, buf, rll );
		if( ret ) break;
		len-=rll;
		buf = reinterpret_cast<char*>(buf) + rll;
	}
	return ret;
}
/* ------------------------------------------------------------------ */ 
//! Flash read cluster 
int fs_env::flash_write( unsigned fpg, unsigned clust, unsigned csize, const void *buf, size_t len )
{
	const auto pg_size = m_flash.page_size();
	const unsigned n_wr = csize/pg_size + (csize%pg_size?(1):(0));
	int ret;
	for( unsigned n=0; n<n_wr; ++n ) {
		const auto paddr = fpg + (clust * csize)/pg_size + n;
		const auto poffs = (clust * csize)%pg_size;
		const auto wrl = (len>pg_size)?(pg_size):(len%pg_size);
		if( wrl <= 0 ) break;
		dbprintf("flash_write(paddr: %i poffs %i, len %i)", paddr, poffs, wrl );
		ret=m_flash.write( paddr, poffs, buf, wrl );
		if( ret ) break;
		len-=wrl;
		buf = reinterpret_cast<const char*>(buf) + wrl;
	}
	return ret;
}
/* ------------------------------------------------------------------ */
//! Reclaim the filesystem
int fs_env::reclaim() {
	return 0;
}
/* ------------------------------------------------------------------ */ 
//! Check sturcture header and if it is invalid format it
int fs_env::find_valid_page( unsigned& clust_size )
{
	int ret;
	pg_hdr hdr;
	do {
			ret = m_flash.read( m_pg_base, 0, &hdr, sizeof hdr );
			if( ret ) break;
			if( hdr.ok() ) {
				ret = err_hdr_first;
				clust_size = hdr.clust_len;
				break;
			}
			if( !can_random_access() ) {
				ret = m_flash.read( m_pg_alt, 0, &hdr, sizeof hdr );
				if( ret ) break;
				if( hdr.ok() ) {
					ret = err_hdr_second;
					clust_size = hdr.clust_len;
					break;
				} else {
					ret = err_hdr_not_found;
					break;
				}
			}
			else {
				ret = err_hdr_not_found;
				break;
			}
	} while(0);
	dbprintf("R: %i %04x %i", ret, hdr.id, hdr.clust_len );
	return ret;
}
/* ------------------------------------------------------------------ */ 
}
}

