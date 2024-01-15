#pragma once

namespace	framework::net
{
	template < typename inheritance_type >
	class	tcp_client : public tcp_handler< inheritance_type >
	{
	private:
		enum
		{
			_reconnect_time_ = 100,
		};

	protected:
		tcp_client(boost::asio::io_context& io_);

	public:
		virtual	~tcp_client();

	protected:
		virtual	void	on_complete_connect() = 0;

		virtual bool	on_route(const inbound_ptr_type& in_)	override;

		virtual void	on_connecting(const std::string_view& ip_, uint16_t port_)	override;

		virtual void	on_connected(const std::string_view& ip_, uint16_t port_)	override;

		virtual void	on_connect_failed(const std::string_view& ip_, uint16_t port_)	override;

		virtual void	on_closing(const std::string_view& ip_, uint16_t port_)	override;

		virtual void	on_disconnected(const std::string_view& ip_, uint16_t port_)	override;
	};
}