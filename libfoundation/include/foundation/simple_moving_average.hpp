/*
 * =====================================================================================
 *
 *       Filename:  simple_moving_average.hpp
 *
 *    Description:  Simple moving a average implementation
 *
 *        Version:  1.0
 *        Created:  12.03.2015 23:44:51
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Lucjan Bryndza (LB), lucck(at)boff(dot)pl
 *   Organization:  BoFF
 *
 * =====================================================================================
 */
#pragma once

#include <memory>
#include <cstddef>
	
namespace fnd {
namespace algo {

	template <typename T, typename A=T>
	class sma {
	public:
		explicit sma( size_t period ) :
			period(period), window(new T[period]), 
			head(nullptr), tail(nullptr), total(0) {
		}
		~sma() {
			delete[] window;
		}
		//! Make it noncopyable
		sma& operator=(sma&) = delete;
		sma( sma& ) = delete;
		// Adds a value to the average, pushing one out if nescessary
		bool operator()(T val) {
			bool cycle {};
			// Special case: Initialization
			if (head == nullptr) {
				head = window;
				*head = val;
				tail = head;
				cycle = inc(tail);
				total = val;
				return cycle;
			}
	
			// Were we already full?
			if (head == tail) {
				// Fix total-cache
				total -= *head;
				// Make room
				cycle = inc(head);
			}
	
			// Write the value in the next spot.
			*tail = val;
			cycle = inc(tail);
	
			// Update our total-cache
			total += val;
			return cycle;
		}
	
		// Returns the average of the last P elements added to this sma.
		// If no elements have been added yet, returns 0.0
		operator T() const {
			ptrdiff_t size = this->size();
			if (size == 0) {
				return 0; // No entries => 0 average
			}
			return total / A(size); // Cast to T for floating point arithmetic
		}
		
		//! Clear
		void clear() {
			head = nullptr;
			tail = nullptr;
			total  = 0;
		}

		//! Resize the SMA 
		void resize( size_t new_size ) {
			delete [] window; 
			window = new T[new_size];
			period = new_size;
			head = nullptr;
			tail = nullptr;
			total  = 0;
		}

		//! Get if period is full
		bool full_period() const {
			return size()==ptrdiff_t(period);
		}

	private:
		size_t period;
		T * window; // Holds the values to calculate the average of.
	
		// Logically, head is before tail
		T * head; // Points at the oldest element we've stored.
		T * tail; // Points at the newest element we've stored.
	
		A total; // Cache the total so we don't sum everything each time.
	
		// Bumps the given pointer up by one.
		// Wraps to the start of the array if needed.
		bool inc(T * & p) {
			if (++p >= window + period) {
				p = window;
				return true;
			}
			return false;
		}
	
		// Returns how many numbers we have stored.
		ptrdiff_t size() const {
			if (head == nullptr)
				return 0;
			if (head == tail)
				return period;
			return (period + tail - head) % period;
		}
	};

} }
