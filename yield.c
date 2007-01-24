#include "lpc214x.h"
#include "printk.h"



int main(void)
{
	printk_init(UART_BAUD(115200));
	printk("Ala=%d Ola=%d Jola=%d\n",10,-20,30);
	return 0;
}

