#pragma once



#ifdef __cplusplus
namespace isix {

	static inline const char* get_version(void) {
		return ISIX_GIT_VERSION;
	}

}
#else
	static inline const char* isix_get_version(void) {
		return ISIX_GIT_VERSION;
	}

#endif
