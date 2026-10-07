/*
 * Copyright (c) 2025 Lucjan Bryndza
 *
 * Heap allocator robustness against kill and suspend of the allocating task
 */
#include <unity.h>
#include <unity_fixture.h>
#include <isix.h>
#include <isix/prv/test_hooks.h>
#include <utils/test_prio.hpp>
#include <cstring>

namespace {
	test_utils::task_pool tk_tasks;
	ossem_t s_hook_sem;
	ossem_t s_go;
	ostask_t s_victim;
	ostask_t s_worker;
	volatile bool s_arm;
	volatile bool s_in_heap;
	volatile bool s_v_done;
	volatile bool s_h_done;
	volatile bool s_z_done;
	void* volatile s_slots[8];

	// Runs inside the allocator section, must neither allocate nor block
	void heap_hook(isix_test_point point, void*)
	{
		if (!(s_arm || s_in_heap) || isix_task_self() != s_victim) {
			return;
		}
		if (point == isix_tp_heap_locked) {
			s_in_heap = true;
			s_arm = false;
			isix_sem_signal(s_hook_sem);
		} else if (point == isix_tp_heap_unlocking) {
			s_in_heap = false;
		}
	}

	// Allocates and frees one block, the hook fires on the free
	void victim_func(void*)
	{
		void* const p = isix_alloc(48);
		s_arm = true;
		isix_free(p);
		s_v_done = true;
		for (;;) {
			isix_wait_ms(1000);
		}
	}

	// Allocates a block as soon as it is released
	void holder_func(void*)
	{
		isix_sem_wait(s_go, 1000);
		void* const p = isix_alloc(32);
		s_h_done = true;
		isix_free(p);
	}

	// Finishes right after it is released, idle frees its resources
	void zombie_func(void*)
	{
		isix_sem_wait(s_go, 1000);
		s_z_done = true;
	}

	// Never blocks, allocates and frees random blocks until killed
	void worker_func(void*)
	{
		unsigned seed = 12345U;
		for (;;) {
			seed = seed * 1103515245U + 12345U;
			const auto idx = (seed >> 16) % 8U;
			void* const old = s_slots[idx];
			if (old) {
				s_slots[idx] = nullptr;
				isix_free(old);
			} else {
				const auto size = 16U + ((seed >> 8) % 49U);
				s_slots[idx] = isix_alloc(size);
			}
		}
	}

	void reset_state()
	{
		s_arm = false;
		s_in_heap = false;
		s_v_done = false;
		s_h_done = false;
		s_z_done = false;
		s_victim = nullptr;
		s_hook_sem = isix_sem_create_limited(nullptr, 0, 1);
		s_go = isix_sem_create_limited(nullptr, 0, 1);
	}
}

TEST_GROUP(heap);
TEST_SETUP(heap)
{
	reset_state();
}

TEST_TEAR_DOWN(heap)
{
	_isixp_test_hook = nullptr;
	if (s_victim) {
		isix_task_resume(s_victim);
	}
	tk_tasks.release();
	if (s_worker) {
		isix_task_kill(s_worker);
		isix_task_unref(s_worker);
		s_worker = nullptr;
	}
	for (auto& slot : s_slots) {
		void* const p = slot;
		slot = nullptr;
		isix_free(p);
	}
	isix_wait_ms(20);
	if (s_hook_sem) { isix_sem_destroy(s_hook_sem); }
	if (s_go) { isix_sem_destroy(s_go); }
	s_hook_sem = nullptr;
	s_go = nullptr;
	s_victim = nullptr;
	test_utils::restore_test_prio();
}

TEST(heap, check_after_alloc_free)
{
	TEST_ASSERT_EQUAL_INT(0, _isixp_heap_check());
	void* blocks[6] {};
	constexpr size_t sizes[] { 16, 100, 7, 300, 48, 1000 };
	for (auto i = 0U; i < 6U; ++i) {
		blocks[i] = isix_alloc(sizes[i]);
		TEST_ASSERT_NOT_NULL(blocks[i]);
	}
	TEST_ASSERT_EQUAL_INT(0, _isixp_heap_check());
	for (auto i : { 1U, 3U, 0U, 5U, 2U, 4U }) {
		isix_free(blocks[i]);
		TEST_ASSERT_EQUAL_INT(0, _isixp_heap_check());
	}
}

TEST(heap, kill_task_inside_allocator)
{
	TEST_ASSERT_NOT_NULL(s_hook_sem);
	isix_wait_ms(20);
	isix_memory_stat_t before {};
	isix_heap_stats(&before);
	s_victim = tk_tasks.spawn(victim_func, nullptr, 3, 1024);
	TEST_ASSERT_NOT_NULL(s_victim);
	_isixp_test_hook = heap_hook;
	const auto wret = isix_sem_wait(s_hook_sem, 1000);
	const bool inside = s_in_heap;
	isix_task_kill(s_victim);
	_isixp_test_hook = nullptr;
	tk_tasks.release();
	s_victim = nullptr;
	isix_wait_ms(20);
	TEST_ASSERT_EQUAL(ISIX_EOK, wret);
	TEST_ASSERT_FALSE(inside);
	TEST_ASSERT_EQUAL_INT(0, _isixp_heap_check());
	void* const p = isix_alloc(64);
	TEST_ASSERT_NOT_NULL(p);
	isix_free(p);
	isix_memory_stat_t after {};
	isix_heap_stats(&after);
	TEST_ASSERT_EQUAL_UINT(before.free, after.free);
}

TEST(heap, suspend_holder_does_not_block_alloc)
{
	TEST_ASSERT_NOT_NULL(s_hook_sem);
	TEST_ASSERT_NOT_NULL(tk_tasks.spawn(holder_func, nullptr, 1, 1024));
	s_victim = tk_tasks.spawn(victim_func, nullptr, 3, 1024);
	TEST_ASSERT_NOT_NULL(s_victim);
	_isixp_test_hook = heap_hook;
	const auto wret = isix_sem_wait(s_hook_sem, 1000);
	isix_task_suspend(s_victim);
	isix_sem_signal(s_go);
	isix_wait_ms(20);
	const bool done = s_h_done;
	isix_task_resume(s_victim);
	isix_wait_ms(20);
	_isixp_test_hook = nullptr;
	TEST_ASSERT_EQUAL(ISIX_EOK, wret);
	TEST_ASSERT_TRUE(done);
	TEST_ASSERT_TRUE(s_h_done);
	TEST_ASSERT_EQUAL_INT(0, _isixp_heap_check());
}

TEST(heap, kill_stress_consistency)
{
	int failed_iter = -1;
	int failed_rule = 0;
	for (auto i = 0; i < 100; ++i) {
		s_worker = isix_task_create(worker_func, nullptr, 1024, 3, isix_task_flag_ref);
		if (!s_worker) {
			failed_iter = i;
			failed_rule = 1;
			break;
		}
		isix_wait_ms(1 + i % 3);
		isix_task_kill(s_worker);
		isix_task_unref(s_worker);
		s_worker = nullptr;
		for (auto& slot : s_slots) {
			void* const p = slot;
			slot = nullptr;
			isix_free(p);
		}
		isix_wait_ms(2);
		const auto rule = _isixp_heap_check();
		if (rule != 0) {
			failed_iter = i;
			failed_rule = rule;
			break;
		}
	}
	if (failed_iter >= 0) {
		tiny_printf("heap check failed in iteration %d rule %d\r\n", failed_iter, failed_rule);
	}
	TEST_ASSERT_EQUAL_INT(-1, failed_iter);
}

namespace {
	constexpr unsigned char pattern(size_t i)
	{
		return static_cast<unsigned char>(0xA5U ^ (i * 7U));
	}

	void fill_pattern(void* p, size_t n)
	{
		auto* const d = static_cast<unsigned char*>(p);
		for (size_t i = 0; i < n; ++i) {
			d[i] = pattern(i);
		}
	}

	bool check_pattern(const void* p, size_t n)
	{
		const auto* const d = static_cast<const unsigned char*>(p);
		for (size_t i = 0; i < n; ++i) {
			if (d[i] != pattern(i)) {
				return false;
			}
		}
		return true;
	}
}

TEST(heap, realloc_grow_shrink_preserves_content)
{
	void* p = isix_alloc(100);
	TEST_ASSERT_NOT_NULL(p);
	fill_pattern(p, 100);
	// Shrink in place
	p = isix_realloc(p, 40);
	TEST_ASSERT_NOT_NULL(p);
	TEST_ASSERT_TRUE(check_pattern(p, 40));
	TEST_ASSERT_EQUAL_INT(0, _isixp_heap_check());
	// Grow in place or by relocation, a neighbour allocated before blocks one of them
	void* const blocker = isix_alloc(16);
	TEST_ASSERT_NOT_NULL(blocker);
	p = isix_realloc(p, 300);
	TEST_ASSERT_NOT_NULL(p);
	TEST_ASSERT_TRUE(check_pattern(p, 40));
	TEST_ASSERT_EQUAL_INT(0, _isixp_heap_check());
	fill_pattern(p, 300);
	p = isix_realloc(p, 900);
	TEST_ASSERT_NOT_NULL(p);
	TEST_ASSERT_TRUE(check_pattern(p, 300));
	TEST_ASSERT_EQUAL_INT(0, _isixp_heap_check());
	// Forced relocation: the block is surrounded by used blocks
	void* const q = isix_alloc(64);
	TEST_ASSERT_NOT_NULL(q);
	void* const r = isix_alloc(64);
	TEST_ASSERT_NOT_NULL(r);
	fill_pattern(q, 64);
	void* const q2 = isix_realloc(q, 512);
	TEST_ASSERT_NOT_NULL(q2);
	TEST_ASSERT_TRUE(check_pattern(q2, 64));
	TEST_ASSERT_EQUAL_INT(0, _isixp_heap_check());
	// Zero size frees the block
	TEST_ASSERT_NULL(isix_realloc(q2, 0));
	isix_free(r);
	isix_free(p);
	isix_free(blocker);
	TEST_ASSERT_EQUAL_INT(0, _isixp_heap_check());
}

TEST(heap, stats_balance)
{
	isix_memory_stat_t before {};
	isix_heap_stats(&before);
	void* p = isix_alloc(50);
	TEST_ASSERT_NOT_NULL(p);
	void* const blocker = isix_alloc(32);
	TEST_ASSERT_NOT_NULL(blocker);
	for (const size_t size : { 120U, 30U, 400U, 64U }) {
		p = isix_realloc(p, size);
		TEST_ASSERT_NOT_NULL(p);
	}
	isix_free(p);
	isix_free(blocker);
	isix_memory_stat_t after {};
	isix_heap_stats(&after);
	TEST_ASSERT_EQUAL_UINT(before.used, after.used);
	TEST_ASSERT_EQUAL_UINT(before.free, after.free);
	TEST_ASSERT_EQUAL_INT(0, _isixp_heap_check());
}

TEST(heap, idle_cleanup_with_suspended_holder)
{
	TEST_ASSERT_NOT_NULL(s_hook_sem);
	TEST_ASSERT_NOT_NULL(tk_tasks.spawn(zombie_func, nullptr, 2, 1024));
	s_victim = tk_tasks.spawn(victim_func, nullptr, 3, 1024);
	TEST_ASSERT_NOT_NULL(s_victim);
	_isixp_test_hook = heap_hook;
	const auto wret = isix_sem_wait(s_hook_sem, 1000);
	isix_task_suspend(s_victim);
	isix_sem_signal(s_go);
	isix_wait_ms(20);
	isix_task_resume(s_victim);
	isix_wait_ms(20);
	_isixp_test_hook = nullptr;
	TEST_ASSERT_EQUAL(ISIX_EOK, wret);
	TEST_ASSERT_TRUE(s_z_done);
	TEST_ASSERT_EQUAL_INT(0, _isixp_heap_check());
}

TEST_GROUP_RUNNER(heap)
{
	RUN_TEST_CASE(heap, check_after_alloc_free);
	RUN_TEST_CASE(heap, kill_task_inside_allocator);
	RUN_TEST_CASE(heap, suspend_holder_does_not_block_alloc);
	RUN_TEST_CASE(heap, kill_stress_consistency);
	RUN_TEST_CASE(heap, idle_cleanup_with_suspended_holder);
	RUN_TEST_CASE(heap, realloc_grow_shrink_preserves_content);
	RUN_TEST_CASE(heap, stats_balance);
}
