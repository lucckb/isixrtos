/*
 * noncopyable.hpp
 *
 *  Created on: 05-01-2012
 *      Author: lucck
 */

#ifndef FND_NONCOPYABLE_HPP_
#define FND_NONCOPYABLE_HPP_

namespace fnd {

//  Private copy constructor and copy assignment ensure classes derived from
//  class noncopyable cannot be copied.

//  Contributed by Dave Abrahams

namespace noncopyable_  // protection from unintended ADL
{
  class noncopyable
  {
   protected:
      noncopyable() {}
      ~noncopyable() {}
#if __cplusplus > 199711L
private:
      noncopyable( const noncopyable& ) = delete;
      const noncopyable& operator=( const noncopyable& ) = delete;
#else
   private:  // emphasize the following members are private
         noncopyable( const noncopyable& );
         const noncopyable& operator=( const noncopyable& );
#endif
  };
}

typedef noncopyable_::noncopyable noncopyable;

} // namespace boost


#endif /* NONCOPYABLE_HPP_ */
