/*
 * pid_regulator.hpp
 *
 *  Created on: 10 lis 2013
 *      Author: lucck
 */

#pragma once

#include <foundation/algo/noncopyable.hpp>

namespace fnd {


template <typename T>
class pid_regulator	 : private fnd::noncopyable
{
public:
	//! Constructor V1
	explicit pid_regulator( T tp )
		: m_tp( tp ) {}
	//! Constructor V2
	pid_regulator( T kp, T ki, T kd, T tp )
		: m_kp(kp)
		, m_ki(ki)
		, m_kd(kd)
		, m_tp(tp)
	{
	}
	//!Reset the regulator to the initial state
	void clear() {
		m_sum = T(0);
		m_ep = T(0);
		m_ov  = false;
	}
	/** Setup the PID params
	 *
	 * @param kp KP parameter
	 * @param ti KI parameter
	 * @param td KD parameter
	 */
	void operator()( T kp, T ki, T kd ) {
		m_kp = kp;
		m_ki = ki;
		m_kd = kd;
		clear();
	}
	/** Calculate the PID step
	 *
	 * @param input Input value from -1 to 1
	 * @return Output value from -1 to 1
	 */
	T operator()( T e )
	{
		T u = m_kp * e;
		if( !m_ov ) m_sum += e;
		if( m_ki != T(0) ) {
			u += m_tp * m_ki * m_sum;
		}
		if( m_kd != T(0) ) {
			u += m_kd * (e - m_ep) / m_tp;
		}
		m_ep = e;
		m_ov = false;
		if(u > T(1)) {
			u = T(1); m_ov = true;
		}
		else if (u < T(-1)) {
			u = T(-1); m_ov = true;
		}
		return u;
	}
private:
	T m_kp { T(1) };		//KP parameter
	T m_ki {};				//TI parameter
	T m_kd {};				//TP parameter
	const T m_tp;			//Sample time
	T m_sum {};				//Integral value
	T m_ep {};				//Previous E
	bool m_ov {};			//Overflow anti windup
};


} /* namespace fnd */



