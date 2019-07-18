#pragma once

#include <cstddef>
#include <forward_list>

namespace periph::blk {

	//! Generic transfer class
	class transfer {
	public:
		transfer( transfer& ) = delete;
		transfer& operator=(transfer&) = delete;
		transfer( transfer&& ) = delete;
		enum type_ : char { tx, rx, trx };
		auto type() const {
			return m_type;
		}
		virtual ~transfer() = default;
	protected:
		constexpr transfer(type_ type)
			: m_type(type)  {}
	private:
		const type_ m_type;
	};

	//! Transfer base class
	template <typename T>
	class transfer_base : public transfer {
	public:
		virtual ~transfer_base() = default;
		auto buf() const {
			return m_buf;
		}
		auto size() const {
			return m_siz;
		}
	protected:
		transfer_base(T buf, std::size_t siz, transfer::type_ type)
			: transfer(type), m_buf(buf), m_siz(siz) {}
	private:
		T const m_buf;
		const std::size_t m_siz;
	};

	//! TRX transfer
	class trx_transfer_base : public transfer {
	public:
		virtual ~trx_transfer_base() = default;
		auto tx_buf() const {
			return m_tx;
		}
		auto size() const {
			return m_tx_siz;
		}
		auto rx_buf() const {
			return m_rx;
		}
		auto tx_size() const  {
			return m_tx_siz;
		}
		auto rx_size() const {
			return m_rx_siz;
		}
	protected:
		trx_transfer_base(const void* tx, void* rx, std::size_t tx_size, std::size_t rx_size)
			: transfer(transfer::trx), m_tx_siz(tx_size), m_rx_siz(rx_size), m_tx(tx), m_rx(rx)
		{}
		trx_transfer_base(const void* tx, void* rx, std::size_t siz)
			: trx_transfer_base(tx,rx,siz,siz)
		{}
	private:
		const std::size_t m_tx_siz;
		const std::size_t m_rx_siz;
		const void* const m_tx;
		void* const m_rx;
	};

	//! <<< Final user class for transfer >>>
	//! TX transfer
	using tx_transfer_base = transfer_base<const void*>;
	template <typename T>
	class tx_transfer : public tx_transfer_base {
	public:
		tx_transfer(const T buf, std::size_t siz)
			: transfer_base(static_cast<const void*>(buf),siz,transfer::tx)
		{}
	};

	//! RX transfer
	using rx_transfer_base = transfer_base<void*>;
	template <typename T>
	class rx_transfer : public rx_transfer_base {
	public:
		rx_transfer( T buf, std::size_t siz)
			:transfer_base(static_cast<void*>(buf),siz,transfer::rx)
		{}
	};

	//! Bidirectional transfer
	template <typename T, typename K>
		class trx_transfer : public trx_transfer_base
	{
	public:
		trx_transfer(const T tx, K rx, std::size_t siz)
			: trx_transfer_base(static_cast<const void*>(tx),
		      static_cast<void*>(rx),siz) {}
		trx_transfer(const T tx, K rx, std::size_t tx_size, std::size_t rx_size)
		: trx_transfer_base(static_cast<const void*>(tx),
		     static_cast<void*>(rx),tx_size,rx_size) {}
	};

	//! Transfer chain
	using transfer_chain = std::forward_list<transfer>;
}

