/*
 * =====================================================================================
 *
 *       Filename:  ssd1306.cpp
 *
 *    Description:  SSD1306 display driver
 *
 *        Version:  1.0
 *        Created:  16.12.2017 22:29:26
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Lucjan Bryndza (LB), lbryndza.p@boff.pl
 *   Organization:  BoFF
 *
 * =====================================================================================
 */
#include <foundation/drv/lcd/ssd1306.hpp>

namespace fnd {
namespace drv {
namespace lcd {


// Enable the display
int ssd1306::enable(bool en) noexcept
{
}

// SSD set display brightness
int ssd1306::brightness(bright_t value) noexcept
{
}

// SSD put character on selected position
int ssd1306::putc(char ch) noexcept
{
}

// Clear the screen
int ssd1306::clear() noexcept
{
}


// Set display at position
int ssd1306::setpos(int x, int y) noexcept
{
}

// Go to new line using font
int ssd1306::endl() noexcept
{
}

}}}

