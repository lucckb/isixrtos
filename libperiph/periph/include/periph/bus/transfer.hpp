#pragma once

#include <cstddef>

namespace periph {
namespace bus {

	//! Generic transfer class
	class transfer {
	public:
		enum type : char { tx, rx, trx };
	protected:
		constexpr transfer(type _type)
			: m_type(_type)  {}
		const type m_type;
	};

	//! Transfer base class
	template <typename T, int J>
	class transfer_base : public transfer {
	protected:
		transfer_base(void* buf, std::size_t siz)
			: transfer(J), m_buf(buf), m_siz(siz){}
		T const m_buf;
		const std::size_t m_siz;
	};
	//! TX transfer base
	using tx_transfer_base = transfer_base<const void*, transfer::tx>;
	//! RX transfer base
	using rx_transfer_base = transfer_base<void*, transfer::tx>;


	//! TRX transfer
	class trx_transfer_base : public transfer {
	protected:
		trx_transfer_base( const void* tx, std::size_t txsiz, void* rx, std::size_t rxsiz )
			: transfer(transfer::trx),m_tx(tx),m_rx(rx),m_txsiz(txsiz),m_rxsiz(rxsiz)
		{}
		const void* const m_tx;
		void* const m_rx;
		std::size_t m_txsiz;
		std::size_t m_rxsiz;
	};

	//! <<< Final user class for transfer >>>

	//! TX transfer
	template <typename T>
	class tx_transfer : public tx_transfer_base {
	public:
		tx_transfer(const T* buf, std::size_t siz)
			: tx_transfer_base(buf,siz)
		{}
	};

	//! RX transfer
	template <typename T>
	class rx_transfer : public rx_transfer_base {
	public:
		rx_transfer( T* buf, std::size_t siz)
			:rx_transfer_base(buf,siz)
		{}
	};

	//! Bidirectional transfer
	template <typename T>
		class trx_transfer : public trx_transfer_base
	{
	public:
		trx_transfer(const T* tx, std::size_t txsiz, T* rx, std::size_t rxsiz)
			: trx_transfer_base(tx,txsiz,rx,rxsiz) {}
	};
}}

