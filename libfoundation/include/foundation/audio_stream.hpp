/*
 * =====================================================================================
 *
 *       Filename:  audio_stream.hpp
 *
 *    Description:  Audio stream buffer
 *
 *        Version:  1.0
 *        Created:  31.05.2017 19:39:14
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Lucjan Bryndza (LB), lbryndza.p@boff.pl
 *   Organization:  BoFF
 *
 * =====================================================================================
 */

#pragma once
#include <cstddef>
#include <iterator>

namespace fnd {
namespace drv {

	template <typename T>
	class audio_stream
	{
	public:
		explicit audio_stream(void *raw, std::size_t len)
			:m_raw( raw ), m_size(len)
		{
		}
		~audio_stream() {}
		//! Copy not allowed (no mempool)
		audio_stream(audio_stream&) = delete;
		audio_stream& operator=(audio_stream&) = delete;
		audio_stream(audio_stream&& o)
			: m_raw(o.m_raw), m_size(o.m_size) {
			o.m_raw = nullptr; o.m_size = 0;
		}
		audio_stream& operator=(audio_stream&& o) {
			m_raw = o.m_raw; o.m_raw = nullptr;
			m_size = o.m_size; o.m_size = 0;
			return *this;
		}
        // type definitions
        typedef T              value_type;
        typedef T*             iterator;
        typedef const T*       const_iterator;
        typedef T&             reference;
        typedef const T&       const_reference;
        typedef std::size_t    size_type;
        typedef std::ptrdiff_t difference_type;

        // iterator support
        iterator        begin()       { return reinterpret_cast<T*>(m_raw); }
        const_iterator  begin() const { return reinterpret_cast<T*>(m_raw); }
        const_iterator cbegin() const { return reinterpret_cast<T*>(m_raw); }

        iterator        end()       { return reinterpret_cast<T*>(m_raw)+m_size; }
        const_iterator  end() const { return reinterpret_cast<T*>(m_raw)+m_size; }
        const_iterator cend() const { return reinterpret_cast<T*>(m_raw)+m_size; }

		using reverse_iterator = std::reverse_iterator<iterator>;
        using const_reverse_iterator = std::reverse_iterator<const_iterator>;

		reverse_iterator rbegin() { return reverse_iterator(end()); }
        const_reverse_iterator rbegin() const {
            return const_reverse_iterator(end());
        }
        const_reverse_iterator crbegin() const {
            return const_reverse_iterator(end());
        }
        reverse_iterator rend() { return reverse_iterator(begin()); }
        const_reverse_iterator rend() const {
            return const_reverse_iterator(begin());
        }
        const_reverse_iterator crend() const {
            return const_reverse_iterator(begin());
        }

        reference operator[](size_type i)
        {
            return reinterpret_cast<T*>(m_raw)[i];
        }
        const_reference operator[](size_type i) const
        {
            return reinterpret_cast<T*>(m_raw)[i];
        }

		// front() and back()
        reference front()
        {
            return reinterpret_cast<T*>(m_raw)[0];
        }
        const_reference front() const
        {
            return reinterpret_cast<T*>(m_raw)[0];
        }
        reference back()
        {
            return reinterpret_cast<T*>(m_raw)[m_size-1];
        }
        const_reference back() const
        {
            return reinterpret_cast<T*>(m_raw)[m_size-1];
        }

		// direct access to data (read-only)
        const T* data() const { return reinterpret_cast<T*>(m_raw); }
        T* data() { return reinterpret_cast<T*>(m_raw); }
        T* c_array() { return reinterpret_cast<T*>(m_raw); }
        size_type size() { return m_size; }
        bool empty() { return !m_raw; }
        size_type max_size() { return m_size; }
		// assign one value to all elements
        void assign (const T& value) { fill ( value ); }
        void fill   (const T& value)
        {
            std::fill_n(begin(),size(),value);
        }
	private:
		/* data */
		void* m_raw;
		std::size_t m_size;
	};

}}

