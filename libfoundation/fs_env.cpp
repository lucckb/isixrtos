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
#ifdef _HAVE_CONFIG_H
#include <config.h>
#endif
#include <foundation/fs_env.hpp>
#include <cstdint>
#include <limits>
#include <cstring>
/* ------------------------------------------------------------------ */
#ifdef CONFIG_LIBFOUNDATION_ENV_FS_DEBUG
#include <foundation/dbglog.h>
#else
#define dbprintf(...) do {} while(0)
#endif
/* ------------------------------------------------------------------ */ 
namespace fnd {
namespace filesystem {
/* ------------------------------------------------------------------ */
namespace detail {
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
static constexpr uint16_t node_cleanup[] = { 0xFFFF, 0xFFFF };
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
	using namespace detail;
	unsigned short lru_cache_elem {};
	++env_id;
	if( env_id >= node_end ) {
		dbprintf("Invalid input identifier");
		return err_invalid_id;
	}
	if( buf_len > std::numeric_limits<decltype(fnode_0::id_next)>::max() ) {
		return err_range_id;
	}
	auto ret = init_fs();
	if( ret >= 0 ) 
	{
		ret = find_first( env_id );
		if( ret == err_invalid_id ) {
			ret = err_success;
		} else {
			ret = delete_chain( ret );
		}
	}
	if( ret == 0 ) 
	{
		const unsigned nclu = buf_len_to_n_clust( buf_len );
		ret = check_chains( nclu );
		if( m_wear_leveling )
		{
			if( ret==err_fs_full || (ret>0&&ret<int(nclu)) ) 
			{
				ret = reclaim();
				if( ret>=0 ) 
				{
					ret = check_chains( nclu );
					if( ret == int(nclu) ) {
						ret = err_success;
					} else if( ret>0 && ret<int(nclu)) {
						ret = err_fs_full;
					}
				}
			} 
			else 
			{
				ret = err_success;
			}
		} 
		else 
		{
			if( ret>0 && ret<int(nclu) ) 
				ret = err_fs_full;
			else
				ret = err_success;
		}
		if( !ret ) 
		{
			crc16 crcc;
			crcc( buf, buf_len );
			auto fc1 = find_free_cluster( get_first_cluster() );
			if( fc1 < 0 ) {
				dbprintf("Hardware failure %i", fc1 );
				return ret;
			}
			int fc2 = node_end;
			lru_cache_elem = fc1;
			for( unsigned c=0; c<nclu; ++c ) 
			{
				char ibuf[ get_clust_size() ];
				unsigned twlen;
				unsigned wlen;
				if( c == 0 ) 
				{
					auto hdr = reinterpret_cast<fnode_0*>( ibuf );
					hdr->id_next = env_id;
					hdr->len = buf_len;
					hdr->crc = crcc();
					hdr->type = 0;
					if( buf_len <= (get_clust_size()-sizeof(fnode_0)) ) 
					{
						hdr->next = node_end;
						wlen = buf_len;
					} 
					else 
					{
						fc2 = find_free_cluster( fc1 + 1 );
						m_last_free_clust = fc1;
						if( fc2 < 0 ) {
							dbprintf("Hardware failure2 %i->%i",fc1, fc2 );
							return ret;
						}
						hdr->next = fc2;
						wlen = get_clust_size()-sizeof(fnode_0);
					}
					std::memcpy( hdr->data, buf, wlen );
					twlen = sizeof(fnode_0) + wlen;
					dbprintf("WRCLU %i -> %i", fc1, hdr->next );
				} 
				else 
				{
					auto hdr = reinterpret_cast<fnode_1*>( ibuf );
					hdr->type = 1;
					if( buf_len<=(get_clust_size()-sizeof(fnode_1)) ) 
					{
						hdr->next = node_end;
						wlen = buf_len;
					} 
					else 
					{
						fc2 = find_free_cluster( fc1 + 1 );
						if( fc2 < 0 ) {
							dbprintf("Hardware failure2 %i->%i",fc1, fc2 );
							return ret;
						}
						hdr->next = fc2;
						wlen = get_clust_size()-sizeof(fnode_1);
					}
					std::memcpy( hdr->data, buf, wlen );
					twlen = sizeof(fnode_1) + wlen;
					dbprintf("WRCLUX %i -> %i", fc1, hdr->next );
				}
				ret = flash_write( get_page(), fc1, get_clust_size(), ibuf, twlen );
				buf = reinterpret_cast<const char*>(buf) + wlen;
				buf_len -= wlen;
				if( ret ) break;
				fc1 = fc2;
			}
			//! Save the last cluster with wear level mode
			m_last_free_clust = fc2;
		}
	}
	if( !ret && lru_cache_elem ) 
	{
		m_lru.put( env_id, lru_cache_elem );
		dbprintf("LRU env_id %i lru_cache_elem %i", env_id, lru_cache_elem );
	}
	return ret;
}
/* ------------------------------------------------------------------ */
/**  Get data from non volatile memory
	*   @param[in] env_id Environment identifier
	*   @param[in] buf Pointer to buffer for store data
	*   @param[in] len Buffer length
	*   @return If success return number of bytes or negative error code
	*/
int fs_env::get( unsigned env_id, void* buf, size_t buf_len )
{
	using namespace detail;
	++env_id;
	if( env_id >= node_end ) {
		dbprintf("Invalid input identifier");
		return err_invalid_id;
	}
	if( buf_len > std::numeric_limits<decltype(fnode_0::id_next)>::max() ) {
		return err_range_id;
	}
	auto ret = init_fs();
	fnode_0 node;
	crc16 ccrc;
	if( ret < 0 ) {
		return ret;
	}
	ret = find_first( env_id, &node );
	if( ret <= 0 )
		return ret;
	unsigned clu = ret;
	char lbuf[get_clust_size()];
	ret = flash_read( get_page(), clu, get_clust_size(), lbuf, get_clust_size() );
	if( ret ) {
		return ret;
	}
	if( buf_len < node.len ) {
		dbprintf("buf_len < node.len: %u<%u", buf_len, node.len );
		ret = err_buf_ovrflow;
		return ret;
	}
	if( buf_len > node.len ) {
		buf_len = node.len;
	}
	const auto real_read = buf_len;
	auto rrl = buf_len>(get_clust_size()-sizeof(fnode_0))?(get_clust_size()-sizeof(fnode_0)):(buf_len);
	{
		const auto n = reinterpret_cast<fnode_0*>(lbuf);
		std::memcpy( buf, n->data, rrl );
	}
	ccrc( buf, rrl );
	buf_len -= rrl; buf = reinterpret_cast<char*>(buf) + rrl;
	dbprintf("RDCLU %i -> %i", clu, node.next );
	clu = node.next;
	while( clu!=node_end && buf_len>0 ) {
		ret = flash_read( get_page(), clu, get_clust_size(), lbuf, sizeof lbuf );
		const auto nnode1 = reinterpret_cast<fnode_1*>(lbuf);
		if( ret ) break;	
		if( nnode1->type == 0 ) {
			ret = err_fs_fmt;
			break;
		}
		rrl = buf_len>(get_clust_size()-sizeof(fnode_1))?(get_clust_size()-sizeof(fnode_1)):(buf_len);
		std::memcpy( buf, nnode1->data, rrl );
		ccrc( buf, rrl );
		buf_len -= rrl; buf = reinterpret_cast<char*>(buf) + rrl;
		dbprintf("RDCLU %i -> %i", clu,  nnode1->next);
		clu = nnode1->next;
	}
	if( !ret ) 
	{
		if( ccrc() != node.crc ) {
			dbprintf("CRC mismatch");
			ret = err_fs_fmt;
		}
	}
	return (ret)?(ret):(real_read);
}
/* ------------------------------------------------------------------ */
/** Unset environment variable
	*   @param[in] env_id Environment identifier
	*   @return Error code on failed
	*/
int fs_env::unset( unsigned env_id )
{
	using namespace detail;
	++env_id;
	if( env_id >= node_end ) {
		dbprintf("Invalid input identifier");
		return err_invalid_id;
	}
	auto ret = init_fs();
	if( ret >= 0 ) {
		ret = find_first( env_id );
		if( ret > 0 ) {
			ret = delete_chain( ret );
		}
	}
	return ret;
}
/* ------------------------------------------------------------------ */ 
//! Calculate required cluster for buffer usage
size_t fs_env::buf_len_to_n_clust( size_t buf_len )
{
	using namespace detail;
	unsigned csize = get_clust_size();
	int ret = buf_len - ( get_clust_size() - sizeof(fnode_0) );
	if( ret <= 0 ) {
		return 1;
	} else {
		csize -= sizeof(fnode_1);
		return unsigned(ret)/csize + (unsigned(ret)%csize?(1U):(0U)) + 1U;
	}
}
/* ------------------------------------------------------------------ */ 
//!Unset internal witohout mod
int fs_env::delete_chain( unsigned cclu )
{
	using namespace detail;
	int ret = err_internal;
	//Read node
	for(unsigned pclu ;cclu!=node_end;) {
		fnode_0 node;
		dbprintf("delete_chain-> next %i", cclu );
		ret = flash_read( get_page(), cclu, get_clust_size(), &node, sizeof node );
		if( ret ) break;
		pclu = cclu;
		if( node.type == 0 ) {
			cclu = node.next;
		} else {
			cclu = node.id_next;
		}
		node.next = node_dirty;
		node.id_next = node_dirty;
		ret = flash_write( get_page(), pclu, get_clust_size(), &node, sizeof node );
		if( ret ) break;
	}
	return ret;
}
/* ------------------------------------------------------------------ */
//! Check fre chain
int fs_env::check_chains( unsigned rclu )
{
	int ret = err_internal;
	unsigned fnd_clu = 0;
	for( unsigned c=0, fc=get_first_cluster(); c<rclu; ++c ) 
	{
		ret = find_free_cluster( fc + 1 );
		if( ret > 0 ) 
		{
			if( ++fnd_clu == rclu ) {
				break;
			}
			fc = ret;
		} 
		else 
		{
			break;
		}
		if( fnd_clu == rclu ) break;
	}
	if( ret >= 0 ) 
	{
		ret = fnd_clu;
	} 
	return ret;
}
/* ------------------------------------------------------------------ */
//! Find free node
int fs_env::find_free_cluster( unsigned sclust )
{	
	using namespace detail;
	const auto pg_size = m_flash.page_size();
	auto ncs = (m_npages * pg_size)/get_clust_size();
	fnode_0 node;
	bool found = false;
	int ret = err_success;
	for( unsigned c=sclust; c<ncs; ++c ) 
	{
		ret = flash_read( get_page(), c, get_clust_size(), &node, sizeof node );
		if( ret ) {
			dbprintf("Free cluster err ret %i", ret );
			break;
		}
		if( node.id_next==node_unused || (!m_wear_leveling&&node.id_next==node_dirty) )
		{
			found = true; ret = c;
			break;
		}
	}
	if( !ret && !found ) {
		 ret = err_fs_full;
	}
	return ret;
}
/* ------------------------------------------------------------------ */
//Init fs  cal on every set and get env
int fs_env::init_fs()
{
	if( m_clust_size ) {
		return err_success;
	}
	unsigned csize;
	auto ret = find_valid_page(csize);
	if( ret == err_hdr_not_found ) 
	{
		ret = format();
		csize = get_initial_clust_size();
		m_alt_page_in_use = false;
		dbprintf("Page not found format requied CS %i RET %i", csize, ret );
	} 
	else if( ret == err_hdr_first ) 
	{
		m_alt_page_in_use = false;
		ret = err_success;
		dbprintf("init_fs -> base_page");
	} 
	else if( ret == err_hdr_second ) 
	{
		m_alt_page_in_use = true;
		ret = err_success;
		dbprintf("init_fs -> alt_page");
	}
	if(ret==err_success) 
	{
		m_clust_size = csize;
	}
	dbprintf("init_fs PG base %i alt %i size %i wear_stat %u", 
			m_pg_base, m_pg_alt, m_npages, m_wear_leveling );
	return ret;
}
/* ------------------------------------------------------------------ */
//! Find first entry by ID
int fs_env::find_first( unsigned id, detail::fnode_0* node )
{
	using namespace detail;
	const auto pg_size = m_flash.page_size();
	auto ncs = (m_npages * pg_size)/get_clust_size();
	fnode_0 tmpn;
	if( node == nullptr ) {
		node = &tmpn;
	}
	///dbprintf("find_first() ncs: %i npgs: %i pg_size %i cs: %i", ncs, m_npages, pg_size, cs );
	int ret = err_internal;
	bool found = false;
	const auto rlu_clu = m_lru.get( id );
	for( unsigned c=rlu_clu?*rlu_clu:1; c<ncs; ++c ) 
	{
		ret = flash_read( get_page(), c, get_clust_size(), node, sizeof(*node) );
		if( ret ) break;
		if( node->type == 0 && node->id_next == id ) 
		{
			ret = c;
			found = true;
			break;
		}
	}
	if( !ret && !found ) {
		ret = err_invalid_id;
	}
	return ret;
}
/* ------------------------------------------------------------------ */
//! Erase all pages beginning from startup pg
int fs_env::erase_all_random( unsigned pg )
{
	using namespace detail;
	const auto pg_size = m_flash.page_size();
	const auto cs = get_initial_clust_size();
	auto ncs = (m_npages * pg_size)/cs;
	int ret = err_internal;
	for( unsigned c = 0; c < ncs; ++c ) {
		ret = flash_write( pg , c, cs, node_cleanup, sizeof node_cleanup );
		if( ret ) break;
	}
	return ret;
}
/* ------------------------------------------------------------------ */
int fs_env::erase_all_nonrandom( unsigned pg )
{
	dbprintf("Erase all nonrandom %u", pg);
	int ret = err_internal;
	for( unsigned p = 0; p < m_npages; ++p ) 
	{
		ret = m_flash.page_erase( pg + p );
		if( ret ) {
			break;
		}
	}
	return ret;
}
/* ------------------------------------------------------------------ */
//! Format memory headers
int fs_env::format() 
{
	using namespace detail;
	auto ret = erase_all_pages( m_pg_base );
	if( ret ) {
		return ret;
	}
	const auto clust_size = get_initial_clust_size();
	const pg_hdr hdr { pg_hdr::id_valid, uint16_t(clust_size) };
	ret = m_flash.write( m_pg_base, 0, &hdr, sizeof hdr );
	if( ret ) {
		return ret;
	}
	if( !can_random_access() ) {
		ret = erase_all_pages( m_pg_alt );
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
	const auto max_clust = (m_npages * pg_size)/csize;
	int ret = err_internal;
	if( clust > max_clust ) {
		dbprintf("Cluster %i doesn't exists", clust );
		ret = err_fs_fmt;
		return ret;
	}
	for( unsigned n=0; n<n_wr; ++n ) {
		const auto paddr = fpg + (clust * csize )/pg_size + n;
		const auto poffs = (clust * csize)%pg_size;
		const auto rll = (len>pg_size)?(pg_size):(len);
		if( rll <= 0 ) break;
		//dbprintf("flash_read(paddr: %i poffs %i, len %i)", paddr, poffs, rll );
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
	const auto max_clust = (m_npages * pg_size)/csize;
	int ret = err_internal;
	if( clust > max_clust ) {
		dbprintf("Cluster %i doesn't exists", clust );
		ret = err_internal;
		return ret;
	}
	for( unsigned n=0; n<n_wr; ++n ) {
		const auto paddr = fpg + (clust * csize)/pg_size + n;
		const auto poffs = (clust * csize)%pg_size;
		const auto wrl = (len>pg_size)?(pg_size):(len);
		if( wrl <= 0 ) break;
		//dbprintf("flash_write(paddr: %i poffs %i, len %i)", paddr, poffs, wrl );
		ret=m_flash.write( paddr, poffs, buf, wrl );
		if( ret ) break;
		len-=wrl;
		buf = reinterpret_cast<const char*>(buf) + wrl;
	}
	return ret;
}
/* ------------------------------------------------------------------ */
//! Reclaim the filesystem
int fs_env::reclaim_random()
{
	using namespace detail;
	if( m_alt_page_in_use ) 
	{
		return err_fs_fmt;
	}
	if( !m_clust_size ) 
	{
		return err_internal;
	}
	int ret {};
	const auto pg_size = m_flash.page_size();
	auto ncs = (m_npages * pg_size)/m_clust_size;
	fnode_0 node;
	for( unsigned c=1; c<ncs; ++c ) 
	{
		ret = flash_read( m_pg_base, c, m_clust_size, &node, sizeof node );
		if( ret ) break;
		if( node.id_next == node_dirty ) 
		{
			node.id_next = node_unused;
			dbprintf("Erase cluster %i", c );
			ret = flash_write( m_pg_base, c, m_clust_size, &node, sizeof node );
			if( ret ) {
				break;
			}
		}
	}
	return ret;
}
/* ------------------------------------------------------------------ */
//! Reclaim the filesystem
int fs_env::reclaim_nonrandom()
{
	using namespace detail;
	const auto pa_from = m_alt_page_in_use?m_pg_alt:m_pg_base;
	const auto pa_to = m_alt_page_in_use?m_pg_base:m_pg_alt;
	const auto pg_size = m_flash.page_size();
	dbprintf("reclaim pa_from %u pa_to %u pg_size %u", pa_from, pa_to, pg_size );
	auto ncs = (m_npages * pg_size)/get_clust_size();
	unsigned char buf[get_clust_size()];
	auto node = reinterpret_cast<fnode_0*>(buf);
	int ret {};
	for( unsigned c=1; c<ncs; ++c ) 
	{
		ret = flash_read( pa_from, c, get_clust_size(), buf, get_clust_size() );
		if( ret ) break;
		if( node->id_next!=node_dirty && node->id_next!=node_unused ) 
		{
			dbprintf("Copy cluster %i", c );
			ret = flash_write( pa_to, c, get_clust_size(), buf, get_clust_size() );
			if( ret ) {
				break;
			}
		}
	}
	ret = erase_all_nonrandom( pa_from );
	if( ret ) {
		return ret;
	}
	const pg_hdr hdr2 { pg_hdr::id_valid, uint16_t(get_clust_size()) };
	ret = m_flash.write( pa_to, 0, &hdr2, sizeof hdr2 );
	m_alt_page_in_use = !m_alt_page_in_use;
	return ret;
}
/* ------------------------------------------------------------------ */ 
//! Check sturcture header and if it is invalid format it
int fs_env::find_valid_page( unsigned& clust_size )
{
	using namespace detail;
	int ret;
	pg_hdr hdr;
	do {
			ret = m_flash.read( m_pg_base, 0, &hdr, sizeof hdr );
			if( ret ) break;
			if( hdr.ok() ) 
			{
				ret = err_hdr_first;
				clust_size = hdr.clust_len;
				break;
			}
			if( !can_random_access() ) 
			{
				ret = m_flash.read( m_pg_alt, 0, &hdr, sizeof hdr );
				if( ret ) break;
				if( hdr.ok() ) 
				{
					ret = err_hdr_second;
					clust_size = hdr.clust_len;
					break;
				} 
				else 
				{
					ret = err_hdr_not_found;
					break;
				}
			}
			else 
			{
				ret = err_hdr_not_found;
				break;
			}
	} while(0);
	return ret;
}
/* ------------------------------------------------------------------ */ 
}
}

