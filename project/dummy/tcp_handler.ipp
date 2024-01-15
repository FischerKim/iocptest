#pragma once

namespace	framework::net
{
	template < typename inheritance_type >
	tcp_handler< inheritance_type >::tcp_handler(boost::asio::io_context& io_)
		: base_handler(io_),
		_session(nullptr)
	{
	}

	template < typename inheritance_type >
	tcp_handler< inheritance_type >::~tcp_handler()
	{
	}

	template < typename inheritance_type >
	bool	tcp_handler< inheritance_type >::on_connect(
		const std::string_view& ip_,
		uint16_t port_)
	{
		_ip.assign(ip_.data(), ip_.size());
		_port = port_;

		if (false == is_closing() || true == is_connected())
		{
			_fatal_log_(
				boost::format("%1%")
				% __FILE_LINE__);

			post(
				boost::bind(
					&tcp_handler::on_connect_failed,
					this->shared_from_this(),
					_ip,
					_port));

			return	false;
		}

		auto	session(boost::make_shared< tcp_session >(get_io()));
		session->bind_route(
			boost::bind(
				&tcp_handler::on_route,
				this->shared_from_this(),
				_1));

		session->bind_connecting(
			boost::bind(
				&tcp_handler::on_connecting,
				this->shared_from_this(),
				_1,
				_2));

		session->bind_connected(
			boost::bind(
				&tcp_handler::on_connected,
				this->shared_from_this(),
				_1,
				_2));

		session->bind_connect_failed(
			boost::bind(
				&tcp_handler::on_connect_failed,
				this->shared_from_this(),
				_1,
				_2));

		session->bind_closing(
			boost::bind(
				&tcp_handler::on_closing,
				this->shared_from_this(),
				_1,
				_2));

		session->bind_disconnected(
			boost::bind(
				&tcp_handler::on_disconnected,
				this->shared_from_this(),
				_1,
				_2));

		session->bind_alloc_outbound(
			boost::bind(
				&tcp_handler::alloc_outbound,
				this->shared_from_this()));

		session->bind_dealloc_outbound(
			boost::bind(
				&tcp_handler::dealloc_outbound,
				this->shared_from_this(),
				_1));

		if (false == session->on_connect(_ip, _port))
		{
			_error_log_(
				boost::format("%1% %2% ( %3% )")
				% _ip
				% _port
				% __FILE_LINE__);
			return	false;
		}

		_session.store(session);
		return	true;
	}

	template < typename inheritance_type >
	void	tcp_handler< inheritance_type >::on_reconnect()
	{
		if (false == is_closing() || true == is_connected())
		{
			_fatal_log_(
				boost::format("%1%")
				% __FILE_LINE__);

			post(
				boost::bind(
					&tcp_handler::on_connect_failed,
					this->shared_from_this(),
					_ip,
					_port));

			return;
		}

		post(
			boost::bind(
				&tcp_handler::on_connect,
				this->shared_from_this(),
				_ip,
				_port));
	}

	template < typename inheritance_type >
	void	tcp_handler< inheritance_type >::on_close()
	{
		base_handler::on_close();

		session_ptr_type	p(_session);
		_session.store(nullptr);
		if (nullptr == p)	return;

		p->on_close();
	}

	template < typename inheritance_type >
	bool	tcp_handler< inheritance_type >::on_send(const outbound_ptr_type& out_)
	{
		if (false == is_connected() || true == is_closing())	return false;

		session_ptr_type	p(_session);
		return	(nullptr == p) ?
			false :
			p->on_send(out_);
	}
}