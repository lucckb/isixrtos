#pragma once

/*--------------------------------------------------*/
#ifdef __cplusplus
namespace isix
{
#endif

/*--------------------------------------------------*/
/** Returned error codes definitions */
enum isix_error
{
	ISIX_EOK = 0,       	/** No Error */
	ISIX_ENOMEM = -2,   	/** No memory */
	ISIX_ETIMEOUT = -3, 	/** Timeout   */
	ISIX_EINVARG = -4,  	/** Invalid argument */
	ISIX_EFIFOFULL = -5, 	/** Fifo queue is full */
	ISIX_EBUSY = -6,     	/** Resource already used  */
	ISIX_ENOPRIO = -7,   	/** Priority is invalid */
	ISIX_ENOTSUP = -8		/** Operation not supported */
};

/*--------------------------------------------------*/
#ifdef __cplusplus
}
#endif /* __cplusplus */

/*--------------------------------------------------*/
