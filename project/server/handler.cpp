#include <pch.h>

handler::handler(impl::util::io_context& io_)
	: impl::network::server::handler< session >(io_.get_io_context()),
	_task_io(io_),
	_60sec_task(boost::make_shared< impl::util::timer::repeat_task >(io_.get_io_context())),
	_1sec_task(boost::make_shared< impl::util::timer::repeat_task >(io_.get_io_context())),
	_max_connect_size(_default_connect_size_),
	_is_shutdown(false)
{
}

handler::~handler()
{
	on_stop();
}

void	handler::on_start()
{
	_1sec_task->start(
		_1_sec_,
		[&]() -> bool
		{
			_info_log_(
				boost::format("%1% ( %2% )")
				% impl::util::memory_pool::count_test::count
				% __FILE_LINE__);

			return	true;
		});

	_60sec_task->start(
		_60_sec_,
		[&]() -> bool
		{
			_info_log_(
				boost::format("user total count : %1% ( %2% )")
				% afx_trader_manager()->count()
				% __FILE_LINE__);

			return	true;
		});

	impl::network::server::handler< session >::on_start();
}

void	handler::on_stop()
{
	_60sec_task->stop();

	impl::network::server::handler< session >::on_stop();
}

bool	handler::on_user_enter(const session_ptr_type& session_)
{
	size_t	count(afx_trader_manager()->count());
	if (_default_connect_size_ <= count)
	{
		_info_log_(
			boost::format("%1% ( %2% )")
			% count
			% __FILE_LINE__);
		return	false;
	}

	if (false == impl::network::server::handler< session >::on_user_enter(session_))
	{
		_error_log_(
			boost::format("%1% %2%( %3% )")
			% session_->on_get_remote_ip()
			% session_->on_get_remote_port()
			% __FILE_LINE__);

		return	false;
	}

	return	on_complete_connect(session_);
}

bool	handler::on_user_leave(const session_ptr_type& session_)
{
	if (0 != session_->compid())
	{
		afx_trader_manager()->session_leave(session_->compid());

		post(
			[compid(session_->compid())]() -> void
		{
			auto v(afx_trader_manager()->users());
			if (!v)	return;

			SC_USER_LOGOUT	pk{ .compid = compid };
			boost::range::for_each(
				*v,
				[&](const session_ptr_type& s_) -> void
				{
					if (false == s_->is_joined)	return;

					s_->fast_send(_SC_USER_LOGOUT, &pk, sizeof(SC_USER_LOGOUT));
				});
		});
	}

	return	impl::network::server::handler< session >::on_user_leave(session_);
}

bool	handler::on_route(
	const session_ptr_type& session_,
	const inbound_ptr_type& in_)
{
	const auto	h(in_->header_ptr());
	_trace_log_(
		boost::format("%1% %2% ( %3% )")
		% h->id
		% h->size
		% __FILE_LINE__);

	switch (h->id)
	{
	case	_CS_HEART_BEAT:				return	dispatch_CS_HEART_BEAT(session_, *reinterpret_cast<const CS_HEART_BEAT*>(in_->body_ptr()));
	case	_CS_TOTAL_USERCOUNT:		return	dispatch_CS_TOTAL_USERCOUNT(session_, *reinterpret_cast<const CS_TOTAL_USERCOUNT*>(in_->body_ptr()));
	case	_CS_SERVER_LOGIN:			return	dispatch_CS_SERVER_LOGIN(session_, *reinterpret_cast<const CS_SERVER_LOGIN*>(in_->body_ptr()));
	case	_CS_SERVER_ACCESS:			return	dispatch_CS_SERVER_ACCESS(session_, *reinterpret_cast<const CS_SERVER_ACCESS*>(in_->body_ptr()));
	case	_CS_USER_LOOKUP:			return	dispatch_CS_USER_LOOKUP(session_, *reinterpret_cast<const CS_USER_LOOKUP*>(in_->body_ptr()));
	case	_CS_MARKET_DATA:			return	dispatch_CS_MARKET_DATA(session_, *reinterpret_cast<const CS_MARKET_DATA*>(in_->body_ptr()));
	default:
		_error_log_(
			boost::format("%1% ( %2% )")
			% h->id
			% __FILE_LINE__);
	}

	return	false;
}

bool	handler::on_complete_connect(
	const session_ptr_type& session_)
{
	session_->on_complete_connect();
	return	true;
}

bool	handler::dispatch_CS_HEART_BEAT(
	const session_ptr_type& session_,
	const CS_HEART_BEAT& pk_)
{
	return	true;
}

bool	handler::dispatch_CS_TOTAL_USERCOUNT(
	const session_ptr_type& session_,
	const CS_TOTAL_USERCOUNT& pk_)
{
	if (0 == pk_.compid)
	{
		_error_log_(
			boost::format("%1% ( %2% )")
			% pk_.compid
			% __FILE_LINE__);
		return	false;
	}

	SC_TOTAL_USERCOUNT	pk
	{
		.compid = pk_.compid,
		.total_count = static_cast<uint16_t>(afx_trader_manager()->count())
	};
	session_->send(_SC_TOTAL_USERCOUNT, &pk, sizeof(SC_TOTAL_USERCOUNT));
	return	true;
}

bool	handler::dispatch_CS_SERVER_LOGIN(
	const session_ptr_type& session_,
	const CS_SERVER_LOGIN& pk_)
{
	if (0 == pk_.compid)
	{
		_error_log_(
			boost::format("( %1% )")
			% __FILE_LINE__);
		return	false;
	}

	auto res(afx_trader_manager()->lookup(pk_.compid));
	if (res)
	{
		auto user(*res);
		afx_trader_manager()->session_leave(user->compid());
		user->on_close();

		_warning_log_(
			boost::format("%1% ( %2% )")
			% user->compid()
			% __FILE_LINE__);
	}

	SC_SERVER_LOGIN p{ .bPass = 1 };
	session_->send(_SC_SERVER_LOGIN, &p, sizeof(SC_SERVER_LOGIN));

	return	true;
}

bool	handler::dispatch_CS_SERVER_ACCESS(
	const session_ptr_type& session_,
	const CS_SERVER_ACCESS& pk_)
{
	bool	is_success = false;
	int		step = 0;
	impl::util::scope_exit_call	exit(
		[&]() -> void
		{
			if (true == is_success)	return;

			_error_log_(
				boost::format("%1% %2% ( %3% )")
				% pk_.compid
				% step
				% __FILE_LINE__);

			SC_SERVER_ACCESS	pk;
			pk.result = 0;	//	error
			pk.compid = pk_.compid;
			session_->send(_SC_SERVER_ACCESS, &pk, sizeof(SC_SERVER_ACCESS));
		});


	auto res(afx_trader_manager()->lookup(pk_.compid));
	if (res)
	{
		step = 2;
		return	false;
	}

	session_->set(pk_.compid);
	if (false == afx_trader_manager()->session_enter(session_))
	{
		step = 3;
		return	false;
	}

	is_success = true;

	SC_SERVER_ACCESS	pk;
	pk.result = 1;
	pk.compid = pk_.compid;
	session_->send(_SC_SERVER_ACCESS, &pk, sizeof(SC_SERVER_ACCESS));

	_info_log_(
		boost::format("%1% ( %2% )")
		% pk_.compid
		% __FILE_LINE__);

	return	true;
}


bool	handler::dispatch_CS_USER_LOOKUP(
	const session_ptr_type& session_,
	const CS_USER_LOOKUP& pk_)
{
	if (0 == pk_.compid)
	{
		_error_log_(
			boost::format("%1% ( %2% )")
			% pk_.compid
			% __FILE_LINE__);

		SC_USER_LOOKUP	pk
		{
			.compid = pk_.compid,
			.is_not_exist = 1,
		};
		session_->send(_SC_USER_LOOKUP, &pk, sizeof(SC_USER_LOOKUP));
		return	true;
	}

	session_ptr_type	lookup_session;
	{
		auto res(afx_trader_manager()->lookup(pk_.compid));
		if (!res)
		{
			SC_USER_LOOKUP	pk
			{
				.compid = pk_.compid,
				.is_not_exist = 1,
			};
			session_->send(_SC_USER_LOOKUP, &pk, sizeof(SC_USER_LOOKUP));
			return	true;
		}

		lookup_session = *res;
	}

	SC_USER_LOOKUP	pk
	{
		.compid = pk_.compid
	};

	auto res(lookup_session->get());

	session_->send(_SC_USER_LOOKUP, &pk, sizeof(SC_USER_LOOKUP));
	return	true;
}


bool	handler::dispatch_CS_MARKET_DATA(
	const session_ptr_type& session_,
	const CS_MARKET_DATA& pk_)
{
	const int minval = 100;
	const int maxval = 1000000;

	if (session_->compid() != pk_.compid)
	{
		_error_log_(
			boost::format("%1% %2% ( %3% )")
			% session_->compid()
			% pk_.compid
			% __FILE_LINE__);
		return	true;
	}

	SC_MARKET_DATA pk
	{
		.compid = session_->compid(),
		.Error = 0,
	};

	for (size_t i = 0; i < std::size(pk.Sym); ++i)
	{
		pk.Sym[i].Symbol = "FOREXSYM" + std::to_string(i); 
		pk.Sym[i].Bid = pk.Sym[i].getRandomValue(minval, maxval);
		pk.Sym[i].Ask = pk.Sym[i].getRandomValue(minval-10, maxval-10);
		pk.Sym[i].DailyChange = pk.Sym[i].getRandomValue(0, 1);
	}

	auto v(afx_trader_manager()->in_range_sessions(session_));
	if (!v) return true;

	boost::range::for_each(
		*v,
		[&](const session_ptr_type& s_) -> void
		{
			s_->fast_send(_SC_MARKET_DATA, &pk, sizeof(SC_MARKET_DATA));
		});

	return true;
}
