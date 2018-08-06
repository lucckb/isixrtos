#pragma once

/** Serial port uart early console driver */
namespace periph::drivers::uart_early {

	/** Initialize the UART
	 * @param[in] name Device name
	 * @param[in] baudrate Baudrate
	 * @return initialization error code
	 */
	int open( const char *name, unsigned baudrate );

	/** Put char uart
	 * @param[in] ch Input char
	 * @return char
	 */
	int putc( int ch );

	/** Get char from the serial port
	 * @return char read from the uart */
	int getc();

	/** Check if new data avail on the RX device
	 * @return Error code or new char avail
	 */
	int isc();

	/** Put string over the serial console
	 * @param[in] str Input string
	 * @return error code
	 */
	int puts( const char str[] );

}


