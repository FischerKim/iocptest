#pragma once

namespace	common
{
	class	base_server_instance : public service_interface
	{
	protected:
		enum
		{
			_timeout_task_time_ = 1000,
			_task_size_ = 8
		};

	private:
		static	base_server_instance* instance;

	public:
		static	base_server_instance* get_instance() { return	instance; }

	protected:
		explicit	base_server_instance(
			const std::vector< std::string_view >& options_,
			const std::string_view& option_file_);

		virtual bool	on_create(int argc_, char** argv_);
		virtual bool	on_listen() = 0;	//	set server_info & acceptor listen
		virtual bool	on_start();
		virtual void	on_stop();
		virtual void	on_destroy();
		virtual void	on_timer();

		bool	get_option(
			const std::string_view& key_,
			OUT std::string& value_);

		template< typename type > requires
			(true == std::is_same_v< std::string, type > ||
				true == std::is_unsigned_v< type > ||
				true == std::is_floating_point_v< type >)
			bool	get_option(const std::string_view& key_, OUT type& t_)
		{
			std::string	value;
			if (false == get_option(key_, value))	return	false;

			try
			{
				t_ = boost::lexical_cast<type>(value);
				return	true;
			}
			catch (const std::exception& e_)
			{
				_fatal_log_(boost::format("%1% ( %2% )") % e_.what() % __FILE_LINE__);
			}

			return	false;
		}

	private:
		bool	command_line_processor(int argc_, char** argv_);
		bool	load_option_file(const std::string_view& file_);

	public:
		impl::util::io_context& task_io() { return	_task_io; }

		const std::string& get_execute_path()	const { return	_execute_path; }

		const std::string& get_webhook_path()	const { return	_webhook_path; }

	private:
		impl::util::io_context					_task_io;	//	network & task

		std::vector< std::string >				_options;
		std::string								_option_file;

		using	option_filed = boost::tuple< std::string, std::string >;
		std::vector< option_filed >				_option_fields;
		std::string								_execute_path;

		std::string								_webhook_path;

		impl::util::timer::repeat_task_ptr_type	_timer;
	};
}