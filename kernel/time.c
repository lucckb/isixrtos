#include <isix/config.h>
#include <isix/types.h>
#include <prv/scheduler.h>
#include <prv/list.h>
#include <isix/time.h>


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
//Convert tm to time_t
time_t mktime(const struct tm *t)
{
	register short	month, year;
	register time_t	result;
	static const int m_to_d[12] = {0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334};
	month = t->tm_mon;
	year = t->tm_year + month / 12 + 1900;
	month %= 12;
	if (month < 0)
	{
		year -= 1;
		month += 12;
	}
	result = (year - 1970) * 365 + (year - 1969) / 4 + m_to_d[month];
	result = (year - 1970) * 365 + m_to_d[month];
	if (month <= 1)
		year -= 1;
	result += (year - 1968) / 4;
	result -= (year - 1900) / 100;
	result += (year - 1600) / 400;
	result += t->tm_mday;
	result -= 1;
	result *= 24;
	result += t->tm_hour;
	result *= 60;
	result += t->tm_min;
	result *= 60;
	result += t->tm_sec;
	return(result);
}

/*------------------------------------------------------------*/
//Get time in time_t format
time_t time(time_t *t)
{
    struct tm tm_time;
    get_tmtime(&tm_time);
    time_t time = mktime(&tm_time);
    if(t) *t=time;
    return time;
}


