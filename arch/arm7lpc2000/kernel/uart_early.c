#include <isix/config.h>
#include <isix/types.h>
#include <asm/uart_early.h>
#include <asm/lpc214x.h>

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

//Wyznaczenie predkosci
#define UART_BAUD(baud) (unsigned short)(CONFIG_PCLK/(baud*16.0) + 0.5)


/* Inicjalizacja Uart0 */
void uart_early_init(void)
{
	const unsigned short baud_rate = UART_BAUD(115200);
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


//Nadawanie znaku
void uart_early_putchar(char c)
{
	if(c=='\n') uart_early_putchar('\r');
	//Czekaj az bedzie mozna zapisac do bufora nadajnika
	while(!(U0LSR & U0LSR_THRE));
	//Wyslij znak i przejdz do nst znaku
	U0THR = c;
}


//Pobierz znak (tylko debuging)
int uart_early_getchar(void)
{
        
}