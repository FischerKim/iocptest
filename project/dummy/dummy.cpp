// dummy.cpp : 이 파일에는 'main' 함수가 포함됩니다. 거기서 프로그램 실행이 시작되고 종료됩니다.
//

#include <pch.h>
#include <boost/program_options.hpp>

static	impl::util::io_context& get_io()
{
	static	impl::util::io_context	io(boost::thread::hardware_concurrency() / 2 + 1);
	return	io;
}

int main(int argc_, char** argv_)
{
	impl::exception_handler::get_instance()->begin_dump("dump");

	namespace	po = boost::program_options;
	po::options_description	desc("Allowed options");
	desc.add_options()("ip", po::value< std::string >(), "");
	desc.add_options()("port", po::value< unsigned short >(), "");
	desc.add_options()("start_id", po::value< size_t >(), "");
	desc.add_options()("count", po::value< size_t >(), "");
	desc.add_options()("index", po::value< size_t >(), "");

	po::variables_map	vm;
	po::store(po::parse_command_line(argc_, argv_, desc), vm);
	po::notify(vm);

	size_t	index = 1;
	auto	pos(vm.find("index"));
	if (pos != vm.end())
		index = pos->second.as< size_t >();

	std::string	log(boost::str(boost::format("dummy_log_%1%") % index));
#ifdef _DEBUG	
	impl::log_tool::get_instance(log, log, boost::log::trivial::debug, true);
#else
	log_tool::get_instance(log, log, boost::log::trivial::info, true);
#endif

	std::string		ip("127.0.0.1");
	if (vm.end() != (pos = vm.find("ip")))
	{
		ip = pos->second.as< std::string >();
		if (true == ip.empty())
			ip = "127.0.0.1";
	}

	unsigned short	port = 10011;
	if (vm.end() != (pos = vm.find("port")) &&
		0 != pos->second.as< unsigned short >())
		port = pos->second.as< unsigned short >();

	size_t	start = 1;
	if (vm.end() != (pos = vm.find("start_id")))
		start = pos->second.as< size_t >();

	size_t	count = 1;
	if (vm.end() != (pos = vm.find("count")))
		count = pos->second.as< size_t >();

	std::vector< player_ptr >	v;
	for (size_t i = start; i < (start + count); ++i)
	{
		v.emplace_back(boost::make_shared< player >(get_io().get_io_context(), i));

		get_io().post(
			[s(v.back()), ip, port]() -> void
		{
			s->on_connect(ip, port);
		});
	}

	while (TRUE == ::SetConsoleCtrlHandler(
		[](unsigned long ctrl_) -> BOOL
		{
			switch (ctrl_)
			{
			case	CTRL_CLOSE_EVENT:
			case	CTRL_LOGOFF_EVENT:
			case	CTRL_SHUTDOWN_EVENT:
				impl::log_tool::get_instance()->stop();
				get_io().get_io_context().stop();
				return	TRUE;
			}

			return	FALSE;
		}, TRUE))
	{
		boost::this_thread::sleep(boost::posix_time::milliseconds(1000));
	}

		return	0;
}