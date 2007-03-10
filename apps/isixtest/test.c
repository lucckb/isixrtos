#include <isix/task.h>
#include <isix/semaphore.h>
#include <isix/time.h>
#include <isix/printk.h>
#include <isix/device.h>
#include <dev/uart16550.h>
/*-----------------------------------------------------------------------*/
TASK_FUNC(wake_task,n)
{
   while(1)
   {
        schedule_timeout(HZ*3);
        printk("wake_task: Hello !\n");
   }
}
/*-----------------------------------------------------------------------*/

TASK_FUNC(fun_task,n)
{
    void *fd = open("/com0",0);
    while(1)
    {
        schedule_timeout(HZ*2);
        write(fd,"a",1);
    }
}
/*-----------------------------------------------------------------------*/
//Main test function
int main(void)
{
   printk("****** Hello from OS ******\n");
   device_t *dev = create_uart_device(0,"com0");
   printk("device ptr %08x\n",dev);
   register_device(dev);
   task_create(fun_task,NULL,400,10);
   //task_create(wake_task,NULL,400,8);
   return 0;
}
