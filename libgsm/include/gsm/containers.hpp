/*
 * =====================================================================================
 *
 *       Filename:  containers.hpp
 *
 *    Description:  Containers definition for GSM library
 *
 *        Version:  1.0
 *        Created:  01.03.2015 19:58:49
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Lucjan Bryndza (LB), lucck(at)boff(dot)pl
 *   Organization:  BoFF
 *
 * =====================================================================================
 */

#include <vector>
namespace gsm_modem {

	template<typename T>
		using vector = std::vector<T>;
}
