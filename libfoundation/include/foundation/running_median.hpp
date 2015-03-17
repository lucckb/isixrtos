/*
 * =====================================================================================
 *
 *       Filename:  running_median.hpp
 *
 *    Description:  Running median filter
 *
 *        Version:  1.0
 *        Created:  14.03.2015 12:52:43
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Lucjan Bryndza (LB), lucck(at)boff(dot)pl
 *   Organization:  BoFF
 *
 * =====================================================================================
 */
#pragma once
#include <cstddef>
#include <algorithm>

namespace fnd {
namespace algo {

	template <typename T> 
	class running_median {
	public:
		//! Constructor
		explicit running_median( size_t period ) 
			: m_arun(new T[period]), m_asort(new T[period]),
			m_size(period)

		{}
		//Noncopyable
		running_median( running_median& ) = delete;
		running_median& operator=(running_median&)=delete;
		//Resize and back to normal state
		void resize( size_t new_size ) 
		{
			delete [] m_arun;
			delete [] m_asort;
			m_size = new_size;
			m_idx = 0;
			m_cnt = 0;
			m_arun = new T[new_size];
			m_asort = new T[new_size];
		}
		//Insert to filter
		bool operator()( T value ) 
		{
			m_arun[m_idx++] = value;
			if (m_idx >= m_size) m_idx = 0; // wrap around
			if (m_cnt < m_size) m_cnt++;
			return m_cnt==m_size;
		}
		//! Get value
		T operator ()() const {
			if( m_cnt > 0 ) {
				std::partial_sort_copy( m_arun, m_arun+m_cnt, m_asort, m_asort+m_cnt );
				if( m_cnt%2==0) {
					return (m_asort[m_cnt/2-1] + m_asort[m_cnt/2] ) / T(2);
				} else {
					return m_asort[m_cnt/2];
				}
			} else {
				return T();
			}
		}
		//! Get size
		size_t size() const {
			return m_size;
		}
	private:
		T* m_arun;
		T* m_asort;
		size_t m_size {};
		size_t m_idx {};
		size_t m_cnt {};
	};

}
}
