/*
 * Copyright (c) 2026 Lucjan Bryndza
 *
 * IRQ priority encoding and kernel interrupt masking
 */
#include <unity.h>
#include <unity_fixture.h>
#include <isix.h>
#include <isix/config.h>
#include <isix/arch/irq_platform.h>
#include <isix/arch/irq.h>
#include <isix/arch/isr_vectors.h>
#include "timer_interrupt.hpp"
#include <stm32_ll_tim.h>
#include <array>
#include <cstdint>

TEST_GROUP(irq_priority);

namespace {
	// The test owned interrupt, raised only by software
	constexpr auto c_irq = TIM5_IRQn;
	constexpr auto c_prigroup7 = isix_cortexm_group_pri7;
	volatile uint32_t s_irq_count;

	uint32_t read_basepri()
	{
		uint32_t val;
		asm volatile("mrs %0, BASEPRI" : "=r"(val));
		return val;
	}

	// Lets a pending interrupt that is not masked be taken
	void irq_settle()
	{
		for (auto i = 0; i < 64; ++i) {
			asm volatile("nop");
		}
		asm volatile("isb" ::: "memory");
	}

	void irq_restore_state()
	{
		isix::mask_irq_restore_priority(0);
		isix::free_irq(c_irq);
		isix::clear_irq_pending(c_irq);
		isix::set_irq_priority_group(c_prigroup7);
		isix::set_raw_irq_priority(c_irq, 0xF0);
		s_irq_count = 0;
	}
}

ISIX_ISR_VECTOR(tim5_isr_vector)
{
	s_irq_count = s_irq_count + 1;
}

TEST_SETUP(irq_priority)
{
	irq_restore_state();
}

TEST_TEAR_DOWN(irq_priority)
{
	irq_restore_state();
}

TEST(irq_priority, set_priority_writes_aligned_ipr)
{
	struct entry { isix_irq_prio_t prio; uint8_t expected; };
	constexpr std::array<entry, 4> c_tab {{
		{{1, 7}, 0xF0}, {{0, 7}, 0x70}, {{1, 0}, 0x80}, {{0, 0}, 0x00}
	}};
	std::array<uint8_t, c_tab.size()> got {};
	for (size_t i = 0; i < c_tab.size(); ++i) {
		isix::set_irq_priority(c_irq, c_tab[i].prio);
		got[i] = isix::get_raw_irq_priority(c_irq);
	}
	for (size_t i = 0; i < c_tab.size(); ++i) {
		TEST_ASSERT_EQUAL_HEX8(c_tab[i].expected, got[i]);
	}
}

TEST(irq_priority, set_priority_system_handler)
{
	constexpr auto c_handler = isix_cortexm_irq_memory_management;
	const auto old = isix::get_raw_irq_priority(c_handler);
	isix::set_irq_priority(c_handler, {1, 7});
	const auto got = isix::get_raw_irq_priority(c_handler);
	isix::set_raw_irq_priority(c_handler, old);
	TEST_ASSERT_EQUAL_HEX8(0xF0, got);
}

TEST(irq_priority, to_raw_matches_register)
{
	struct entry {
		isix_cortexm_prigroup group;
		isix_irq_prio_t prio;
		uint8_t expected;
	};
	constexpr std::array<entry, 6> c_tab {{
		{isix_cortexm_group_pri7_1, {1, 7}, 0x10},
		{isix_cortexm_group_pri7_1, {15, 0}, 0xF0},
		{isix_cortexm_group_pri7_4, {5, 0}, 0x50},
		{isix_cortexm_group_pri7_5, {3, 1}, 0x70},
		{isix_cortexm_group_pri7_6, {2, 1}, 0x90},
		{isix_cortexm_group_pri_none, {0, 15}, 0xF0},
	}};
	std::array<uint8_t, c_tab.size()> raw {};
	std::array<uint8_t, c_tab.size()> reg {};
	for (size_t i = 0; i < c_tab.size(); ++i) {
		isix::set_irq_priority_group(c_tab[i].group);
		raw[i] = isix::irq_priority_to_raw_priority(c_tab[i].prio);
		isix::set_irq_priority(c_irq, c_tab[i].prio);
		reg[i] = isix::get_raw_irq_priority(c_irq);
	}
	isix::set_irq_priority_group(c_prigroup7);
	for (size_t i = 0; i < c_tab.size(); ++i) {
		TEST_ASSERT_EQUAL_HEX8(c_tab[i].expected, raw[i]);
		TEST_ASSERT_EQUAL_HEX8(raw[i], reg[i]);
	}
}

TEST(irq_priority, mask_save_sets_basepri)
{
	const auto old = isix::mask_irq_save_priority({1, 0});
	const auto masked = read_basepri();
	isix::mask_irq_restore_priority(old);
	const auto restored = read_basepri();
	TEST_ASSERT_EQUAL_HEX32(0x80, masked);
	TEST_ASSERT_EQUAL_HEX32(old, restored);
}

TEST(irq_priority, mask_blocks_lower_group)
{
	isix::set_irq_priority(c_irq, {1, 7});
	isix::request_irq(c_irq);
	isix::mask_irq_priority({1, 0});
	isix::set_irq_pending(c_irq);
	irq_settle();
	const auto masked_count = s_irq_count;
	isix::umask_irq_priority();
	irq_settle();
	const auto released_count = s_irq_count;

	// An interrupt above the mask still runs
	isix::set_irq_priority(c_irq, {0, 0});
	isix::mask_irq_priority({1, 0});
	isix::set_irq_pending(c_irq);
	irq_settle();
	const auto above_count = s_irq_count;
	isix::umask_irq_priority();

	TEST_ASSERT_EQUAL_UINT32(0, masked_count);
	TEST_ASSERT_EQUAL_UINT32(1, released_count);
	TEST_ASSERT_EQUAL_UINT32(2, above_count);
}

TEST(irq_priority, kernel_critical_masks_irq_prigroup6)
{
	isix::set_irq_priority(c_irq, {0, 0});
	isix::request_irq(c_irq);
	isix_enter_critical();
	isix::set_irq_pending(c_irq);
	irq_settle();
	const auto inside = s_irq_count;
	isix_exit_critical();
	irq_settle();
	const auto outside = s_irq_count;
	TEST_ASSERT_EQUAL_UINT32(0, inside);
	TEST_ASSERT_EQUAL_UINT32(1, outside);
}

TEST(irq_priority, kernel_critical_masks_irq_prigroup3)
{
	isix::set_irq_priority_group(isix_cortexm_group_pri7_4);
	isix::set_irq_priority(c_irq, {1, 7});
	isix::request_irq(c_irq);
	isix_enter_critical();
	isix::set_irq_pending(c_irq);
	irq_settle();
	const auto inside = s_irq_count;
	isix_exit_critical();
	irq_settle();
	const auto outside = s_irq_count;
	isix::set_irq_priority_group(c_prigroup7);
	TEST_ASSERT_EQUAL_UINT32(0, inside);
	TEST_ASSERT_EQUAL_UINT32(1, outside);
}

TEST(irq_priority, test_timer_irq_is_kernel_masked)
{
	const auto started = tests::detail::periodic_timer_setup([] {}, 1000);
	const auto raw = isix::get_raw_irq_priority(TIM3_IRQn);
	tests::detail::periodic_timer_stop();
	TEST_ASSERT_TRUE(started);
	TEST_ASSERT_EQUAL_HEX8(0x00, raw);
	TEST_ASSERT_TRUE(isix::irq_raw_priority_is_kernel_masked(raw));
}

TEST(irq_priority, kernel_masked_helper)
{
	constexpr std::array<uint8_t, 3> c_raw6 {0x00, 0x70, 0xF0};
	constexpr std::array<uint8_t, 3> c_raw3 {0x00, 0x10, 0xF0};
	std::array<bool, 3> res6 {};
	std::array<bool, 3> res3 {};
	for (size_t i = 0; i < c_raw6.size(); ++i) {
		res6[i] = isix::irq_raw_priority_is_kernel_masked(c_raw6[i]);
	}
	isix::set_irq_priority_group(isix_cortexm_group_pri7_4);
	for (size_t i = 0; i < c_raw3.size(); ++i) {
		res3[i] = isix::irq_raw_priority_is_kernel_masked(c_raw3[i]);
	}
	isix::set_irq_priority_group(c_prigroup7);
	TEST_ASSERT_TRUE(res6[0]);
	TEST_ASSERT_TRUE(res6[1]);
	TEST_ASSERT_TRUE(res6[2]);
	TEST_ASSERT_FALSE(res3[0]);
	TEST_ASSERT_TRUE(res3[1]);
	TEST_ASSERT_TRUE(res3[2]);
}

TEST_GROUP_RUNNER(irq_priority)
{
	RUN_TEST_CASE(irq_priority, set_priority_writes_aligned_ipr);
	RUN_TEST_CASE(irq_priority, set_priority_system_handler);
	RUN_TEST_CASE(irq_priority, to_raw_matches_register);
	RUN_TEST_CASE(irq_priority, mask_save_sets_basepri);
	RUN_TEST_CASE(irq_priority, mask_blocks_lower_group);
	RUN_TEST_CASE(irq_priority, kernel_critical_masks_irq_prigroup6);
	RUN_TEST_CASE(irq_priority, kernel_critical_masks_irq_prigroup3);
	RUN_TEST_CASE(irq_priority, test_timer_irq_is_kernel_masked);
	RUN_TEST_CASE(irq_priority, kernel_masked_helper);
}
