#include <stdarg.h>
#include <isix/config.h>
#include <asm/lpc214x.h>
#include <isix/scheduler.h>

//Ustawienia kontrolera VIC
#define TXD0_P00_SEL (1<<0) 
#define RXD0_P01_SEL (1<<2)


#define U0LCR_8Bit_Data 3
#define U0LCR_1Bit_Stop 0
#define U0LCR_No_Parity 0
#define U0FCR_14Char_Fifo (3<<6)
#define U0FCR_FIFO_Enable 0x1
#define U0LSR_THRE 0x20
#define U0LSR_RDR 0x1 
#define U0LCR_Divisor_Latch_Access_Bit 0x80U



#include <stdarg.h>
#include <string.h>

//Printf buf
#define SCRATCH 80

#define kputchar putc


/* Inicjalizacja Uart0 */
void printk_init(unsigned short baud_rate)
{
	//Wybor RXD i TXD jako funkcja alternatywna
	PINSEL0 |= TXD0_P00_SEL | RXD0_P01_SEL;
	//Ustawianie predkosci transmisji
	U0LCR = U0LCR_Divisor_Latch_Access_Bit;
	//Ustaw predkosci transmisji
	U0DLL = (unsigned char)baud_rate;
	U0DLM = (unsigned char)(baud_rate>>8);
	//Ustawienie 8,n,1
	U0LCR = U0LCR_8Bit_Data | U0LCR_1Bit_Stop | U0LCR_No_Parity;
	//Wlacz fifo
	U0FCR = U0FCR_FIFO_Enable |	U0FCR_14Char_Fifo;
	//Wylacz przerwania i kasuj flagi przerwan
	U0IER = 0;
	U0IIR = 0;
	//Wszystkie znaczniki odebrania znaku
	U0LSR = 0;
}


//Nadawanie znak
static void putc(char c)
{
	if(c=='\n') putc('\r');
	//Czekaj az bedzie mozna zapisac do bufora nadajnika
	while(!(U0LSR & U0LSR_THRE));
	//Wyslij znak i przejdz do nst znaku
	U0THR = c;
}

#define SCRATCH 80	//32Bits go up to 4GB + 1 Byte for \0

//Spare some program space by making a comment of all not used format flag lines
#define USE_LONG 	// %lx, %Lu and so on, else only 16 bit integer is allowed
//#define USE_OCTAL	// %o, %O Octal output. Who needs this ?
#define USE_STRING      // %s, %S Strings as parameters
#define USE_CHAR	// %c, %C Chars as parameters
#define USE_INTEGER	// %i, %I Remove this format flag. %d, %D does the same
#define USE_HEX		// %x, %X Hexadezimal output
#define USE_UPPERHEX	// %x, %X outputs A,B,C... else a,b,c...
#ifndef USE_HEX
 #undef USE_UPPERHEX    // ;)
#endif
#define USE_UPPER	// uncommenting this removes %C,%D,%I,%O,%S,%U,%X and %L..
                        // only lowercase format flags are used
#define PADDING         //SPACE and ZERO padding



#define myputchar putc


void printk(char const *format, ...)
{
  unsigned char scratch[SCRATCH];
  unsigned char format_flag;
  unsigned short base;
  unsigned char *ptr;
  unsigned char issigned=0;
  va_list ap;

#ifdef USE_LONG
  unsigned char islong=0;
  unsigned long u_val=0;
  long s_val=0;
#else
  unsigned int u_val=0;
  int s_val=0;
#endif

  unsigned char fill;
  unsigned char width;

  //Lock scheduler
  sched_lock();
  
  va_start (ap, format);
  for (;;){
    while ((format_flag = *(format++)) != '%')
    {      // Until '%' or '\0' 
      if (!format_flag){ va_end (ap); sched_unlock(); return;}
      myputchar(format_flag);
    }

    issigned=0; //default unsigned
    base = 10;

    format_flag = *format++; //get char after '%'

#ifdef PADDING
    width=0; //no formatting
    fill=0;  //no formatting
    if(format_flag=='0' || format_flag==' ') //SPACE or ZERO padding  ?
     {
      fill=format_flag;
      format_flag = *format++; //get char after padding char
      if(format_flag>='0' && format_flag<='9')
       {
        width=format_flag-'0';
        format_flag = *format++; //get char after width char
       }
     }
#endif

#ifdef USE_LONG
    islong=0; //default int value
#ifdef USE_UPPER
    if(format_flag=='l' || format_flag=='L') //Long value 
#else
    if(format_flag=='l') //Long value 
#endif
     {
      islong=1;
      format_flag = *format++; //get char after 'l' or 'L'
     }
#endif

    switch (format_flag)
    {
#ifdef USE_CHAR
    case 'c':
#ifdef USE_UPPER
    case 'C':
#endif
      format_flag = va_arg(ap,int);
      // no break -> run into default
#endif

    default:
      myputchar(format_flag);
      continue;

#ifdef USE_STRING
#ifdef USE_UPPER
    case 'S':
#endif
    case 's':
      ptr = (unsigned char*)va_arg(ap,char *);
      while(*ptr) { myputchar(*ptr); ptr++; }
      continue;
#endif

#ifdef USE_OCTAL
    case 'o':
#ifdef USE_UPPER
    case 'O':
#endif
      base = 8;
      myputchar('0');
      goto CONVERSION_LOOP;
#endif

#ifdef USE_INTEGER //don't use %i, is same as %d
    case 'i':
#ifdef USE_UPPER
    case 'I':
#endif
#endif
    case 'd':
#ifdef USE_UPPER
    case 'D':
#endif
      issigned=1;
      // no break -> run into next case
    case 'u':
#ifdef USE_UPPER
    case 'U':
#endif

//don't insert some case below this if USE_HEX is undefined !
//or put       goto CONVERSION_LOOP;  before next case.
#ifdef USE_HEX
      goto CONVERSION_LOOP;
    case 'x':
#ifdef USE_UPPER
    case 'X':
#endif
      base = 16;
#endif

    CONVERSION_LOOP:

      if(issigned) //Signed types
       {
#ifdef USE_LONG
        if(islong) { s_val = va_arg(ap,long); }
        else { s_val = va_arg(ap,int); }
#else
        s_val = va_arg(ap,int);
#endif

        if(s_val < 0) //Value negativ ?
         {
          s_val = - s_val; //Make it positiv
          myputchar('-');    //Output sign
         }

        u_val = (unsigned long)s_val;
       }
      else //Unsigned types
       {
#ifdef USE_LONG
        if(islong) { u_val = va_arg(ap,unsigned long); }
        else { u_val = va_arg(ap,unsigned int); }
#else
        u_val = va_arg(ap,unsigned int);
#endif
       }
    
      ptr = scratch + SCRATCH;
      *--ptr = 0;
      do
       {
        char ch = u_val % base + '0';
#ifdef USE_HEX
        if (ch > '9')
         {
          ch += 'a' - '9' - 1;
#ifdef USE_UPPERHEX
          ch-=0x20;
#endif
         }
#endif          
        *--ptr = ch;
        u_val /= base;

#ifdef PADDING
        if(width) width--; //calculate number of padding chars
#endif
      } while (u_val);

#ifdef PADDING
     while(width--) *--ptr = fill; //insert padding chars		      
#endif

      while(*ptr) { myputchar(*ptr); ptr++; }
    }
  }
  //Unlock scheduler
   sched_unlock(); 
}

