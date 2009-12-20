#ifndef __ISIX_ERROR_H__
#define __ISIX_ERROR_H__

/*--------------------------------------------------*/
#ifdef __cplusplus
namespace isix
{
#endif

/*--------------------------------------------------*/
/* Returned error types definition */
enum isix_error
{
	ISIX_EOK = 0,       	/* <------ No Error */
	ISIX_ENOMEM = -1,   	/* <------ No memory */
	ISIX_ETIMEOUT = -2, 	/* <------ Timeout   */
	ISIX_EINVARG = -3,  	/* <------ Invalid argument */
	ISIX_EFIFOFULL = -4, 	/* <----- Fifo queue is full */
	ISIX_EBUSY = -5,     	/* <------ Resource is used  */
	ISIX_ENOPRIO = -6   	/* <------ Given priority is invalid */
};

/*--------------------------------------------------*/
#ifdef __cplusplus
}
#endif /* __cplusplus */

/*--------------------------------------------------*/
#endif /* _ISIX_ERROR_H__ */

/*--------------------------------------------------*/
