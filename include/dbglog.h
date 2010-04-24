/* ------------------------------------------------------------------ */
/*
 * dbglog.h
 *
 *  Created on: 2010-03-21
 *      Author: lucck
 */
/* ------------------------------------------------------------------ */
#ifndef DBGLOG_H_
#define DBGLOG_H_
/* ------------------------------------------------------------------ */
#ifdef PDEBUG /* Debug enabled */
#include <tiny_printf.h>



#ifdef __cplusplus

#define dblog_init(function,arg) fnd::register_printf_putc_handler(function,arg)
#define dbprintf(fmt,...) fnd::tiny_printf("%s:%d|"fmt"\r\n",__FILE__,__LINE__,__VA_ARGS__)
#define dbputs(str) fnd::tiny_printf("%s:%d|"str"\r\n",__FILE__,__LINE__)

#else

#define dblog_init(function,arg) register_printf_putc_handler(function,arg)
#define dbprintf(fmt,...) tiny_printf("%s:%d|"fmt"\r\n",__FILE__,__LINE__,__VA_ARGS__)
#define dbputs(str) tiny_printf("%s:%d|"str"\r\n",__FILE__,__LINE__)

#endif /*__cplusplus */


#else

#define dblog_init(function,arg)

#define dbprintf(fmt,...)

#define dbputs(str)

#endif /* PDEBUG */


/* ------------------------------------------------------------------ */
#endif /* DBGLOG_H_ */
/* ------------------------------------------------------------------ */
