/*----------------------------------------------------------*/
/*
 * fixed_vector.hpp
 *
 *  Created on: 2010-05-04
 *      Author: lucck
 */
/*----------------------------------------------------------*/
#ifndef FND_FIXED_VECTOR_HPP_
#define FND_FIXED_VECTOR_HPP_
/*----------------------------------------------------------*/
#include <bits/stl_iterator_base_types.h>
#include <bits/stl_iterator_base_funcs.h>
#include <bits/stl_iterator.h>
#include <cstddef>
namespace fnd {

/*----------------------------------------------------------*/
template <typename _Tp, std::size_t Sz=128>
class fixed_vector
{
public:
      typedef _Tp                    value_type;
      typedef  _Tp*           pointer;
      typedef const _Tp*     const_pointer;
      typedef _Tp&         reference;
      typedef const _Tp& const_reference;
      typedef __gnu_cxx::__normal_iterator<pointer, fixed_vector> iterator;
      typedef __gnu_cxx::__normal_iterator<const_pointer, fixed_vector>
      const_iterator;
      typedef std::reverse_iterator<const_iterator>  const_reverse_iterator;
      typedef std::reverse_iterator<iterator>        reverse_iterator;
      typedef size_t                     size_type;
      typedef ptrdiff_t                  difference_type;
public:
    fixed_vector() : nelems(0)
    {
    }

    const_reference at ( size_type n ) const
    {
        return elems[n];

    }
    reference at ( size_type n )
    {
        return elems[n];
    }

    reference back ( )
    {
        return elems[nelems>0?(nelems-1):0];
    }

    const_reference back ( ) const
    {
        return elems[nelems>0?(nelems-1):0];
    }

    reference front ( )
    {
        return elems[0];
    }

    const_reference front ( ) const
    {
        return elems[0];
    }

    iterator begin ()
    {
        return iterator(&front());
    }

    const_iterator begin () const
    {
       return const_iterator(&front());
    }

	const iterator cbegin() const 
	{
	   return begin();
	}

    iterator end ()
    {
       return iterator(&elems[nelems]);
    }

    const_iterator end () const
    {
        return const_iterator(&elems[nelems]);
    }

    const_iterator cend () const {
		return end();
	}

    size_type capacity () const
    {
        return Sz;
    }

    void clear()
    {
        nelems = 0;
    }

    bool empty () const
    {
        return !nelems;
    }

    size_type max_size () const
    {
        return Sz;
    }

    reference operator[] ( size_type n )
    {
        return elems[n];
    }

    const_reference operator[] ( size_type n ) const
    {
        return elems[n];
    }

    void pop_back ( )
    {
        if(nelems>0) nelems--;
    }

    void push_back ( const_reference x )
    {
        if(nelems<Sz)
            elems[nelems++] = x;
    }

    reverse_iterator rbegin()
    {
       return reverse_iterator(end());
    }

    const_reverse_iterator rbegin() const
    {
        return const_reverse_iterator(end());
    }

    const_reverse_iterator crbegin() const 
	{
		return rbegin();
	}

    reverse_iterator rend()
    {
        return reverse_iterator(begin());
    }

    const_reverse_iterator rend() const
    {
       return const_reverse_iterator(begin());
    }

    const_reverse_iterator crend() const 
	{
		return rend();
	}

    size_type size() const
    {
        return nelems;
    }

private:
    _Tp elems[Sz];
    std::size_t nelems;
};
/*----------------------------------------------------------*/
}
/*----------------------------------------------------------*/
#endif /* FIXED_VECTOR_HPP_ */
