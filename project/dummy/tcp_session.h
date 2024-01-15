#pragma once

namespace	framework::net
{
	class	tcp_session : public	base_session,
		public	boost::enable_shared_from_this< tcp_session >
	{
	public:
		tcp_session(boost::asio::io_context& io_);
		virtual ~tcp_session();

		virtual bool	on_connect(
			const std::string_view& ip_,
			uint16_t port_)	override;

		virtual void	on_close()	override;

		virtual	bool	on_send(const outbound_ptr_type& out_)	override;

	private:
		void	connect(const boost::system::error_code& ec_);

		void	read_header(
			const boost::system::error_code& ec_,
			size_t bytes_transferred_);

		void	read_body(
			const boost::system::error_code& ec_,
			size_t bytes_transferred_);

		void	route(const inbound_ptr_type& in_);

		void	route_poll();

		void	write(
			const boost::system::error_code& ec_,
			const size_t& bytes_transferred_);

	private:
		using	socket_type = boost::asio::ip::tcp::socket;

		socket_type& tcp_socket();

	private:
		socket_type			_socket;
		inbound_ptr_type	_inbound;
	};
}