// QUnit.hpp - a simple unit test framework for C++
// 
// Typical usage:
// 
// #include "QUnit.hpp"
// 
// int main() {
//   QUnit::UnitTest qunit(std::cerr, QUnit::verbose);
// 
//   QUNIT_IS_TRUE(true);
//   QUNIT_IS_FALSE(4!=4);
//   QUNIT_IS_EQUAL(42, 42.0);
//   QUNIT_IS_NOT_EQUAL(42,"43");
// 
//   return qunit.errors();
// }
//

#ifndef _QUNIT_HPP_
#define _QUNIT_HPP_

#include <cstring>
#include <foundation/utils.h>
#include <foundation/tiny_printf.h>
#include <string>

namespace QUnit {
namespace detail {
	static inline void convert(int value, char* buf, size_t ) {
		fnd::fnd_itoa(buf,value,1,'0');
	}
	static inline void convert(unsigned value, char* buf, size_t) {
		fnd::fnd_uitoa(buf,value,1,'0',10);
	}
	static inline void convert(float value, char* buf, size_t) {
		fnd::fnd_ftoa(value,buf,4);
	}
	static inline void convert(double value, char* buf, size_t) {
		fnd::fnd_dtoa(value,buf,4);
	}
	static inline void convert(const char *value, char* buf, size_t s) {
		std::strncpy(buf,value, s);
	}
	static inline void convert(const void *value, char* buf, size_t ) {
			fnd::fnd_uitoa(buf, reinterpret_cast<unsigned>(value),8,'0',16);
		}
	static inline void convert(std::nullptr_t&, char* buf, size_t s) {
		std::strncpy(buf,"nullptr", s);
	}
	static inline void convert(bool value, char* buf, size_t s) {
		if( value )
			std::strncpy(buf,"true", s);
		else
			std::strncpy(buf,"false", s);
	}
    static inline void convert( std::string &str, char* buf, size_t s) {
        std::strncpy( buf, str.c_str(), s );
    }
	static inline void convert( char ch, char* buf, size_t s) {
        std::strncpy( buf, &ch, s );
    }

}}


#define QUNIT_IS_EQUAL(expr1,expr2)     QUNIT_COMPARE(true,QUnit::equal,expr1,expr2)
#define QUNIT_IS_NOT_EQUAL(expr1,expr2) QUNIT_COMPARE(true,QUnit::nequal,expr1,expr2)
#define QUNIT_IS_TRUE(expr)             QUNIT_COMPARE(false,QUnit::equal,expr,true)
#define QUNIT_IS_FALSE(expr)            QUNIT_COMPARE(false,QUnit::equal,expr,false)

#define QUNIT_COMPARE(compare,result,expr1,expr2) do {             \
        qunit.evaluate(                                         \
            compare,result,expr1,expr2, #expr1, #expr2,         \
            __FILE__, __LINE__, __FUNCTION__ );                 \
    } while(0)                                                       \


namespace QUnit {

    enum { silent, quiet, normal, verbose, noisy };
    enum expr_t { equal, nequal };
    
	namespace compare {
		struct ops {};
		struct equal : public ops {
			template <typename T1, typename T2> bool operator()( T1 o1, T2 o2 ) {
				return o1==o2;
			}
		};
	}


	class UnitTest {
        
    public:
        UnitTest(  int verboseLevel)
            : verboseLevel_(verboseLevel) , errors_(0) , tests_(0)
        {
        }

        inline ~UnitTest()
        {
            if ( verboseLevel_ > quiet )
                printStatus();
        }

        inline void verboseLevel(int level)
        {
            verboseLevel_ = level;
        }

        int verboseLevel()
        {
            return verboseLevel_;
        }

        void printStatus() {
            fnd::tiny_printf( "Testing %s (%i tests, %i ok, %i failed)\r\n",
            		( errors_ ? "FAILED" : "OK" ),
            		tests_, ( tests_ - errors_ ), errors_
            );
        }

        int errors() const
        {
            return errors_;
        }
        const char* type2str( expr_t type )
        {
             switch(type) {
               case equal: return "==";
               case nequal: return "!=";
               default: return "";
               }
        }
        template <typename T1, typename T2>
        inline void evaluate(
           	bool compare, expr_t type, T1 expr1, T2 expr2,
           	const char* str1, const char* str2,
            const char * file, int line, const char * func)
        {
        	   bool ok {};
               if( type ==  equal ) {
                  ok = (expr1==expr2);
               } else if( type ==  nequal ) {
                   ok = (expr1!=expr2);
               }
               tests_ += 1;
               errors_ += ok ? 0 : 1;
               if( (ok && !(verboseLevel_ > normal)) || verboseLevel_ == silent )
                   return;
               char s1[48] = {0};
               char s2[48] = {0};
               detail::convert(expr1, s1, sizeof s1);
               detail::convert(expr2, s2, sizeof s2);
               if( compare )
               {
               	const char* cmp = type2str(type);
               	fnd::tiny_printf(
                   	 "%s%s%i: %s%s(): compare {%s} %s {%s} got {\"%s\"} %s {\"%s\"}\r\n",
                   	 file,( ok ? ";" : ":" ),line,( ok ? "OK/" : "FAILED/" ),
                   	 func, str1, cmp, str2, s1, cmp, s2
                  );
               }
               else
               {
                   fnd::tiny_printf(
                   	 "%s%s%i: %s%s(): evaluate {%s} == %s\r\n",
                   	 file, ( ok ? ";" : ":" ),line,
                   	 ( ok ? "OK/" : "FAILED/" ),
                   	 func,
                   	 str1,
                   	 s1
                   );
               }
           }
    private:
        int verboseLevel_;
        int errors_;
        int tests_;
    };
}

#endif // _QUNIT_HPP_
