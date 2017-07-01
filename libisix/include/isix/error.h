#pragma once


/** Returned error codes definitions */
enum isix_error
{
	ISIX_EOK			= 0,		/** No Error */
	ISIX_ENOMEM			= -2,		/** No memory */
	ISIX_ETIMEOUT		= -3,		/** Timeout   */
	ISIX_EINVARG		= -4,		/** Invalid argument */
	ISIX_EFIFOFULL		= -5,		/** Fifo queue is full */
	ISIX_EBUSY			= -6,		/** Resource already used  */
	ISIX_ENOPRIO		= -7,		/** Priority is invalid */
	ISIX_ENOTSUP		= -8,		/** Operation not supported */
	ISIX_ERESET			= -9,		/** Current object is in reset state */
	ISIX_EDESTROY		= -10,		/** Object has been destroyed */
	ISIX_ESTATE			= -11,		/** Invalid state  */
	ISIX_EPERM			= -12,		/** Not permited */
	ISIX_ENOTLOCKED		= -14,		/** Not locked */
	ISIX_ENOREF			= -15,		/** Task is not referenced */
	ISIX_EFIFOEMPTY		= -16,		/** Fifo queue is empty */
	ISIX_EINVADDR		= -17,		/** Invalid address */
	ISIX_EBADF			= -18,		/** Invalid data state */
};
