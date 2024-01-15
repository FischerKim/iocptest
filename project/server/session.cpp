#include <pch.h>

session::session()
{
}

session::~session()
{
	//_csa.Close();
}

void	session::on_complete_connect()
{
	{
		_exclusive_lock_(self());
		//_csa.Init(true);
	}
}

bool	session::on_route(const inbound_ptr_type& in_)
{
	return	false;
}

boost::optional< boost::tuple< uint64_t > >
session::get()	const
{
	_non_exclusive_lock_(self());
	return
	{
		{
			_compid
		}
	};
}

bool	session::set(
	uint64_t compid_)
{
	if (0 == compid_)
	{
		_error_log_(
			boost::format("%1% ( %2% )")
			% compid_
			% __FILE_LINE__);
		return	false;
	}

	_compid = compid_;

	return	true;
}
