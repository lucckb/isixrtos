#ifndef __ISIX_PRINTK_H
#define __ISIX_PRINTK_H

#include <isix/types.h>

void console_output(bool use_sem,const char *text,...);

#define printk(...) console_output(false,__VA_ARGS__)

#define printf(...) console_output(true,__VA_ARGS__)

#endif


