/*
 * Awaitable USART driver written on top of the coroutine primitives.
 *
 * Reception: the RXNE interrupt writes bytes into a stream_buffer, a coroutine
 * reads them with a timeout. Transmission: the TXE interrupt feeds the bytes and
 * signals an event when the buffer is sent. A write that times out or is
 * canceled stops the interrupt before it returns, so the caller's buffer is
 * never touched afterwards.
 *
 * USART6 (PC6 TX, PC7 RX) is used because USART2 carries the debug console.
 * With c_loopback the USART runs in single-wire mode, every transmitted byte is
 * received back and the example needs no wiring: it sends test strings and
 * checks them. Set it to false and connect a terminal for a real echo.
 *
 * Author: Lucjan Bryndza
 */
#define CONFIG_ISIX_CPP_COROUTINES 1

#include "example_support.hpp"

#include <isix.h>
#include <array>
#include <cstring>
#include <span>
#include <foundation/sys/dbglog.h>
#include <isix/arch/irq.h>
#include <isix/arch/isr_vectors.h>
#include <stm32_ll_bus.h>
#include <stm32_ll_gpio.h>
#include <stm32_ll_usart.h>

namespace {

constexpr bool c_loopback = true;
constexpr unsigned c_baudrate = 115200;

class async_uart {
public:
	async_uart(isix::co::scheduler& sched)
		: m_rx(sched), m_tx_done(sched) {}

	void start()
	{
		LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOC);
		LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_USART6);
		for (const auto pin : { LL_GPIO_PIN_6, LL_GPIO_PIN_7 }) {
			LL_GPIO_SetPinMode(GPIOC, pin, LL_GPIO_MODE_ALTERNATE);
			LL_GPIO_SetAFPin_0_7(GPIOC, pin, LL_GPIO_AF_8);
			LL_GPIO_SetPinSpeed(GPIOC, pin, LL_GPIO_SPEED_FREQ_MEDIUM);
			LL_GPIO_SetPinPull(GPIOC, pin, LL_GPIO_PULL_UP);
		}
		if (c_loopback) {
			// The single wire is shared by the transmitter and the receiver
			LL_GPIO_SetPinOutputType(GPIOC, LL_GPIO_PIN_6, LL_GPIO_OUTPUT_OPENDRAIN);
		}
		LL_USART_InitTypeDef cfg {
			c_baudrate, LL_USART_DATAWIDTH_8B, LL_USART_STOPBITS_1, LL_USART_PARITY_NONE,
			LL_USART_DIRECTION_TX_RX, LL_USART_HWCONTROL_NONE, LL_USART_OVERSAMPLING_16
		};
		LL_USART_Init(USART6, &cfg);
		if (c_loopback) {
			LL_USART_EnableHalfDuplex(USART6);
		}
		LL_USART_EnableIT_RXNE(USART6);
		LL_USART_Enable(USART6);
		isix::set_irq_priority(USART6_IRQn, { 0, 0 });
		isix::request_irq(USART6_IRQn);
	}

	//! Wait for at least min_bytes (and at most buf.size()) bytes
	auto read(std::span<std::byte> buf, std::size_t min_bytes, ostick_t timeout)
	{
		return m_rx.read(buf, min_bytes, timeout);
	}

	//! Send a buffer; the buffer must stay valid until the returned task has finished
	isix::co::task<int> write(std::span<const std::byte> data)
	{
		// A signal left by an aborted write must not complete this one
		co_await m_tx_done.wait_for(ISIX_TIME_DONTWAIT);
		isix::enter_critical();
		m_tx = data;
		m_tx_pos = 0;
		isix::exit_critical();
		LL_USART_EnableIT_TXE(USART6);
		const int rc = co_await m_tx_done.wait_for(ISIX_TIME_INFINITE);
		if (rc != ISIX_EOK) {
			// Timeout or cancel: silence the interrupt before the buffer goes away
			isix::enter_critical();
			LL_USART_DisableIT_TXE(USART6);
			isix::exit_critical();
		}
		co_return rc;
	}

	unsigned overruns() const noexcept { return m_overruns; }

	void irq()
	{
		if (LL_USART_IsActiveFlag_ORE(USART6)) {
			LL_USART_ReceiveData8(USART6);
			++m_overruns;
		}
		if (LL_USART_IsActiveFlag_RXNE(USART6)) {
			const auto byte = static_cast<std::byte>(LL_USART_ReceiveData8(USART6));
			m_rx.write_isr({ &byte, 1 });
		}
		if (LL_USART_IsEnabledIT_TXE(USART6) && LL_USART_IsActiveFlag_TXE(USART6)) {
			if (m_tx_pos < m_tx.size()) {
				LL_USART_TransmitData8(USART6, static_cast<uint8_t>(m_tx[m_tx_pos++]));
			} else {
				LL_USART_DisableIT_TXE(USART6);
				m_tx_done.set_isr();
			}
		}
	}

private:
	isix::co::stream_buffer<256> m_rx;
	isix::co::event m_tx_done;
	std::span<const std::byte> m_tx {};
	std::size_t m_tx_pos {};
	volatile unsigned m_overruns {};
};

async_uart* g_uart {};

isix::co::task<void> loopback_test(async_uart& uart)
{
	constexpr char c_text[] = "coroutines over a USART";
	for (unsigned round = 1;; ++round) {
		const auto data = std::as_bytes(std::span{ c_text, sizeof(c_text) - 1 });
		auto sent = uart.write(data);
		const auto wr = co_await isix::co::with_timeout(sent, isix::ms2tick(500));
		std::array<std::byte, 64> in {};
		const auto rd = co_await uart.read(in, data.size(), isix::ms2tick(500));
		const bool match = rd && *rd == data.size() && std::memcmp(in.data(), data.data(), data.size()) == 0;
		dbprintf("round %u: write %s, read %s, data %s", round,
			wr ? "ok" : isix_strerror(wr.error()), rd ? "ok" : isix_strerror(rd.error()),
			match ? "matches" : "differs");
		co_await isix::co::sleep_ms(1000);
	}
}

isix::co::task<void> echo(async_uart& uart)
{
	for (;;) {
		std::array<std::byte, 64> buf {};
		const auto rd = co_await uart.read(buf, 1, isix::ms2tick(1000));
		if (!rd) {
			dbprintf("idle");
			continue;
		}
		auto sent = uart.write(std::span<const std::byte>(buf.data(), *rd));
		const auto wr = co_await isix::co::with_timeout(sent, isix::ms2tick(1000));
		if (!wr) {
			dbprintf("write failed: %s", isix_strerror(wr.error()));
		}
	}
}

}

ISIX_ISR_VECTOR(usart6_isr_vector)
{
	if (g_uart) {
		g_uart->irq();
	}
}

int main()
{
	examples::console_init();
	static isix::co::scheduler_thread executor;
	if (executor.start(4096, 3) != ISIX_EOK) {
		dbprintf("cannot start the executor");
		return -1;
	}
	static async_uart uart(executor.sched());
	g_uart = &uart;
	uart.start();
	const int rc = c_loopback ? executor.post(loopback_test(uart)) : executor.post(echo(uart));
	if (rc != ISIX_EOK) {
		dbprintf("out of memory for the task");
	}
	isix_start_scheduler();
	return 0;
}
