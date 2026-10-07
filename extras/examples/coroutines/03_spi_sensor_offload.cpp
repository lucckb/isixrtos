/*
 * Blocking sensor access from a coroutine. The STM32F411E-DISCO has an L3GD20
 * gyroscope on SPI1. Its polled transaction function blocks, so it runs on a
 * worker thread while the scheduler keeps serving the other coroutines; the wait
 * is limited with with_timeout().
 *
 * Author: Lucjan Bryndza
 */
#define CONFIG_ISIX_CPP_COROUTINES 1

#include "example_support.hpp"

#include <isix.h>
#include <algorithm>
#include <array>
#include <span>
#include <foundation/sys/dbglog.h>
#include <stm32_ll_bus.h>
#include <stm32_ll_gpio.h>
#include <stm32_ll_spi.h>

namespace {

constexpr uint8_t c_reg_who_am_i = 0x0F;
constexpr uint8_t c_reg_ctrl1 = 0x20;
constexpr uint8_t c_reg_out_x_l = 0x28;
constexpr uint8_t c_who_am_i_values[] { 0xD3, 0xD4, 0xD7 };

void sensor_init()
{
	LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOA | LL_AHB1_GRP1_PERIPH_GPIOE);
	LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_SPI1);
	for (const auto pin : { LL_GPIO_PIN_5, LL_GPIO_PIN_6, LL_GPIO_PIN_7 }) {
		LL_GPIO_SetPinMode(GPIOA, pin, LL_GPIO_MODE_ALTERNATE);
		LL_GPIO_SetAFPin_0_7(GPIOA, pin, LL_GPIO_AF_5);
		LL_GPIO_SetPinSpeed(GPIOA, pin, LL_GPIO_SPEED_FREQ_HIGH);
	}
	// Chip select of the gyroscope is PE3, active low
	LL_GPIO_SetPinMode(GPIOE, LL_GPIO_PIN_3, LL_GPIO_MODE_OUTPUT);
	LL_GPIO_SetOutputPin(GPIOE, LL_GPIO_PIN_3);
	LL_SPI_InitTypeDef cfg {};
	cfg.TransferDirection = LL_SPI_FULL_DUPLEX;
	cfg.Mode = LL_SPI_MODE_MASTER;
	cfg.DataWidth = LL_SPI_DATAWIDTH_8BIT;
	cfg.ClockPolarity = LL_SPI_POLARITY_HIGH;
	cfg.ClockPhase = LL_SPI_PHASE_2EDGE;
	cfg.NSS = LL_SPI_NSS_SOFT;
	cfg.BaudRate = LL_SPI_BAUDRATEPRESCALER_DIV32;
	cfg.BitOrder = LL_SPI_MSB_FIRST;
	LL_SPI_Init(SPI1, &cfg);
	LL_SPI_Enable(SPI1);
}

uint8_t spi_byte(uint8_t out)
{
	while (!LL_SPI_IsActiveFlag_TXE(SPI1)) {
	}
	LL_SPI_TransmitData8(SPI1, out);
	while (!LL_SPI_IsActiveFlag_RXNE(SPI1)) {
	}
	return LL_SPI_ReceiveData8(SPI1);
}

//! Blocking transaction: bit 7 selects a read, bit 6 increments the address
void read_registers(uint8_t reg, std::span<uint8_t> out)
{
	LL_GPIO_ResetOutputPin(GPIOE, LL_GPIO_PIN_3);
	spi_byte(reg | 0xC0U);
	for (auto& b : out) {
		b = spi_byte(0);
	}
	LL_GPIO_SetOutputPin(GPIOE, LL_GPIO_PIN_3);
}

void write_register(uint8_t reg, uint8_t value)
{
	LL_GPIO_ResetOutputPin(GPIOE, LL_GPIO_PIN_3);
	spi_byte(reg);
	spi_byte(value);
	LL_GPIO_SetOutputPin(GPIOE, LL_GPIO_PIN_3);
}

struct sample {
	int16_t x, y, z;
};

isix::co::task<isix::co::result<sample>> read_sample(isix::co::worker_thread& worker)
{
	sample s {};
	auto job = [&s] {
		std::array<uint8_t, 6> raw {};
		read_registers(c_reg_out_x_l, raw);
		s.x = static_cast<int16_t>(raw[0] | (raw[1] << 8));
		s.y = static_cast<int16_t>(raw[2] | (raw[3] << 8));
		s.z = static_cast<int16_t>(raw[4] | (raw[5] << 8));
	};
	const int rc = co_await worker.run(job);
	if (rc != ISIX_EOK) {
		co_return std::unexpected(rc);
	}
	co_return s;
}

isix::co::task<void> monitor(isix::co::worker_thread& worker)
{
	uint8_t id {};
	auto probe = [&id] {
		read_registers(c_reg_who_am_i, std::span<uint8_t>(&id, 1));
		write_register(c_reg_ctrl1, 0x0F);
	};
	co_await worker.run(probe);
	const bool known = std::find(std::begin(c_who_am_i_values), std::end(c_who_am_i_values), id)
		!= std::end(c_who_am_i_values);
	dbprintf("WHO_AM_I %02x (%s)", id, known ? "L3GD20 family" : "unexpected");
	for (;;) {
		const auto r = co_await isix::co::with_timeout(read_sample(worker), isix::ms2tick(50));
		if (r && *r) {
			dbprintf("x %d y %d z %d", (*r)->x, (*r)->y, (*r)->z);
		} else {
			dbprintf("read failed: %s", isix_strerror(r ? r->error() : r.error()));
		}
		co_await isix::co::sleep_ms(500);
	}
}

isix::co::task<void> heartbeat()
{
	for (unsigned n = 1;; ++n) {
		co_await isix::co::sleep_ms(250);
		if (n % 8 == 0) {
			dbprintf("scheduler still responsive");
		}
	}
}

}

int main()
{
	examples::console_init();
	sensor_init();
	static isix::co::scheduler_thread executor;
	static isix::co::worker_thread worker(executor.sched());
	if (executor.start(4096, 3) != ISIX_EOK || worker.start(2048, 4) != ISIX_EOK) {
		dbprintf("cannot start the threads");
		return -1;
	}
	if (executor.post(monitor(worker)) != ISIX_EOK || executor.post(heartbeat()) != ISIX_EOK) {
		dbprintf("out of memory for the tasks");
	}
	isix_start_scheduler();
	return 0;
}
