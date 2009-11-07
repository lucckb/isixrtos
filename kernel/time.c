#include <isix/config.h>
#include <isix/types.h>
#include <prv/scheduler.h>
#include <prv/list.h>
#include <isix/time.h>


/*------------------------------------------------------------*/
//Get currrent jiffies
uint64_t get_jiffies(void)
{
    uint64_t t1,t2;
    do
    {
        t1 = jiffies;
        t2 = jiffies;
    }
    while(t1!=t2);
    return t2;
}

/*------------------------------------------------------------*/
