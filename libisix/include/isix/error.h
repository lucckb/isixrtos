#ifndef ISIX_ERROR_H_
#define ISIX_ERROR_H_

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
	ISIX_ENOMEM = -1,   	/** No memory */
	ISIX_ETIMEOUT = -2, 	/** Timeout   */
	ISIX_EINVARG = -3,  	/** Invalid argument */
	ISIX_EFIFOFULL = -4, 	/** Fifo queue is full */
	ISIX_EBUSY = -5,     	/** Resource already used  */
	ISIX_ENOPRIO = -6,   	/** Priority is invalid */
	ISIX_ENOTSUP = -7		/** Operation not supported */
};

/*--------------------------------------------------*/
#ifdef __cplusplus
}
#endif /* __cplusplus */

/*--------------------------------------------------*/
#endif /* _ISIX_ERROR_H__ */

/*--------------------------------------------------*/
