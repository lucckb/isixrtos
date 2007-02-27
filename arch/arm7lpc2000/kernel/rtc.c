#include <isix/config.h>
#include <isix/types.h>
#include <isix/time.h>
#include <asm/lpc214x.h>

/*--------------------------------------------*/
//PREINT RTC DIVIDER
#define RTC_PREINT (CONFIG_PCLK/32768 - 1)
//PREFRAC RTC DIVIDER
#define RTC_PREFRAC (CONFIG_PCLK - (RTC_PREINT+1)*32768)

#define CCR_CTCRST 0x2
#define CCR_CLKEN  0x1
#define CCR_EXTOSC (1<<4)

/*--------------------------------------------*/
//Get current time in timeval struct
void get_tmtime(struct tm *tmtime)
{
   register u32 a,b,c;
   do
   {
        a = CTIME1;
        b = CTIME2;
        c = CTIME1;
   }
   while(a!=c);
   tmtime->tm_sec = a & 0x3f;
   tmtime->tm_min = (a >> 8) & 0x3f;
   tmtime->tm_hour = (a >> 16) & 0x1f;
   tmtime->tm_mday = b & 0x1f;
   tmtime->tm_mon =  (b >> 8) & 0x0f;
   tmtime->tm_year = (b >> 16) & 0x0fff;
   tmtime->tm_wday = (a>>24) & 0x07;
   tmtime->tm_yday = DOY;
   
}


/*--------------------------------------------*/
//Set current time in timeval value
void set_tmtime(const struct tm *tmtime)
{
    //Reset and disable timer
    CCR = CCR_CTCRST;
    CCR = 0;
    SEC = tmtime->tm_sec;
    MIN = tmtime->tm_min;
    HOUR = tmtime->tm_hour;
    DOM = tmtime->tm_mday;
    MONTH = tmtime->tm_mon;
    YEAR = tmtime->tm_year;
    DOW = tmtime->tm_wday;
    DOY = tmtime->tm_yday;
    //Enable timer again
    CCR = CCR_CLKEN;
}

/*--------------------------------------------*/
//Set current time in timeval value
void rtc_clock_init(void)
{
    //Use internal oscilator
    PREINT = RTC_PREINT;
    PREFRAC = RTC_PREFRAC;
    //Reset Clock
	CCR = CCR_CTCRST;
    CCR = 0;
    //Setup 1 January 2000
    SEC = MIN = HOUR = 0;
    DOM = MONTH = 1;
    YEAR = 2000;
    DOW = 6;
    DOY = 1;
    //Enable RTC
	CCR = CCR_CLKEN;
}


