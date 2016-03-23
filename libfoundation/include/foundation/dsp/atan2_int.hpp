/*
 * atan2_int.hpp
 *
 *  Created on: 04-05-2013
 *      Author: lucck
 */

#ifndef DSP_ATAN2_INT_HPP_
#define DSP_ATAN2_INT_HPP_

/* ------------------------------------------------------------------------- */
namespace dsp {
namespace integer {
/* ------------------------------------------------------------------------- */
//Template for calculate integer version of the ATAN2
template<typename T, int SCALE>
T atan2( T y , T x )
{
   constexpr auto CPI = 4.0 * std::atan(1.0);
   constexpr decltype(x) K1 = (1.0/0.1963)* SCALE;
   constexpr decltype(x) K2  = (1.0/0.9817)* SCALE;
   constexpr decltype(x) K3 = (CPI/4)* SCALE;
   constexpr decltype(x) K4 = (3.0*CPI/4.0)* SCALE;
   auto abs_y = (y>0)?(y):(-y);
   decltype(x) angle;
   if( x == 0 && y == 0 ) return 0;
   if (x>=0)
   {
      auto r = ((x - abs_y)*SCALE) / (x + abs_y);
      angle = ((r*r)/SCALE)*r /K1 - (r* SCALE)/K2 + K3;
   }
   else
   {
      auto r = ((x + abs_y)*SCALE) / (abs_y - x);
      angle = ((r*r)/SCALE)*r /K1 - (r*SCALE)/K2 + K4;
   }
   // negate if in quad III or IV
   return (y < 0)?(-angle):(angle);
}

/* ------------------------------------------------------------------------- */
}}
/* ------------------------------------------------------------------------- */
#endif /* ATAN2_INT_HPP_ */
