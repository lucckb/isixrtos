/*
 * ihandle.h
 *
 *  Created on: 27-03-2011
 *      Author: lucck
 */

#ifndef ISIX_IHANDLE_H_
#define ISIX_IHANDLE_H_

/*--------------------------------------------------------------*/
#ifdef __cplusplus
namespace isix {
#endif
/*--------------------------------------------------------------*/
//Universal handle for multiple objs
union ihandle
{
#ifdef __cplusplus
	ihandle(sem_t *sem_) : sem(sem_) {}
	ihandle(fifo_t *fifo_) : fifo(fifo_) {}
	ihandle(semaphore &sem_) : sem(sem_.sem) {}
	ihandle(fifo_base &fifo_) : fifo(fifo_.hwnd) {}
private:
#endif
	sem_t *sem;
	fifo_t *fifo;
};
/*--------------------------------------------------------------*/
typedef union ihandle ihandle_t;
/*--------------------------------------------------------------*/
#ifdef __cplusplus
}
#endif

/*--------------------------------------------------------------*/
#endif /* IHANDLE_H_ */
