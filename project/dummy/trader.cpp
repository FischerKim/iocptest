#include <pch.h>
#include <random>

std::mutex csvMutex;

trader::trader(
	boost::asio::io_context& io_, uint64_t compid_)
	: tcp_client< trader >(io_),
	_30sec_task(boost::make_shared< util::timer::repeat_task >(io_)),
	_200ms_task(boost::make_shared< util::timer::repeat_task >(io_)),
	_1sec_task(boost::make_shared< util::timer::repeat_task >(io_)),
	_compid(static_cast<int64_t>(compid_)) 
{
}

trader::~trader()
{
	_30sec_task->stop();
	_200ms_task->stop();
	_1sec_task->stop();
}

void trader::login()
{
	if (!loggedin) {
		auto p(CS_SERVER_LOGIN{ .compid = static_cast<uint64_t>(_compid) });
		send(_CS_SERVER_LOGIN, &p, sizeof(CS_SERVER_LOGIN));
	}
}

void trader::access_server()
{
	CS_SERVER_ACCESS p{ .compid = _compid };
	send(_CS_SERVER_ACCESS, &p, sizeof(CS_SERVER_ACCESS));
}

bool	trader::simulate()
{
	//_unique_lock_(_gen);

	//if (0 != _gen.rand(0, 1))	return	false;
	//데이터를 받아오기만해서 여기서 하는 기능은 없음

	return	true;
}

void	trader::on_complete_connect()
{
	login();

	_30sec_task->start(
		_30sec_,
		[&]() -> bool
		{
			if (initialized)
			{
				CS_HEART_BEAT p{ .compid = static_cast<uint64_t>(_compid) };
				send(_CS_HEART_BEAT, &p, sizeof(CS_HEART_BEAT));
			}
			return	true;
		});

	_200ms_task->start(
		_200ms_,
		[&]() -> bool
		{
			if (initialized)
			{
				//if (false == simulate())	return	true;

				CS_MARKET_DATA  p{ .compid = static_cast<uint64_t>(_compid), };
				send(_CS_MARKET_DATA, &p, sizeof(CS_MARKET_DATA));
			}
			return	true;
		});

	_1sec_task->start(
		_1sec_,
		[&]() -> bool
		{
			return	true;
		});
}

bool	trader::on_route(const inbound_ptr_type& in_)
{
	switch (in_->header_ptr()->id)
	{
	case	_SC_HEART_BEAT:
	{
		const SC_HEART_BEAT* p = reinterpret_cast<const SC_HEART_BEAT*>(in_->body_ptr());
		_info_log_(
			boost::format("%1% %2% ( %3% )")
			% p->compid
			% p->result
			% __FILE_LINE__);
	}
	return	true;

	case	_SC_TOTAL_USERCOUNT:
		return	true;

	case	_SC_SERVER_LOGIN:
	{
		const SC_SERVER_LOGIN* p = reinterpret_cast<const SC_SERVER_LOGIN*>(in_->body_ptr());

		_info_log_(
			boost::format("compid: %1% login success: %2% (%3%)")
			% _compid
			% (p->bPass == 1 ? "true" : "false")
			% __FILE_LINE__);

		loggedin = true;
	}
	return	true;

	case	_SC_SERVER_ACCESS:
	{
		const SC_SERVER_ACCESS* p = reinterpret_cast<const SC_SERVER_ACCESS*>(in_->body_ptr());
		if (p->result == 0) return true;

		_info_log_(
			boost::format("login successful %1% (%2%)")
			% _compid
			% __FILE_LINE__);

		initialized = true;
	}
	return true;

	case	_SC_USER_LOGOUT:
	{
		const SC_USER_LOGOUT* p = reinterpret_cast<const SC_USER_LOGOUT*>(in_->body_ptr());
		_debug_log_(
			boost::format("%1% ( %2% )")
			% p->compid
			% __FILE_LINE__);
	}
	return	true;

	case	_SC_USER_LOOKUP:
	{
		const SC_USER_LOOKUP* p = reinterpret_cast<const SC_USER_LOOKUP*>(in_->body_ptr());
		_debug_log_(
			boost::format("%1% %2% ( %3% )")
			% p->compid
			% static_cast<size_t>(p->is_not_exist)
			% __FILE_LINE__);
	}
	return	true;

	case	_SC_MARKET_DATA:
	{
		const SC_MARKET_DATA* p = reinterpret_cast<const SC_MARKET_DATA*>(in_->body_ptr());
		
		_debug_log_(
			boost::format("received: compid %1% 0: %2% 1: %3% 2: %4% (%5%)")
			% _compid
			% p->Sym[0].Bid
			% p->Sym[0].Ask
			% p->Sym[0].DailyChange
			% __FILE_LINE__);
	}
	return true;

	default:
	{
		_error_log_(
			boost::format("%1% ( %2% )")
			% in_->header_ptr()->id
			% __FILE_LINE__);
	}
	break;
	}

	return	__super::on_route(in_);
}

