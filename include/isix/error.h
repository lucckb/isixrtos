#ifndef __ISIX_ERROR_H__
#define __ISIX_ERROR_H__


/* Returned error types definition */
#define ISIX_EOK 0       /* <------ No Error */

#define ISIX_ENOMEM -1   /* <------ No memory */

#define ISIX_ETIMEOUT -2 /* <------ Timeout   */

#define ISIX_EINVARG -3  /* <------ Invalid argument */

#define ISIX_EFIFOFULL -4 /* <----- Fifo queue is full */

#define ISIX_EBUSY -5     /* <------ Resource is used  */

#define ISIX_ENOPRIO -6   /* <------ Given priority is invalid */

#endif

