#pragma once


#ifdef __cplusplus
namespace isix {
	static inline const char* get_version(void) {
		return "ver2.0-185-gecd00ab-dirty";
	}

}
#else
	static inline const char* isix_get_version(void) {
		return "ver2.0-185-gecd00ab-dirty";
	}
#endif
