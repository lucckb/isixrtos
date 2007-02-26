#include <isix/config.h>
#include <isix/types.h>
#include <isix/scheduler.h>
#include <isix/list.h>
#include <isix/time.h>

/*------------------------------------------------------------*/

//All timer lists
static list_entry_t timers_list;

/*------------------------------------------------------------*/
//Get currrent jiffies
u64 get_jiffies(void)
{
    u64 t1,t2;
    do
    {
        t1 = jiffies;
        t2 = jiffies;
    }
    while(t1!=t2);
    return t2;
}

/*------------------------------------------------------------*/
//System timer ticks
void sys_timer_tick(void)
{
    
}

