//
// Copyright (c) 2014-2016 Pavel Medvedev. All rights reserved.
// Copyright (c) 2018 Lucjan Bryndza. Embedded systems modifications.
// Distributed under the MIT software license, see the accompanying
// file LICENSE

#pragma once

#include <stdexcept>
#include <functional>
#include <tuple>
#include <type_traits>


namespace std {
	void __throw_bad_function_call();
}


namespace fnd {
namespace estd {


enum class function_construct_t {
	none,
	copy,
	move,
	copy_and_move,
};


namespace details {



// V-table implementation
template<typename Ret, typename ...Args>
struct fixed_function_vtable_base
{
    Ret  (*call)(void*, Args&& ...) = nullptr;
    void (*destroy)(void*) = nullptr;
};

template<function_construct_t ConstructStrategy, typename Ret, typename ...Args>
struct fixed_function_vtable;

template<typename Ret, typename ...Args>
struct fixed_function_vtable<function_construct_t::none, Ret, Args...>
        : fixed_function_vtable_base<Ret, Args...>
{
};

template<typename Ret, typename ...Args>
struct fixed_function_vtable<function_construct_t::copy, Ret, Args...>
        : fixed_function_vtable_base<Ret, Args...>
{
    void (*copy)(const void*, void*) = nullptr;
};

template<typename Ret, typename ...Args>
struct fixed_function_vtable<function_construct_t::move, Ret, Args...>
        : fixed_function_vtable_base<Ret, Args...>
{
    void (*move)(void*, void*) = nullptr;
};

template<typename Ret, typename ...Args>
struct fixed_function_vtable<function_construct_t::copy_and_move, Ret, Args...>
        : fixed_function_vtable_base<Ret, Args...>
{
    void (*copy)(const void*, void*) = nullptr;
    void (*move)(void*, void*) = nullptr;
};

} // namespace details

template<typename Function, size_t MaxSize = 16, function_construct_t ConstructStrategy = function_construct_t::copy_and_move>
class function;

template<typename Ret, typename ...Args, size_t MaxSize, function_construct_t ConstructStrategy>
class function<Ret (Args...), MaxSize, ConstructStrategy>
{
public:
// Compile-time information

	using is_copyable = std::integral_constant<bool, ConstructStrategy == function_construct_t::copy
		|| ConstructStrategy == function_construct_t::copy_and_move>;
	using is_movable = std::integral_constant<bool, ConstructStrategy == function_construct_t::move
		|| ConstructStrategy == function_construct_t::copy_and_move>;

	using result_type = Ret;

	static const std::size_t arity = sizeof...(Args);

	template <std::size_t N>
	struct argument
	{
		static_assert(N < arity, "invalid argument index");
		using type = typename std::tuple_element<N, std::tuple<Args...>>::type;
	};

public:
	template<typename F, size_t S, function_construct_t C> function(function<F, S, C> const&) = delete;
	template<typename F, size_t S, function_construct_t C> function(function<F, S, C>&) = delete;
	template<typename F, size_t S, function_construct_t C> function(function<F, S, C>&&) = delete;
	template<typename F, size_t S, function_construct_t C> function& operator=(function<F, S, C> const&) = delete;
	template<typename F, size_t S, function_construct_t C> function& operator=(function<F, S, C>&) = delete;
	template<typename F, size_t S, function_construct_t C> function& operator=(function<F, S, C>&&) = delete;
	template<typename F, size_t S, function_construct_t C> void assign(function<F, S, C> const&) = delete;
	template<typename F, size_t S, function_construct_t C> void assign(function<F, S, C>&) = delete;
	template<typename F, size_t S, function_construct_t C> void assign(function<F, S, C>&&) = delete;

	function() {}

	~function()
	{
		reset();
	}

	function(std::nullptr_t) {}

	function& operator=(std::nullptr_t)
	{
		reset();
		return *this;
	}

	function(function const& src)
	{
		copy(src);
	}

	function& operator=(function const& src)
	{
		assign(src);
		return *this;
	}

	function(function& src)
	{
		copy(src);
	}
	
	function& operator=(function& src)
	{
		assign(src);
		return *this;
	}

	function(function&& src)
	{
		move(std::move(src), is_movable());
	}

	function& operator=(function&& src)
	{
		assign(std::move(src));
		return *this;
	}

	template<typename Functor>
	function(Functor&& f)
	{
		create(std::forward<Functor>(f));
	}

	template<typename Functor>
	function& operator=(Functor&& f)
	{
		assign(std::forward<Functor>(f));
		return *this;
	}

	void assign(function const& src)
	{
		reset();
		copy(src);
	}

	void assign(function& src)
	{
		reset();
		copy(src);
	}

	void assign(function&& src)
	{
		reset();
		move(std::move(src), is_movable());
	}

	template<typename Functor>
	void assign(Functor&& f)
	{
		reset();
		create(std::forward<Functor>(f));
	}

	void reset()
	{
		auto destroy = vtable_.destroy;
		if (destroy)
		{
			vtable_ = vtable();
			destroy(&storage_);
		}
	}

	explicit operator bool() const { return vtable_.call != nullptr; }

	Ret operator()(Args... args)
	{
		return vtable_.call ?
			vtable_.call(&storage_, std::forward<Args>(args)...) :
			std::__throw_bad_function_call();
	}

	void swap(function& other)
	{
		function tmp = std::move(other);
		other = std::move(*this);
		*this = std::move(tmp);
	}

	friend void swap(function& lhs, function& rhs)
	{
		lhs.swap(rhs);
	}

	friend bool operator==(std::nullptr_t, function const& f)
	{
		return !f;
	}

	friend bool operator==(function const& f, std::nullptr_t)
	{
		return !f;
	}

	friend bool operator!=(std::nullptr_t, function const& f)
	{
		return f;
	}

	friend bool operator!=(function const& f, std::nullptr_t)
	{
		return f;
	}

private:
	template<typename Functor>
	void create(Functor&& f)
	{
		using functor_type = typename std::decay<Functor>::type;
		static_assert(sizeof(functor_type) <= StorageSize, "Functor must be smaller than storage buffer");

		new (&storage_) functor_type(std::forward<Functor>(f));

		vtable_.call = &call_impl<functor_type>;
		vtable_.destroy = &destroy_impl<functor_type>;
		init_copy<functor_type>(is_copyable());
		init_move<functor_type>(is_movable());
	}

	void copy(function const& src)
	{
		if (src.vtable_.copy)
		{
			src.vtable_.copy(&src.storage_, &storage_);
			vtable_ = src.vtable_;
		}
	}

	void move(function&& src, std::true_type movable)
	{
		if (src.vtable_.move)
		{
			src.vtable_.move(&src.storage_, &storage_);
			vtable_ = src.vtable_;
			src.reset();
		}
	}

	void move(function const& src, std::false_type movable)
	{
		copy(src);
	}

private:
	template<typename Functor>
	static Ret call_impl(void* functor, Args&& ... args)
	{
		return (*static_cast<Functor*>(functor))(std::forward<Args>(args)...);
	}

	template<typename Functor>
	static void destroy_impl(void* functor)
	{
		static_cast<Functor*>(functor)->~Functor();
	}

	template<typename Functor>
	static void copy_impl(void const* functor, void* dest)
	{
		new (dest) Functor(*static_cast<Functor const*>(functor));
	}

	template<typename Functor>
	static void move_impl(void* functor, void* dest)
	{
		new (dest) Functor(std::move(*static_cast<Functor*>(functor)));
	}

	template<typename Functor>
	void init_copy(std::true_type /*copyable*/) { vtable_.copy = &copy_impl<Functor>; }

	template<typename Functor>
	void init_copy(std::false_type /*copyable*/) {}

	template<typename Functor>
	void init_move(std::true_type /*movable*/) { vtable_.move = &move_impl<Functor>; }

	template<typename Functor>
	void init_move(std::false_type /*movable*/) {}

private:
	using vtable = details::fixed_function_vtable<ConstructStrategy, Ret, Args...>;
	static const size_t StorageSize = MaxSize - sizeof(vtable);
	using storage = typename std::aligned_storage<StorageSize>::type;

	vtable vtable_;
	storage storage_;
};

}}
