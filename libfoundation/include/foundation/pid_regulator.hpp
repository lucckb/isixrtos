/*
 * pid_regulator.hpp
 *
 *  Created on: 10 lis 2013
 *      Author: lucck
 */
/* ------------------------------------------------------------------ */
#ifndef LIBFOUNDATION_PID_REGULATOR_HPP_
#define LIBFOUNDATION_PID_REGULATOR_HPP_
/* ------------------------------------------------------------------ */
#include <foundation/noncopyable.hpp>
/* ------------------------------------------------------------------ */
namespace fnd {

/* ------------------------------------------------------------------ */
template <typename T>
class pid_regulator	 : private fnd::noncopyable
{
public:
	explicit pid_regulator( T tp )
		: m_tp( tp ) {}

	//!Reset the regulator to the initial state
	void clear() {
		m_sum = T(0);
		m_ep = T(0);
		m_ov  = false;
	}
	/** Setup the PID params
	 *
	 * @param kp KP parameter
	 * @param ti TI parameter
	 * @param td TD parameter
	 */
	void operator()( T kp, T ti, T td ) {
		m_kp = kp;
		m_ti = ti;
		m_td = td;
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
		if( m_ti != T(0) ) {
			u += m_kp * (m_tp/m_ti) * m_sum;
		}
		if( m_td !=  T(0) ) {
			u += m_kp * (m_td/m_tp) * (e - m_ep);
		}
		m_ep = e;
		m_ov = false;
		if(u > T(1)) {
			u = T(1);
			m_ov = true;
		}
		else if (u < T(-1)) {
			u = T(-1); m_ov = true;
		}
		return u;
	}
private:
	T m_kp { T(1) };		//KP parameter
	T m_ti {};				//TI parameter
	T m_td {};				//TP parameter
	const T m_tp;			//Sample time
	T m_sum {};				//Integral value
	T m_ep {};				//Previous E
	bool m_ov {};			//Overflow anti windup
};

/* ------------------------------------------------------------------ */
} /* namespace fnd */

/* ------------------------------------------------------------------ */
#endif /* PID_REGULATOR_HPP_ */
/* ------------------------------------------------------------------ */
