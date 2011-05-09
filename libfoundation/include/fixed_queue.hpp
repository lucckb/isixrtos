/*----------------------------------------------------------*/
/*
 * fixed_queue.hpp
 *
 *  Created on: 2009-12-09
 *      Author: lucck
 */
/*----------------------------------------------------------*/
#ifndef FIXED_QUEUE_HPP_
#define FIXED_QUEUE_HPP_
/*----------------------------------------------------------*/
#include <cstddef>

/*----------------------------------------------------------*/
namespace fnd
{

/*----------------------------------------------------------*/
template <typename T, std::size_t Sz=128>
class fixed_queue
{
public:

	//Fixed queue constructor
	fixed_queue():head(0),tail(0) {};

	//Push element
	bool push(const T &c)
	{
		std::size_t next;
		// check if FIFO has room
		next = (head + 1) % Sz;
		if (next == tail) return true;
		buf[head] = c;
		head = next;
		return false;
	}

	T& front() { return buf[tail]; }
	const T& front() const { return buf[tail]; }

	//Pop element
	bool pop(T &c)
	{
		std::size_t next;
		// check if FIFO has data
		if (head == tail) return true;
		next = (tail + 1) % Sz;
		c = buf[tail];
		tail = next;
		return false;
	}

	//Get size
	std::size_t size() const
	{
		return (Sz + head - tail) % Sz;
	}

	//Get free count
	std::size_t free() const
	{
		return (Sz - 1 - size());
	}

	//Clear the fifo context
	void erase()
	{
		head = 0;
		tail = 0;
	}

private:
	T buf[Sz];
	std::size_t head;
	std::size_t tail;
};

/*----------------------------------------------------------*/

}
/*----------------------------------------------------------*/
#endif /* FIXED_QUEUE_HPP_ */
