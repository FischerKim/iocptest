#pragma once

namespace	framework::net
{
	class	base_handler : private	boost::noncopyable
	{
	private:
		enum
		{
			_default_pool_size_ = 4,
			_max_pool_size_ = 16,
		};

	public:
		base_handler(
			boost::asio::io_context& io_);

		virtual	~base_handler();

		virtual bool	on_connect(
			const std::string_view& ip_,
			uint16_t port_) = 0;

		virtual void	on_reconnect() = 0;

		virtual void	on_close();

	public:
		bool	send(
			uint16_t packet_id_,
			const void* body_,
			size_t size_);

	protected:
		virtual	bool	on_send(const outbound_ptr_type& out_) = 0;

		virtual bool	on_route(const inbound_ptr_type& in_) = 0;

		virtual void	on_connecting(const std::string_view& ip_, uint16_t port_) {}
		virtual void	on_connected(const std::string_view& ip_, uint16_t port_);
		virtual void	on_connect_failed(const std::string_view& ip_, uint16_t port_) {}
		virtual void	on_closing(const std::string_view& ip_, uint16_t port_);
		virtual void	on_disconnected(const std::string_view& ip_, uint16_t port_);

	protected:
		void	post(const void_func_type& f_);
		void	dispatch(const void_func_type& f_);

		boost::optional< outbound_ptr_type >
			alloc_outbound();

		void	dealloc_outbound(const outbound_ptr_type& p_);

	public:
		const std::string& get_ip()	const { return	_ip; }

		uint16_t	get_port()	const { return	_port; }

		boost::asio::io_context& get_io()	const { return	_io; }

		bool	is_connected()	const { return	_is_connected; }
		bool	is_closing()	const { return	_is_closing; }

		void	set_connected(bool connected_) { _is_connected = connected_; }
		void	set_closing(bool closing_) { _is_closing = closing_; }

	protected:
		std::string	_ip;
		uint16_t	_port;

	private:
		boost::asio::io_context& _io;

		boost::atomic_bool	_is_connected;
		boost::atomic_bool	_is_closing;

		class	lock_outbound_deque : public std::deque< outbound_ptr_type >, public impl::util::spin_lock {};
		lock_outbound_deque	_outbound_pool;
	};
}