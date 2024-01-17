#include <pch.h>

namespace	framework::net
{
	tcp_session::tcp_session(boost::asio::io_context& io_)
		: base_session(io_),
		_socket(io_)
	{
	}

	tcp_session::~tcp_session()
	{
	}

	bool	tcp_session::on_connect(
		const std::string_view& ip_,
		uint16_t port_)
	{
		if (true == ip_.empty() || 0 == port_)
		{
			_error_log_(boost::format("%1%:%2% ( %3% )") % ip_ % port_ % __FILE_LINE__);
			return false;
		}

		_end_point = boost::asio::ip::tcp::endpoint(boost::asio::ip::address::from_string(ip_.data()), port_);

		connecting();

		tcp_socket().async_connect(
			_end_point,
			boost::bind(
				&tcp_session::connect,
				shared_from_this(),
				boost::asio::placeholders::error));

		return	true;
	}

	void	tcp_session::on_close()
	{
		if (false == is_connected())	return;

		closing();

		set_closed();

		dispatch(
			[this, self = shared_from_this()]() -> void
		{
			boost::system::error_code	ec;

			if (false == tcp_socket().is_open())
			{
				tcp_socket().close(ec);
				if (boost::system::errc::success != ec)
				{
					_warning_log_(
						boost::format("%1% %2% ( %3% )")
						% ec.value()
						% ec.message()
						% __FILE_LINE__);
				}

				disconnected();
				return;
			}

			tcp_socket().cancel(ec);
			if (boost::system::errc::success != ec)
			{
				_warning_log_(
					boost::format("%1% %2% ( %3% )")
					% ec.value()
					% ec.message()
					% __FILE_LINE__);
			}

			tcp_socket().shutdown(boost::asio::socket_base::shutdown_both, ec);
			switch (ec.value())
			{
			case	boost::system::errc::success:
				break;

			case	boost::asio::error::connection_reset:
			case	boost::asio::error::connection_aborted:
			case	boost::asio::error::operation_aborted:
				_trace_log_(
					boost::format("%1% %2% ( %3% )")
					% ec.value()
					% ec.message()
					% __FILE_LINE__);
				break;

			default:
				_warning_log_(
					boost::format("%1% %2% ( %3% )")
					% ec.value()
					% ec.message()
					% __FILE_LINE__);
			}

			tcp_socket().close(ec);
			if (boost::system::errc::success != ec)
			{
				_warning_log_(
					boost::format("%1% %2% ( %3% )")
					% ec.value()
					% ec.message()
					% __FILE_LINE__);
			}

			disconnected();
		});
	}

	bool	tcp_session::on_send(const outbound_ptr_type& out_)
	{
		if (nullptr == out_)
		{
			_fatal_log_(boost::format("( %1% )") % __FILE_LINE__);
			return	false;
		}

		if (false == is_connected())	return	false;

		dispatch(
			[this,
			self = shared_from_this(),
			out_]() -> void
		{
			_outbound_q.emplace_back(out_);
			if (1 < _outbound_q.size())	return;

			if (false == is_connected())	return;

			boost::asio::async_write(
				tcp_socket(),
				boost::asio::buffer(
					_outbound_q.front()->get_buffer(),
					_outbound_q.front()->get_buf_use_size()),
				boost::asio::bind_executor(
					_strand,
					boost::bind(
						&tcp_session::write,
						shared_from_this(),
						boost::asio::placeholders::error,
						boost::asio::placeholders::bytes_transferred)));
		});

		return	true;
	}

	void	tcp_session::connect(const boost::system::error_code& ec_)
	{
		if (boost::system::errc::success != ec_)
		{
			_error_log_(
				boost::format("%1%:%2% %3% %4% ( %5% )")
				% get_endpoint_ip()
				% get_endpoint_port()
				% ec_.value()
				% ec_.message()
				% __FILE_LINE__);

			closing();
			set_closed();

			boost::system::error_code ec;

			tcp_socket().close(ec);
			if (boost::system::errc::success != ec)
			{
				_warning_log_(
					boost::format("%1% %2% ( %3% )")
					% ec.value()
					% ec.message()
					% __FILE_LINE__);
			}

			_route.clear();

			connect_failed();
			return;
		}

		//	options
		{
			tcp_socket().non_blocking(true);

			using namespace	boost::asio;

			tcp_socket().set_option(socket_base::send_buffer_size(_packet_size_));
			tcp_socket().set_option(socket_base::receive_buffer_size(_packet_size_));
			tcp_socket().set_option(ip::tcp::no_delay(true));
			tcp_socket().set_option(socket_base::keep_alive(true));

#ifdef _DEBUG
			tcp_socket().set_option(socket_base::debug(true));
#endif
		}

		auto in(alloc_inbound());
		if (!in)
		{
			_fatal_log_(
				boost::format("%1%:%2% ( %3% )")
				% get_endpoint_ip()
				% get_endpoint_port()
				% __FILE_LINE__);

			on_close();
			return;
		}

		_inbound = boost::move(*in);

		boost::asio::async_read(
			tcp_socket(),
			boost::asio::buffer(
				_inbound->header_buffer(),
				_header_size_),
			boost::asio::bind_executor(
				_strand,
				boost::bind(
					&tcp_session::read_header,
					shared_from_this(),
					boost::asio::placeholders::error,
					boost::asio::placeholders::bytes_transferred)));

		set_connected();
		connected();
	}

	void	tcp_session::read_header(
		const boost::system::error_code& ec_,
		size_t bytes_transferred_)
	{
		if (nullptr == _inbound)
		{
			_fatal_log_(
				boost::format("%1%:%2% - %3% %4% %5% (%6%)")
				% get_endpoint_ip()
				% get_endpoint_port()
				% ec_.value()
				% ec_.message()
				% bytes_transferred_
				% __FILE_LINE__);

			on_close();
			return;
		}

		if (boost::system::errc::success != ec_)
		{
			on_close();

			switch (ec_.value())
			{
			case	boost::asio::error::eof:
			case	boost::asio::error::connection_reset:
			case	boost::asio::error::connection_aborted:
			case	boost::asio::error::operation_aborted:
				_trace_log_(
					boost::format("%1%:%2% %3% %4% %5% (%6%)")
					% get_endpoint_ip()
					% get_endpoint_port()
					% ec_.value()
					% ec_.message()
					% bytes_transferred_
					% __FILE_LINE__);
				break;

			default:
				_error_log_(
					boost::format("%1%:%2% %3% %4% %5% (%6%)")
					% get_endpoint_ip()
					% get_endpoint_port()
					% ec_.value()
					% ec_.message()
					% bytes_transferred_
					% __FILE_LINE__);
			}

			return;
		}

		if (0 == bytes_transferred_)
		{
			if (false == is_connected())	return;

			_inbound->init();

			boost::asio::async_read(
				tcp_socket(),
				boost::asio::buffer(
					_inbound->header_buffer(),
					_header_size_),
				boost::asio::bind_executor(
					_strand,
					boost::bind(
						&tcp_session::read_header,
						shared_from_this(),
						boost::asio::placeholders::error,
						boost::asio::placeholders::bytes_transferred)));
			return;
		}

		if (_header_size_ != bytes_transferred_)
		{
			_error_log_(
				boost::format("%1% ( %2% )")
				% bytes_transferred_
				% __FILE_LINE__);

			on_close();
			return;
		}

		_inbound->decide();

		if (0 < _inbound->body_size())
		{
			if (false == is_connected())	return;

			boost::asio::async_read(
				tcp_socket(),
				boost::asio::buffer(
					_inbound->raw_buffer(),
					_inbound->body_size()),
				boost::asio::bind_executor(
					_strand,
					boost::bind(
						&tcp_session::read_body,
						shared_from_this(),
						boost::asio::placeholders::error,
						boost::asio::placeholders::bytes_transferred)));
			return;
		}

		impl::util::scope_exit_call		exit(
			[&, in(boost::move(_inbound))]() -> void
		{
			route(in);
		});

		auto in(alloc_inbound());
		if (!in)
		{
			_fatal_log_(
				boost::format("%1%:%2% ( %3% )")
				% get_endpoint_ip()
				% get_endpoint_port()
				% __FILE_LINE__);

			on_close();
			return;
		}

		_inbound = boost::move(*in);

		if (false == is_connected())	return;

		boost::asio::async_read(
			tcp_socket(),
			boost::asio::buffer(
				_inbound->header_buffer(),
				_header_size_),
			boost::asio::bind_executor(
				_strand,
				boost::bind(
					&tcp_session::read_header,
					shared_from_this(),
					boost::asio::placeholders::error,
					boost::asio::placeholders::bytes_transferred)));
	}

	void	tcp_session::read_body(
		const boost::system::error_code& ec_,
		size_t bytes_transferred_)
	{
		if (nullptr == _inbound)
		{
			_fatal_log_(
				boost::format("%1%:%2% %3% %4% %5% (%6%)")
				% get_endpoint_ip()
				% get_endpoint_port()
				% ec_.value()
				% ec_.message()
				% bytes_transferred_
				% __FILE_LINE__);

			on_close();
			return;
		}

		if (boost::system::errc::success != ec_)
		{
			on_close();

			switch (ec_.value())
			{
			case	boost::asio::error::eof:
			case	boost::asio::error::connection_reset:
			case	boost::asio::error::connection_aborted:
			case	boost::asio::error::operation_aborted:
				break;

			default:
				_error_log_(
					boost::format("%1% %2% %3% %4% %5% ( %6% )")
					% get_endpoint_ip()
					% get_endpoint_port()
					% ec_.value()
					% ec_.message()
					% bytes_transferred_
					% __FILE_LINE__);
			}

			return;
		}

		if (bytes_transferred_ != _inbound->body_size())
		{
			_error_log_(
				boost::format("%1% %2% ( %3% )")
				% bytes_transferred_
				% _inbound->body_size()
				% __FILE_LINE__);

			on_close();
			return;
		}

		impl::util::scope_exit_call		exit(
			[&, in(boost::move(_inbound))]() -> void
		{
#ifdef _DEBUG
			const auto* p(in->header_ptr());
			int i = p->id;
			int ii = i;
#endif

			if (2 == in->header_ptr()->crc)
				in->body_decode();


			route(in);
		});

		auto in(alloc_inbound());
		if (!in)
		{
			_fatal_log_(
				boost::format("%1%:%2% ( %3% )")
				% get_endpoint_ip()
				% get_endpoint_port()
				% __FILE_LINE__);

			on_close();
			return;
		}

		_inbound = boost::move(*in);

		if (false == is_connected())	return;

		boost::asio::async_read(
			tcp_socket(),
			boost::asio::buffer(
				_inbound->header_buffer(),
				_header_size_),
			boost::asio::bind_executor(
				_strand,
				boost::bind(
					&tcp_session::read_header,
					shared_from_this(),
					boost::asio::placeholders::error,
					boost::asio::placeholders::bytes_transferred)));
	}

	void	tcp_session::route(const inbound_ptr_type& in_)
	{
		if (nullptr == in_)
		{
			_fatal_log_(
				boost::format("%1%:%2% ( %3% )")
				% get_endpoint_ip()
				% get_endpoint_port()
				% __FILE_LINE__);

			on_close();
			return;
		}

		_inbound_q.emplace_back(in_);
		if (1 < _inbound_q.size())	return;

		dispatch(
			boost::bind(
				&tcp_session::route_poll,
				shared_from_this()));
	}

	void	tcp_session::route_poll()
	{
		if (true == _inbound_q.empty())
		{
			_fatal_log_(
				boost::format("%1% %2% ( %3% )")
				% get_endpoint_ip()
				% get_endpoint_port()
				% __FILE_LINE__);

			return;
		}

		inbound_ptr_type	in(_inbound_q.front());
		_inbound_q.pop_front();

		packet_route(in);

		dealloc_inbound(in);

		if (true == _inbound_q.empty())	return;

		dispatch(
			boost::bind(
				&tcp_session::route_poll,
				shared_from_this()));
	}

	void	tcp_session::write(
		const boost::system::error_code& ec_,
		const size_t& bytes_transferred_)
	{
		if (boost::system::errc::success != ec_)
			on_close();

		if (true == _outbound_q.empty())
		{
			_fatal_log_(
				boost::format("%1%:%2% %3% %4% %5% (%6%)")
				% get_endpoint_ip()
				% get_endpoint_port()
				% ec_.value()
				% ec_.message()
				% bytes_transferred_
				% __FILE_LINE__);

			on_close();
			return;
		}

		outbound_ptr_type	pk(_outbound_q.front());
		_outbound_q.pop_front();
		impl::util::scope_exit_call	exit([&]() -> void { dealloc_outbound(pk); });

		if (boost::system::errc::success == ec_)
		{
			if (true == _outbound_q.empty())	return;
			if (false == is_connected())		return;

			boost::asio::async_write(
				tcp_socket(),
				boost::asio::buffer(
					_outbound_q.front()->get_buffer(),
					_outbound_q.front()->get_buf_use_size()),
				boost::asio::bind_executor(
					_strand,
					boost::bind(
						&tcp_session::write,
						shared_from_this(),
						boost::asio::placeholders::error,
						boost::asio::placeholders::bytes_transferred)));
			return;
		}

		switch (ec_.value())
		{
		case	boost::asio::error::eof:
		case	boost::asio::error::connection_reset:
		case	boost::asio::error::connection_aborted:
		case	boost::asio::error::operation_aborted:
			break;

		default:
			_error_log_(
				boost::format("%1% %2% %3% %4% %5% ( %6% )")
				% get_endpoint_ip()
				% get_endpoint_port()
				% ec_.value()
				% ec_.message()
				% bytes_transferred_
				% __FILE_LINE__);
		}
	}

	tcp_session::socket_type& tcp_session::tcp_socket()
	{
		return	_socket;
	}
}