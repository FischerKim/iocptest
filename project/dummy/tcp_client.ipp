#pragma once

namespace	framework::net
{
	template < typename inheritance_type >
	tcp_client< inheritance_type >::tcp_client(boost::asio::io_context& io_)
		: tcp_handler< inheritance_type >(io_)
	{
	}

	template < typename inheritance_type >
	tcp_client< inheritance_type >::~tcp_client()
	{
	}

	template < typename inheritance_type >
	void	tcp_client< inheritance_type >::on_complete_connect()
	{
		int i = 0;
	}

	template < typename inheritance_type >
	bool	tcp_client< inheritance_type >::on_route(const inbound_ptr_type& in_)
	{
		return	true;
	}

	template < typename inheritance_type >
	void	tcp_client< inheritance_type >::on_connecting(const std::string_view& ip_, uint16_t port_)
	{
		tcp_handler< inheritance_type >::on_connecting(ip_, port_);

		_info_log_(
			boost::format("connecting! %1%:%2% ( %3% )")
			% ip_
			% port_
			% __FILE_LINE__);
	}

	template < typename inheritance_type >
	void	tcp_client< inheritance_type >::on_connected(const std::string_view& ip_, uint16_t port_)
	{
		tcp_handler< inheritance_type >::on_connected(ip_, port_);

		_info_log_(
			boost::format("connected! %1%:%2% ( %3% )")
			% ip_
			% port_
			% __FILE_LINE__);

		on_complete_connect();
	}

	template < typename inheritance_type >
	void	tcp_client< inheritance_type >::on_connect_failed(const std::string_view& ip_, uint16_t port_)
	{
		tcp_handler< inheritance_type >::on_connect_failed(ip_, port_);

		_warning_log_(
			boost::format("connect failed! %1%:%2% ( %3% )")
			% ip_
			% port_
			% __FILE_LINE__);

		if (true == this->is_connected())		return;

		impl::util::timer::timed_task::invoke(
			this->get_io(),
			_reconnect_time_,
			[&, self = this->shared_from_this()]() -> void
		{
			if (true == this->is_connected())		return;

			this->on_reconnect();
		});
	}

	template < typename inheritance_type >
	void	tcp_client< inheritance_type >::on_closing(const std::string_view& ip_, uint16_t port_)
	{
		tcp_handler< inheritance_type >::on_closing(ip_, port_);

		_info_log_(
			boost::format("closing! %1%:%2% ( %3% )")
			% ip_
			% port_
			% __FILE_LINE__);
	}

	template < typename inheritance_type >
	void	tcp_client< inheritance_type >::on_disconnected(const std::string_view& ip_, uint16_t port_)
	{
		tcp_handler< inheritance_type >::on_disconnected(ip_, port_);
	}
}