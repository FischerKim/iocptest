#include <pch.h>

namespace	common
{
	service_interface* service_interface::_this = nullptr;

	service_interface::service_interface()
		: _service_status_handle(nullptr),
		_state(0),
		_is_pause(false),
		_exit_event(nullptr)
	{
		_this = this;
	}

	int		service_interface::run(int argc_, char** argv_)
	{
		_argc = argc_;
		_argv = argv_;

		namespace	po = boost::program_options;

		po::options_description	desc("Allowed options");
		desc.add_options()("service_name", po::value< std::string >(), "");
		desc.add_options()("service_mode", po::value< int >(), "");

		char* av[] = { argv_[0], argv_[1], argv_[2] };

		po::variables_map	vm;
		po::store(po::parse_command_line(3, av, desc), vm);
		po::notify(vm);

		_service_name = boost::locale::conv::to_utf< wchar_t >(vm["service_name"].as< std::string >(), "UTF-8");
		if (0 != vm["service_mode"].as< int >())
		{
			SERVICE_TABLE_ENTRY s[] =
			{
				{
					const_cast<wchar_t*>(_service_name.c_str()),
					(LPSERVICE_MAIN_FUNCTION)service_interface::callback_main
				},
				{
					nullptr,
					nullptr
				}
			};

			::StartServiceCtrlDispatcher(s);
			return 0;
		}

		if (false == on_create(argc_, argv_))		return 0;

		while (TRUE == ::SetConsoleCtrlHandler(
			[](unsigned long ctrl_) -> BOOL
			{
				switch (ctrl_)
				{
				case	CTRL_CLOSE_EVENT:
				case	CTRL_LOGOFF_EVENT:
				case	CTRL_SHUTDOWN_EVENT:
					_this->on_destroy();
					return	TRUE;
				}

				return	FALSE;
			},
			TRUE))
		{
			boost::this_thread::sleep(boost::posix_time::milliseconds(1));
		}

			return	0;
	}

	void	service_interface::callback_main()
	{
		_this->service_main();
	}

	void	service_interface::callback_handler(uint32_t control_)
	{
		_this->service_handler(control_);
	}

	void	service_interface::service_main()
	{
		if (0 == (_service_status_handle =
			::RegisterServiceCtrlHandler(
				const_cast<wchar_t*>(_service_name.c_str()),
				(LPHANDLER_FUNCTION)service_interface::callback_handler)))
			return;

		set_status(SERVICE_START_PENDING);
		_is_pause = false;

		if (false == on_create(_argc, _argv))
		{
			set_status(SERVICE_STOPPED);
			return;
		}

		if (nullptr == (_exit_event = ::CreateEvent(nullptr, TRUE, FALSE, _service_name.c_str())))		return;

		set_status(SERVICE_RUNNING);

		while (true)
		{
			if (WAIT_OBJECT_0 == ::WaitForSingleObject(_exit_event, INFINITE))
				break;
		}

		on_destroy();
		set_status(SERVICE_STOPPED);
	}

	void	service_interface::service_handler(uint32_t control_)
	{
		if (control_ == _state)		return;

		switch (control_)
		{
		case SERVICE_CONTROL_PAUSE:
			set_status(SERVICE_PAUSE_PENDING, 0);
			_is_pause = true;
			set_status(SERVICE_PAUSED);
			break;

		case SERVICE_CONTROL_CONTINUE:
			set_status(SERVICE_CONTINUE_PENDING, 0);
			_is_pause = false;
			set_status(SERVICE_RUNNING);
			break;

		case SERVICE_CONTROL_STOP:
			set_status(SERVICE_STOP_PENDING, 0);
			::SetEvent(_exit_event);
			break;

		case SERVICE_CONTROL_INTERROGATE:
		default:
			set_status(_state);
		}
	}

	void	service_interface::set_status(
		uint32_t state_,
		uint32_t accept_ /*= SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_PAUSE_CONTINUE*/)
	{
		SERVICE_STATUS	s = { 0 };
		s.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
		s.dwCurrentState = state_;
		s.dwControlsAccepted = accept_;
		_state = state_;
		::SetServiceStatus(_service_status_handle, &s);
	}

	base_server_instance* base_server_instance::instance = nullptr;

	base_server_instance::base_server_instance(
		const std::vector< std::string_view >& options_,
		const std::string_view& option_file_)
		: _option_file(option_file_.data())
	{
		boost::range::for_each(
			options_,
			[&](const std::string_view& r_) -> void
			{
				_options.emplace_back(r_.data());
			});

		_options.emplace_back("service_name");
		_options.emplace_back("service_mode");
		_options.emplace_back("execute_path");
		_options.emplace_back("log_key");
		_options.emplace_back("log_level");
		_options.emplace_back("log_console");
		_options.emplace_back("task_size");

		base_server_instance::instance = this;
	}

	bool	base_server_instance::on_create(int argc_, char** argv_)
	{
		if (false == command_line_processor(argc_, argv_))
		{
			throw	std::exception(boost::str(
				boost::format("%1% %2% ( %3% )")
				% argc_
				% argv_
				% __FILE_LINE__).c_str());
		}

		uint32_t	thread_size = 0;
		if (false == get_option< uint32_t >("task_size", thread_size))
			_task_io.run();
		else
			(_task_size_ <= thread_size) ? _task_io.run(thread_size) : _task_io.run();

		std::string	log_key;
		if (false == get_option("log_key", log_key))				return	false;

		std::string	log_path(_execute_path + log_key);

		size_t	log_level = 0;
		if (false == get_option< size_t >("log_level", log_level))
			log_level = boost::log::trivial::severity_level::info;

		if (log_level > boost::log::trivial::severity_level::info)
			log_level = boost::log::trivial::severity_level::info;

#ifdef	_DEBUG
		size_t	log_console = 1;
#else
		size_t	log_console = 0;
		if (false == get_option< size_t >("log_console", log_console))			return false;
#endif

		log::log_tool::get_instance(
			log_key,
			log_path,
			static_cast<boost::log::trivial::severity_level>(log_level),
			(log_console > 0));

		_info_log_(boost::format("task thread size : %1% ") % _task_io.get_thread_size());

		if (false == on_listen())
		{
			_error_log_(boost::format("( %1% )") % __FILE_LINE__);
			return	false;
		}

		load_db();
		return	on_start();
	}

	bool	base_server_instance::on_start()
	{
		_timer = boost::make_shared< util::timer::repeat_task >(_task_io.get_io_context());
		_timer->start(
			_timeout_task_time_,
			[&]() -> bool
			{
				on_timer();
				return	true;
			});
		return	true;
	}

	void	base_server_instance::on_stop()
	{
		if (nullptr == _timer)	return;

		_timer->stop();
	}

	void	base_server_instance::on_destroy()
	{
		on_stop();

		log::log_tool::get_instance()->stop();

		_db_io.stop();
		_task_io.stop();
	}

	void	base_server_instance::on_timer()
	{
	}

	bool	base_server_instance::get_option(
		const std::string_view& key_,
		OUT std::string& value_)
	{
		if (true == key_.empty())	return	false;

		std::vector< option_filed >::const_iterator	pos =
			boost::range::find_if(
				_option_fields,
				[&key_](const option_filed& r_) -> bool
				{
					return	0 == r_.get< 0 >().compare(key_.data());
				});
		if (_option_fields.end() == pos)	return	false;

		value_ = boost::get< 1 >(*pos);
		return	false == value_.empty();
	}

	bool	base_server_instance::command_line_processor(int argc_, char** argv_)
	{
		if (true == _options.empty() || 2 > argc_)		return	false;

		namespace	po = boost::program_options;

		po::options_description	desc("Allowed options");

		for (const std::string& ref_ : _options)
			desc.add_options()(ref_.c_str(), po::value< std::string >(), "");

		po::variables_map	vm;
		po::store(po::parse_command_line(argc_, argv_, desc), vm);
		po::notify(vm);

		boost::range::for_each(
			_options,
			[&](const std::string& r_) -> void
			{
				po::variables_map::const_iterator	pos = vm.find(r_.c_str());
				if (pos == vm.end())	return;

				_option_fields.emplace_back(r_, pos->second.as< std::string >());
			});

		if (false == get_option("execute_path", _execute_path))	return	false;

		std::string	opt_file;
		if (false == _option_file.empty())
		{
			opt_file = boost::str(
				boost::format("%1%%2%")
				% _execute_path
				% _option_file);

			if (false == boost::filesystem::exists(opt_file))	return	false;
			if (false == load_option_file(opt_file))			return	false;
		}

		return	true;
	}

	bool	base_server_instance::load_option_file(const std::string_view& file_)
	{
		if (true == file_.empty())	return	false;

		try
		{
			std::string	s;

			{
				std::ifstream	ifs;
				ifs.open(file_.data(), std::ios::in);
				if (false == ifs.is_open())
				{
					_fatal_log_(
						boost::format("%1% ( %2% )")
						% file_.data()
						% __FILE_LINE__);
					return	false;
				}

				ifs.seekg(0, std::ios::end);
				size_t	size = ifs.tellg();
				s.resize(size);
				ifs.seekg(0, std::ios::beg);

				ifs.read(s.data(), size);
				ifs.close();
			}

			namespace	json = boost::json;
			json::value	root(json::parse(s.c_str()));
			boost::range::for_each(
				_options,
				[&](const std::string& r_) -> void
				{
					std::vector< option_filed >::const_iterator	pos =
						boost::range::find_if(
							_option_fields,
							[&r_](const option_filed& of_) -> bool
							{
								return	0 == r_.compare(of_.get< 0 >());
							});
					if (_option_fields.end() != pos)	return;

					const json::value& j = root.at(r_.c_str());
					if (true == j.is_string())
					{
						_option_fields.emplace_back(r_, json::value_to< std::string >(j));
						return;
					}

					if (true == j.is_number())
					{
						_option_fields.emplace_back(
							r_,
							boost::lexical_cast<std::string>(json::value_to< size_t >(j)));

						return;
					}

					_error_log_(
						boost::format("%1% %2% ( %3% )")
						% r_
						% file_.data()
						% __FILE_LINE__);
				});

			return	true;
		}
		catch (const std::exception& e_)
		{
			_fatal_log_(
				boost::format("%1% %2% ( %3% )")
				% file_.data()
				% e_.what()
				% __FILE_LINE__);
		}

		return	false;
	}
}