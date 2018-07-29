#pragma once

#include <cstddef>
#include <forward_list>

namespace periph::blk {

	//! Generic transfer class
	class transfer {
	public:
		enum type_ : char { tx, rx, trx };
	protected:
		constexpr transfer(type_ type)
			: m_type(type)  {}
		auto type() const {
			return m_type;
		}
	private:
		const type_ m_type;
	};

	//! Transfer base class
	template <typename T>
	class transfer_base : public virtual transfer {
	public:
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
	class trx_transfer_base : public transfer_base<const void*>, public transfer_base<void*> {
	public:
		auto tx_buf() const {
			transfer_base<const void*>::buf();
		}
		auto tx_size() const {
			transfer_base<const void*>::size();
		}
		auto rx_buf() const {
			transfer_base<void*>::buf();
		}
		auto rx_size() const {
			transfer_base<void*>::size();
		}
	protected:
		trx_transfer_base(const void* tx, std::size_t txsiz, void* rx, std::size_t rxsiz)
			: transfer(transfer::trx),
			  transfer_base<const void*>(tx,txsiz,transfer::trx),
			  transfer_base<void*>(rx,rxsiz,transfer::trx)
		{}
	};

	//! <<< Final user class for transfer >>>

	//! TX transfer
	template <typename T>
	class tx_transfer : public transfer_base<const void*> {
	public:
		tx_transfer(const T buf, std::size_t siz)
			: transfer_base(static_cast<const void*>(buf),siz,transfer::tx)
		{}
	};

	//! RX transfer
	template <typename T>
	class rx_transfer : public transfer_base<void*> {
	public:
		rx_transfer( T buf, std::size_t siz)
			:transfer_base(static_cast<void*>(buf),siz,transfer::rx)
		{}
	};

	//! Bidirectional transfer
	template <typename T>
		class trx_transfer : public trx_transfer_base
	{
	public:
		trx_transfer(const T tx, std::size_t txsiz, T rx, std::size_t rxsiz)
			: trx_transfer_base(static_cast<const void*>(tx),txsiz,static_cast<void*>(rx),rxsiz) {}
	};

	//! Ttransfer chain
	using transfer_chain = std::forward_list<transfer>;
}

