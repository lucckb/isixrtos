/*
 * Two executors at different priorities: a high priority one samples a "device"
 * every 10 ms, a low priority one aggregates the samples. They talk through a
 * channel that belongs to the consumer, filled with the thread-safe try_send().
 *
 * Author: Lucjan Bryndza
 */
#define CONFIG_ISIX_CPP_COROUTINES 1

#include "example_support.hpp"

#include <isix.h>
#include <foundation/sys/dbglog.h>

namespace {

constexpr unsigned c_sample_period_ms = 10;
constexpr unsigned c_report_every = 100;

isix::co::task<void> sampler(isix::co::channel<int, 8>& out)
{
	int value = 0;
	for (;;) {
		co_await isix::co::sleep_ms(c_sample_period_ms);
		value = (value + 7) % 100;
		if (!out.try_send(value)) {
			dbprintf("sample %d dropped", value);
		}
	}
}

isix::co::task<void> aggregator(isix::co::channel<int, 8>& in)
{
	long sum = 0;
	unsigned count = 0;
	for (;;) {
		const auto sample = co_await in.recv();
		if (!sample) {
			co_return;
		}
		sum += *sample;
		if (++count == c_report_every) {
			dbprintf("average of %u samples: %ld", count, sum / static_cast<long>(count));
			sum = 0;
			count = 0;
		}
	}
}

}

int main()
{
	examples::console_init();
	static isix::co::scheduler_thread high;
	static isix::co::scheduler_thread low;
	if (high.start(4096, 2) != ISIX_EOK || low.start(4096, 5) != ISIX_EOK) {
		dbprintf("cannot start the executors");
		return -1;
	}
	// The channel belongs to the consumer's scheduler
	static isix::co::channel<int, 8> samples(low.sched());
	if (low.post(aggregator(samples)) != ISIX_EOK || high.post(sampler(samples)) != ISIX_EOK) {
		dbprintf("out of memory for the tasks");
	}
	isix_start_scheduler();
	return 0;
}
