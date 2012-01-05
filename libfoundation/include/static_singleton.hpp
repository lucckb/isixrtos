/*
 * singleton.hpp
 *
 *  Created on: 05-01-2012
 *      Author: lucck
 */

#ifndef FND_SINGLETON_HPP_
#define FND_SINGLETON_HPP_
/*----------------------------------------------------------*/
#include <cstddef>

/*----------------------------------------------------------*/
namespace fnd
{
template <typename T> class static_singleton
{
public:
	static T& instance()
	{
	   static T me;
	   return me;
	}
protected:
	static_singleton();
	~static_singleton();
private:
	static_singleton(static_singleton const&);
	static_singleton& operator=(static_singleton const&);
};

}
/*----------------------------------------------------------*/
#endif /* SINGLETON_HPP_ */
