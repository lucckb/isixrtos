#include "detail_dtoa.hpp"

extern "C" {

	void fnd_dtoa(double value, char* str, int prec)
	{
		fnd::detail::fptoa(value, str, prec );
	}

}
