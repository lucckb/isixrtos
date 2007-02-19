#include <isix/config.h>
#include <isix/types.h>
#include <isix/scheduler.h>
#include <asm/interrupt.h>
#include <isix/time.h>

/*-----------------------------------------*/
//Return current time
time_t time(time_t *time)
{ 
    reg_t irq_s = irq_disable();
    time_t t = sched_time;
    irq_restore(irq_s);
    if(time!=NULL)  *time = t;
    return t;
}
/*-----------------------------------------*/

