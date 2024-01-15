#include <pch.h>

namespace	framework::net
{
	base_handler::base_handler(
		boost::asio::io_context& io_)
		: _port(0),
		_io(io_),
		_is_connected(false),
		_is_closing(true)
	{
		for (size_t i = 0; i < _default_pool_size_; ++i)
		{
			try
			{
				outbound_ptr_type	out(boost::make_shared< outbound >());
				if (nullptr == out)
				{
					_fatal_log_(boost::format("( %1% )") % __FILE_LINE__);
					continue;
				}

				out->init();
				_outbound_pool.emplace_back(out);
			}
			catch (const std::exception& e_)
			{
				_fatal_log_(boost::format("%1% ( %2% )") % e_.what() % __FILE_LINE__);
			}
		}
	}

	base_handler::~base_handler()
	{
	}

	void	base_handler::on_close()
	{
		set_closing(true);
		set_connected(false);
	}

	bool	base_handler::send(
		uint16_t packet_id_,
		const void* body_,
		size_t size_)
	{
		if (true == is_closing() || false == is_connected())		return false;

		auto	out(alloc_outbound());
		if (!out)
		{
			_error_log_(
				boost::format("%1% %2% ( %3% )")
				% packet_id_
				% size_
				% __FILE_LINE__);

			return	false;
		}

		out->get()->encode(packet_id_, body_, size_);

		return	on_send(*out);
	}

	void	base_handler::on_connected(
		const std::string_view& ip_,
		uint16_t port_)
	{
		set_closing(false);
		set_connected(true);
	}

	void	base_handler::on_closing(
		const std::string_view& ip_,
		uint16_t port_)
	{
		set_closing(true);
	}

	void	base_handler::on_disconnected(
		const std::string_view& ip_,
		uint16_t port_)
	{
		set_closing(true);
		set_connected(false);
	}

	void	base_handler::post(const void_func_type& f_)
	{
		if (true == f_.empty())	return;

		boost::asio::post(get_io(), f_);
	}

	void	base_handler::dispatch(const void_func_type& f_)
	{
		if (true == f_.empty())	return;

		boost::asio::dispatch(get_io(), f_);
	}

	boost::optional< outbound_ptr_type >
		base_handler::alloc_outbound()
	{
		outbound_ptr_type	p;

		{
#ifdef	TRACE
			_spin_lock_fl_(_outbound_pool, __FILE_LINE__);
#else
			_spin_lock_(_outbound_pool);
#endif
			if (false == _outbound_pool.empty())
			{
				p = _outbound_pool.front();
				_outbound_pool.pop_front();
				return	{ p };
			}
		}

		try
		{
			p = boost::make_shared< outbound >();
			if (nullptr == p)
			{
				_fatal_log_(boost::format("( %1% )") % __FILE_LINE__);
				return	{};
			}

			p->init();
			return { p };
		}
		catch (const std::exception& e_)
		{
			_fatal_log_(boost::format("%1% ( %2% )") % e_.what() % __FILE_LINE__);
		}

		return	{};
	}

	void	base_handler::dealloc_outbound(const outbound_ptr_type& p_)
	{
		if (nullptr == p_)
		{
			_fatal_log_(boost::format("( %1% )") % __FILE_LINE__);
			return;
		}

		p_->init();

#ifdef	TRACE
		_spin_lock_fl_(_outbound_pool, __FILE_LINE__);
#else
		_spin_lock_(_outbound_pool);
#endif

		if (_max_pool_size_ <= _outbound_pool.size())		return;

		_outbound_pool.emplace_back(p_);
	}
}