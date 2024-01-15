#pragma once

namespace	framework::net
{
	class	base_session : private boost::noncopyable
	{
	protected:
		enum
		{
			_default_pool_size_ = 4,
			_max_pool_size_ = 16,
		};

	public:
		using	route_func_type = boost::function< bool(const inbound_ptr_type&) >;
		using	event_func_type = boost::function< void(const std::string_view&, uint16_t) >;

		using	alloc_outbound_func_type = boost::function< boost::optional< outbound_ptr_type >() >;
		using	dealloc_outbound_func_type = boost::function< void(const outbound_ptr_type&) >;

	public:
		explicit	base_session(boost::asio::io_context& io_);
		virtual ~base_session();

		boost::asio::io_context&
			get_io_context() { return	_io; }

		std::string	get_endpoint_ip()		const { return	_end_point.address().to_string(); }
		uint16_t	get_endpoint_port()		const { return	_end_point.port(); }

		bool	is_all_bind()	const;
		bool	is_connected()	const { return	_is_connected; }

		void	post(const void_func_type& f_);
		void	dispatch(const void_func_type& f_);

		void	bind_route(const route_func_type& f_);

		void	bind_connecting(const event_func_type& f_);
		void	bind_connected(const event_func_type& f_);
		void	bind_connect_failed(const event_func_type& f_);
		void	bind_closing(const event_func_type& f_);
		void	bind_disconnected(const event_func_type& f_);

		void	bind_alloc_outbound(const alloc_outbound_func_type& f_);
		void	bind_dealloc_outbound(const dealloc_outbound_func_type& f_);

	public:
		virtual bool	on_connect(const std::string_view& ip_, uint16_t port_) = 0;
		virtual void	on_close() = 0;
		virtual	bool	on_send(const outbound_ptr_type& out_) = 0;

	protected:
		bool	packet_route(const inbound_ptr_type& in_);
		void	connecting();
		void	connected();
		void	connect_failed();
		void	closing();
		void	disconnected();

	public:
		boost::optional< outbound_ptr_type >
			alloc_outbound();

		void	dealloc_outbound(const outbound_ptr_type& p_);

	protected:
		boost::optional< inbound_ptr_type >
			alloc_inbound();

		void	dealloc_inbound(const inbound_ptr_type& p_);

		void	set_connected() { _is_connected = true; }
		void	set_closed() { _is_connected = false; }

	protected:
		using	tcp_endpoint = boost::asio::ip::tcp::endpoint;
		tcp_endpoint	_end_point;

		using	strand = boost::asio::strand< boost::asio::io_context::executor_type >;
		strand			_strand;

	protected:
		route_func_type	_route;

		using	outbound_q = std::deque< outbound_ptr_type >;
		outbound_q		_outbound_q;

		using	inbound_q = std::deque< inbound_ptr_type >;
		inbound_q		_inbound_q;

	private:
		event_func_type	_connecting;
		event_func_type	_connected;
		event_func_type	_connect_failed;
		event_func_type	_closing;
		event_func_type	_disconnected;

		alloc_outbound_func_type	_alloc_outbound;
		dealloc_outbound_func_type	_dealloc_outbound;

		boost::asio::io_context& _io;

		using	inbound_queue = std::deque< inbound_ptr_type >;
		inbound_queue		_inbound_pool;

		boost::atomic_bool	_is_connected;
	};
}