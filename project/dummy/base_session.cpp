#include <pch.h>

namespace	framework::net
{
	base_session::base_session(boost::asio::io_context& io_)
		: _strand(boost::asio::make_strand(io_)),
		_io(io_),
		_is_connected(false)
	{
		for (size_t i = 0; i < _default_pool_size_; ++i)
		{
			try
			{
				inbound_ptr_type	in(boost::make_shared< inbound >());
				if (nullptr == in)
				{
					_fatal_log_(boost::format("( %1% )") % __FILE_LINE__);
					continue;
				}

				in->init();
				_inbound_pool.emplace_back(in);
			}
			catch (const std::exception& e_)
			{
				_fatal_log_(boost::format("%1% ( %2% )") % e_.what() % __FILE_LINE__);
			}
		}
	}

	base_session::~base_session()
	{
	}

	void	base_session::bind_route(const route_func_type& f_)
	{
		_route = f_;
	}

	void	base_session::bind_connecting(const event_func_type& f_)
	{
		_connecting = f_;
	}

	void	base_session::bind_connected(const event_func_type& f_)
	{
		_connected = f_;
	}

	void	base_session::bind_connect_failed(const event_func_type& f_)
	{
		_connect_failed = f_;
	}

	void	base_session::bind_closing(const event_func_type& f_)
	{
		_closing = f_;
	}

	void	base_session::bind_disconnected(const event_func_type& f_)
	{
		_disconnected = f_;
	}

	void	base_session::bind_alloc_outbound(const alloc_outbound_func_type& f_)
	{
		_alloc_outbound = f_;
	}

	void	base_session::bind_dealloc_outbound(const dealloc_outbound_func_type& f_)
	{
		_dealloc_outbound = f_;
	}

	bool	base_session::is_all_bind()	const
	{
		if (true == _connecting.empty())			return false;
		if (true == _connected.empty())			return false;
		if (true == _connect_failed.empty())		return false;
		if (true == _closing.empty())				return false;
		if (true == _disconnected.empty())		return false;
		if (true == _alloc_outbound.empty())		return false;
		if (true == _dealloc_outbound.empty())	return false;
		return true;
	}

	void	base_session::post(const void_func_type& f_)
	{
		if (true == f_.empty())	return;

		boost::asio::post(boost::asio::bind_executor(_strand, f_));
	}

	void	base_session::dispatch(const void_func_type& f_)
	{
		if (true == f_.empty())	return;

		boost::asio::dispatch(boost::asio::bind_executor(_strand, f_));
	}

	bool	base_session::packet_route(const inbound_ptr_type& in_)
	{
		if (true == _route.empty() || nullptr == in_)	return false;
		return	_route(in_);
	}

	void	base_session::connecting()
	{
		if (true == _connecting.empty())	return;
		_connecting(get_endpoint_ip(), get_endpoint_port());
	}

	void	base_session::connected()
	{
		if (true == _connected.empty())	return;
		_connected(get_endpoint_ip(), get_endpoint_port());
	}

	void	base_session::connect_failed()
	{
		if (true == _connect_failed.empty())	return;
		_connect_failed(get_endpoint_ip(), get_endpoint_port());
	}

	void	base_session::closing()
	{
		if (true == _closing.empty())	return;
		_closing(get_endpoint_ip(), get_endpoint_port());
	}

	void	base_session::disconnected()
	{
		if (true == _disconnected.empty())	return;
		_disconnected(get_endpoint_ip(), get_endpoint_port());
	}

	boost::optional< outbound_ptr_type >
		base_session::alloc_outbound()
	{
		if (true == _alloc_outbound.empty())
			return	{};

		return	_alloc_outbound();
	}

	void	base_session::dealloc_outbound(const outbound_ptr_type& p_)
	{
		if (true == _dealloc_outbound.empty())
			return;

		_dealloc_outbound(p_);
	}

	boost::optional< inbound_ptr_type >
		base_session::alloc_inbound()
	{
		inbound_ptr_type	p;

		if (false == _inbound_pool.empty())
		{
			p = _inbound_pool.front();
			_inbound_pool.pop_front();
			return	{ p };
		}

		try
		{
			p = boost::make_shared< inbound >();
			if (nullptr == p)
			{
				_fatal_log_(boost::format("( %1% )") % __FILE_LINE__);
				return	{};
			}

			p->init();
			return	{ p };
		}
		catch (const std::exception& e_)
		{
			_fatal_log_(boost::format("%1% ( %2% )") % e_.what() % __FILE_LINE__);
		}

		return	{};
	}

	void	base_session::dealloc_inbound(const inbound_ptr_type& p_)
	{
		if (nullptr == p_)
		{
			_fatal_log_(boost::format("( %1% )") % __FILE_LINE__);
			return;
		}

		if (_max_pool_size_ <= _inbound_pool.size())	return;

		p_->init();
		_inbound_pool.emplace_back(p_);
	}
}