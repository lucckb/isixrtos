/*
 * Board support shared by the coroutine examples: console and crash hooks.
 *
 * Author: Lucjan Bryndza
 */
#pragma once

namespace examples {

//! Route dbprintf to the board debug UART (115200 8N1)
void console_init();

}
