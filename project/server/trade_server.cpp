#include <pch.h>

server::server()
	: common::base_server_instance({ "port" }, ""),
	_handler(task_io()),
	_acceptor(_handler)
{
}

server* server::get_instance()
{
	static	boost::movelib::unique_ptr< server >	inst;
	if (nullptr == inst)
		inst.reset(new server());

	return	inst.get();
}

bool	server::on_listen()
{
	uint16_t	port = 0;
	if (false == get_option< uint16_t >("port", port))
	{
		_error_log_(boost::format("( %1% )") % __FILE_LINE__);
		return	false;
	}

	try
	{
		if (false == _acceptor.listen_start(port))
		{
			_error_log_(boost::format("( %1% )") % __FILE_LINE__);
			return	false;
		}

		_info_log_(
			boost::format("listen start! %1%")
			% port);

		return	true;
	}
	catch (const std::exception& e_)
	{
		_fatal_log_(boost::format("%1% ( %2% )") % e_.what() % __FILE_LINE__);
	}

	return	false;
}

bool	server::on_start()
{
	return	__super::on_start();
}

void	server::on_stop()
{
	__super::on_stop();
}

void	server::on_destroy()
{
	_acceptor.stop();
	_handler.on_stop();
	__super::on_destroy();
}

void	server::on_timer()
{
	__super::on_timer();
}