/*
 * Coroutine frame allocation: global allocator hook, caller supplied static
 * storage and fixed block pools. Frames are never allocated on the hot path;
 * a failed allocation yields an empty task instead of a panic.
 *
 * Author: Lucjan Bryndza
 */
#pragma once

#if defined(CONFIG_ISIX_CPP_COROUTINES) && defined(__cplusplus)

#include <bit>
#include <cstddef>
#include <cstdint>

#include <isix/cpp/coroutine_scheduler.hpp>
#include <isix/memory.h>

namespace isix::co {

//! Memory source for coroutine frames
struct frame_allocator {
	void* (*alloc)(void* ctx, std::size_t size) noexcept;
	void (*free)(void* ctx, void* ptr, std::size_t size) noexcept;
	void* ctx;
};

namespace detail {

//! Stored in front of every frame, so a frame is released to the source it came from
struct frame_header {
	void (*free)(void* ctx, void* ptr, std::size_t size) noexcept;
	void* ctx;
};

inline constexpr std::size_t c_frame_header_size = sizeof(frame_header);
static_assert(c_frame_header_size % 8U == 0U);

inline void* heap_frame_alloc(void*, std::size_t size) noexcept { return isix_alloc(size); }
inline void heap_frame_free(void*, void* ptr, std::size_t) noexcept { isix_free(ptr); }

inline frame_allocator g_frame_allocator { &heap_frame_alloc, &heap_frame_free, nullptr };

//! Place the header at the start of a raw block and return the frame address
inline void* frame_begin(void* block, const frame_header& h) noexcept
{
	if (!block) {
		return nullptr;
	}
	auto* const hdr = static_cast<frame_header*>(block);
	*hdr = h;
	return hdr + 1;
}

inline void* frame_acquire(std::size_t size) noexcept
{
	const auto& a = g_frame_allocator;
	return frame_begin(a.alloc(a.ctx, size + c_frame_header_size), { a.free, a.ctx });
}

inline void frame_release(void* frame, std::size_t size) noexcept
{
	if (frame) {
		auto* const hdr = static_cast<frame_header*>(frame) - 1;
		hdr->free(hdr->ctx, hdr, size + c_frame_header_size);
	}
}

} // namespace detail

//! Replace the global frame source. Call before any coroutine is created.
inline void set_frame_allocator(const frame_allocator& a) noexcept
{
	detail::g_frame_allocator = a;
}

//! Restore the heap as the frame source
inline void reset_frame_allocator() noexcept
{
	detail::g_frame_allocator = { &detail::heap_frame_alloc, &detail::heap_frame_free, nullptr };
}

[[nodiscard]] inline frame_allocator get_frame_allocator() noexcept
{
	return detail::g_frame_allocator;
}

/**
 * Storage for the frame of one coroutine at a time. Pass it as the first
 * parameter of the coroutine function. Size includes the frame header.
 */
template<std::size_t Size>
class frame_storage final {
	static_assert(Size > detail::c_frame_header_size);
public:
	frame_storage() noexcept = default;
	frame_storage(const frame_storage&) = delete;
	frame_storage& operator=(const frame_storage&) = delete;

	[[nodiscard]] bool in_use() const noexcept { return m_used; }

	//! Frame bytes available for a coroutine
	static constexpr std::size_t capacity() noexcept { return Size - detail::c_frame_header_size; }

	[[nodiscard]] void* acquire(std::size_t size) noexcept
	{
		if (m_used || size > capacity()) {
			return nullptr;
		}
		m_used = true;
		return detail::frame_begin(m_buf, { &frame_storage::release, this });
	}

private:
	static void release(void* ctx, void*, std::size_t) noexcept
	{
		static_cast<frame_storage*>(ctx)->m_used = false;
	}

	alignas(8) std::byte m_buf[Size] {};
	bool m_used { false };
};

/**
 * Pool of Count blocks of BlockSize bytes (frame header included). Pass it as
 * the first parameter of the coroutine function. Safe to share between threads.
 */
template<std::size_t BlockSize, std::size_t Count>
class frame_pool final {
	static_assert(BlockSize > detail::c_frame_header_size && BlockSize % 8U == 0U);
	static_assert(Count > 0U && Count <= 32U);
public:
	frame_pool() noexcept = default;
	frame_pool(const frame_pool&) = delete;
	frame_pool& operator=(const frame_pool&) = delete;

	//! Frame bytes available in one block
	static constexpr std::size_t capacity() noexcept { return BlockSize - detail::c_frame_header_size; }

	[[nodiscard]] std::size_t used() const noexcept
	{
		detail::critical_guard g;
		return static_cast<std::size_t>(std::popcount(m_map));
	}

	[[nodiscard]] void* acquire(std::size_t size) noexcept
	{
		if (size > capacity()) {
			return nullptr;
		}
		detail::critical_guard g;
		for (std::size_t n = 0; n < Count; ++n) {
			if ((m_map & (1U << n)) == 0U) {
				m_map |= 1U << n;
				return detail::frame_begin(m_mem + n * BlockSize, { &frame_pool::release, this });
			}
		}
		return nullptr;
	}

private:
	static void release(void* ctx, void* block, std::size_t) noexcept
	{
		auto* const self = static_cast<frame_pool*>(ctx);
		const auto idx = static_cast<std::size_t>(static_cast<std::byte*>(block) - self->m_mem) / BlockSize;
		detail::critical_guard g;
		self->m_map &= ~(1U << idx);
	}

	alignas(8) std::byte m_mem[BlockSize * Count] {};
	std::uint32_t m_map {};
};

} // namespace isix::co

#endif
