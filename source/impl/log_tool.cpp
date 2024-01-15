#include <pch.h>

#include <boost/core/null_deleter.hpp>
#include <boost/log/common.hpp>
#include <boost/log/attributes/timer.hpp>
#include <boost/log/attributes/counter.hpp>
#include <boost/log/attributes/clock.hpp>
#include <boost/log/sinks/text_ostream_backend.hpp>
#include <boost/log/sinks.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/utility/record_ordering.hpp>
#include <boost/log/support/date_time.hpp>
#include <boost/locale/generator.hpp>

namespace	impl
{
	using	namespace	boost::log;

	log_tool*	log_tool::get_instance( 
		const std::string_view& key_, 
		const std::string_view& indirect_path_, 
		trivial::severity_level severity_level_, 
		bool console_mode_ )
	{
		static	boost::movelib::unique_ptr< log_tool >	inst;

		if ( nullptr == inst )
		{
			inst.reset( new log_tool( 
				key_, 
				indirect_path_, 
				severity_level_, 
				console_mode_ ) );
		}

		return	inst.get();
	}

	log_tool::log_tool( 
		const std::string_view& key_, 
		const std::string_view& indirect_path_, 
		trivial::severity_level severity_level_, 
		bool console_mode_ )
	{
		if ( true == indirect_path_.empty() )
			throw	std::logic_error( "indirect path empty!" );

		std::locale	loc( boost::locale::generator()( "en_US.UTF-8" ) );

		{
			using	sink_type	=	
				sinks::asynchronous_sink
				<
					sinks::text_file_backend,
					sinks::bounded_ordering_queue
					< 
						attribute_value_ordering< uint64_t, std::less< uint64_t > >, 
						128, 
						sinks::block_on_overflow 
					>
				>;

			trivial::severity_level	lvl	=	severity_level_;

			while ( trivial::fatal >= lvl )
			{
				std::string	file_name	=	( true == key_.empty() )	?
					boost::str( boost::format( "%1%_" ) % trivial::to_string( lvl ) ) :
					boost::str( boost::format( "%1%_%2%_" ) % key_ % trivial::to_string( lvl ) );
				file_name	+=	"%Y%m%d_%5N.log";

				boost::shared_ptr< sink_type >	sink =
					boost::make_shared< sink_type >
					(
						(
							keywords::file_name				=	file_name,
							keywords::target_file_name		=	file_name,
							keywords::rotation_size			=	16 * 1024 * 1024,	//	16 mb
							keywords::time_based_rotation	=	sinks::file::rotation_at_time_point( 0, 0, 0 ),
							keywords::auto_flush			=	true
						),
						keywords::order = make_attr_ordering< uint64_t >( "record_id", std::less< uint64_t >() )
					);

				std::string	dir_path( boost::str( 
					boost::format( "%1%\\%2%" ) 
						%	indirect_path_ 
						%	trivial::to_string( lvl ) ) );

				sink->locked_backend()->set_file_collector
					(
						sinks::file::make_collector
						(
							keywords::target			=	dir_path,
							keywords::max_size			=	1024 * 1024 * 1024,	//	1 gb
							keywords::min_free_space	=	128 * 1024 * 1024	//	128 mb
						)
					);

				sink->set_formatter
					(
						expressions::format( "[ %1% ][ %2% ][ %3% ][ %4% ] %5%" )
							%	expressions::attr< uint64_t >( "record_id" )
							%	expressions::format_date_time< boost::posix_time::ptime >( "timestamp", "%m-%d %H:%M:%S.%f" )
							%	expressions::format_date_time< attributes::timer::value_type >( "uptime", "%O:%M:%S" )
							%	expressions::attr< boost::thread::id >( "thread_id" )
							%	expressions::message
					);

				sink->locked_backend()->scan_for_files();
				sink->set_filter( trivial::severity == lvl );
				sink->imbue( loc );

				core::get()->add_sink( sink );
				
				uint32_t	level	=	lvl;
				level++;
				lvl	=	static_cast< trivial::severity_level >( level );
			}
		}

		if ( true == console_mode_ )
		{
			using	sink_type	=	sinks::synchronous_sink< sinks::text_ostream_backend >;

			boost::shared_ptr< sink_type >	sink( boost::make_shared< sink_type >() );
			sink->locked_backend()->add_stream( boost::shared_ptr< std::ostream >( &std::clog, boost::null_deleter() ) );
			sink->locked_backend()->auto_flush( true );
			sink->set_filter( trivial::severity >= severity_level_ );
			sink->imbue( loc );
			sink->set_formatter
				(
					expressions::format( "[ %1% ][ %2% ][ %3% ][ %4% ][ %5% ] %6%" )
						%	expressions::attr< uint64_t >( "record_id" )
						%	expressions::format_date_time< boost::posix_time::ptime >( "timestamp", "%m-%d %H:%M:%S.%f" )
						%	expressions::format_date_time< attributes::timer::value_type >( "uptime", "%O:%M:%S" )
						%	expressions::attr< boost::thread::id >( "thread_id" )
						%	trivial::severity
						%	expressions::message
				);

			core::get()->add_sink( sink );
		}

#ifdef	_DEBUG
		{
			using	sink_type	=	sinks::synchronous_sink< sinks::debug_output_backend >;

			boost::shared_ptr< sink_type >	sink( boost::make_shared< sink_type >() );
			sink->set_filter( trivial::severity >= trivial::trace );
			sink->imbue( loc );
			sink->set_formatter
				(
					expressions::format( "[ %1% ][ %2% ][ %3% ][ %4% ][ %5% ] %6%\n" )
						%	expressions::attr< uint64_t >( "record_id" )
						%	expressions::format_date_time< boost::posix_time::ptime >( "timestamp", "%m-%d %H:%M:%S.%f" )
						%	expressions::format_date_time< attributes::timer::value_type >( "uptime", "%O:%M:%S" )
						%	expressions::attr< boost::thread::id >( "thread_id" )
						%	trivial::severity
						%	expressions::message
				);

			core::get()->add_sink( sink );
		}
#endif

		core::get()->add_global_attribute( "record_id", attributes::counter< uint64_t >() );
		core::get()->add_global_attribute( "timestamp", attributes::local_clock() );
		core::get()->add_global_attribute( "uptime", attributes::timer() );
	}

	void	log_tool::stop()
	{
		core::get()->flush();
		core::get()->remove_all_sinks();
	}

	void	log_tool::trace_log( const std::string_view& log_ )
	{
		sources::severity_logger_mt< trivial::severity_level >	slg;
		BOOST_LOG_SCOPED_THREAD_TAG( "thread_id", boost::this_thread::get_id() );
		BOOST_LOG_SEV( slg, trivial::trace ) << log_;
	}

	void	log_tool::debug_log( const std::string_view& log_ )
	{
		sources::severity_logger_mt< trivial::severity_level >	slg;
		BOOST_LOG_SCOPED_THREAD_TAG( "thread_id", boost::this_thread::get_id() );
		BOOST_LOG_SEV( slg, trivial::debug ) << log_;
	}

	void	log_tool::info_log( const std::string_view& log_ )
	{
		sources::severity_logger_mt< trivial::severity_level >	slg;
		BOOST_LOG_SCOPED_THREAD_TAG( "thread_id", boost::this_thread::get_id() );
		BOOST_LOG_SEV( slg, trivial::info ) << log_;
	}

	void	log_tool::warning_log( const std::string_view& log_ )
	{
		sources::severity_logger_mt< trivial::severity_level >	slg;
		BOOST_LOG_SCOPED_THREAD_TAG( "thread_id", boost::this_thread::get_id() );
		BOOST_LOG_SEV( slg, trivial::warning ) << log_;
	}

	void	log_tool::error_log( const std::string_view& log_ )
	{
		sources::severity_logger_mt< trivial::severity_level >	slg;
		BOOST_LOG_SCOPED_THREAD_TAG( "thread_id", boost::this_thread::get_id() );
		BOOST_LOG_SEV( slg, trivial::error ) << log_;
	}

	void	log_tool::fatal_log( const std::string_view& log_ )
	{
		sources::severity_logger_mt< trivial::severity_level >	slg;
		BOOST_LOG_SCOPED_THREAD_TAG( "thread_id", boost::this_thread::get_id() );
		BOOST_LOG_SEV( slg, trivial::fatal ) << log_;
	}

	void	log_tool::trace_log( const std::wstring_view& log_ )
	{
		sources::wseverity_logger_mt< trivial::severity_level >	slg;
		BOOST_LOG_SCOPED_THREAD_TAG( "thread_id", boost::this_thread::get_id() );
		BOOST_LOG_SEV( slg, trivial::trace ) << log_;
	}

	void	log_tool::debug_log( const std::wstring_view& log_ )
	{
		sources::wseverity_logger_mt< trivial::severity_level >	slg;
		BOOST_LOG_SCOPED_THREAD_TAG( "thread_id", boost::this_thread::get_id() );
		BOOST_LOG_SEV( slg, trivial::debug ) << log_;
	}

	void	log_tool::info_log( const std::wstring_view& log_ )
	{
		sources::wseverity_logger_mt< trivial::severity_level >	slg;
		BOOST_LOG_SCOPED_THREAD_TAG( "thread_id", boost::this_thread::get_id() );
		BOOST_LOG_SEV( slg, trivial::info ) << log_;
	}

	void	log_tool::warning_log( const std::wstring_view& log_ )
	{
		sources::wseverity_logger_mt< trivial::severity_level >	slg;
		BOOST_LOG_SCOPED_THREAD_TAG( "thread_id", boost::this_thread::get_id() );
		BOOST_LOG_SEV( slg, trivial::warning ) << log_;
	}

	void	log_tool::error_log( const std::wstring_view& log_ )
	{
		sources::wseverity_logger_mt< trivial::severity_level >	slg;
		BOOST_LOG_SCOPED_THREAD_TAG( "thread_id", boost::this_thread::get_id() );
		BOOST_LOG_SEV( slg, trivial::error ) << log_;
	}

	void	log_tool::fatal_log( const std::wstring_view& log_ )
	{
		sources::wseverity_logger_mt< trivial::severity_level >	slg;
		BOOST_LOG_SCOPED_THREAD_TAG( "thread_id", boost::this_thread::get_id() );
		BOOST_LOG_SEV( slg, trivial::fatal ) << log_;
	}

	void	log_tool::trace_log( const boost::format& log_ )
	{
		sources::severity_logger_mt< trivial::severity_level >	slg;
		BOOST_LOG_SCOPED_THREAD_TAG( "thread_id", boost::this_thread::get_id() );
		BOOST_LOG_SEV( slg, trivial::trace ) << log_;
	}

	void	log_tool::debug_log( const boost::format& log_ )
	{
		sources::severity_logger_mt< trivial::severity_level >	slg;
		BOOST_LOG_SCOPED_THREAD_TAG( "thread_id", boost::this_thread::get_id() );
		BOOST_LOG_SEV( slg, trivial::debug ) << log_;
	}

	void	log_tool::info_log( const boost::format& log_ )
	{
		sources::severity_logger_mt< trivial::severity_level >	slg;
		BOOST_LOG_SCOPED_THREAD_TAG( "thread_id", boost::this_thread::get_id() );
		BOOST_LOG_SEV( slg, trivial::info ) << log_;
	}

	void	log_tool::warning_log( const boost::format& log_ )
	{
		sources::severity_logger_mt< trivial::severity_level >	slg;
		BOOST_LOG_SCOPED_THREAD_TAG( "thread_id", boost::this_thread::get_id() );
		BOOST_LOG_SEV( slg, trivial::warning ) << log_;
	}

	void	log_tool::error_log( const boost::format& log_ )
	{
		sources::severity_logger_mt< trivial::severity_level >	slg;
		BOOST_LOG_SCOPED_THREAD_TAG( "thread_id", boost::this_thread::get_id() );
		BOOST_LOG_SEV( slg, trivial::error ) << log_;
	}

	void	log_tool::fatal_log( const boost::format& log_ )
	{
		sources::severity_logger_mt< trivial::severity_level >	slg;
		BOOST_LOG_SCOPED_THREAD_TAG( "thread_id", boost::this_thread::get_id() );
		BOOST_LOG_SEV( slg, trivial::fatal ) << log_;
	}

	void	log_tool::trace_log( const boost::wformat& log_ )
	{
		sources::wseverity_logger_mt< trivial::severity_level >	slg;
		BOOST_LOG_SCOPED_THREAD_TAG( "thread_id", boost::this_thread::get_id() );
		BOOST_LOG_SEV( slg, trivial::trace ) << log_;
	}

	void	log_tool::debug_log( const boost::wformat& log_ )
	{
		sources::wseverity_logger_mt< trivial::severity_level >	slg;
		BOOST_LOG_SCOPED_THREAD_TAG( "thread_id", boost::this_thread::get_id() );
		BOOST_LOG_SEV( slg, trivial::debug ) << log_;
	}

	void	log_tool::info_log( const boost::wformat& log_ )
	{
		sources::wseverity_logger_mt< trivial::severity_level >	slg;
		BOOST_LOG_SCOPED_THREAD_TAG( "thread_id", boost::this_thread::get_id() );
		BOOST_LOG_SEV( slg, trivial::info ) << log_;
	}

	void	log_tool::warning_log( const boost::wformat& log_ )
	{
		sources::wseverity_logger_mt< trivial::severity_level >	slg;
		BOOST_LOG_SCOPED_THREAD_TAG( "thread_id", boost::this_thread::get_id() );
		BOOST_LOG_SEV( slg, trivial::warning ) << log_;
	}

	void	log_tool::error_log( const boost::wformat& log_ )
	{
		sources::wseverity_logger_mt< trivial::severity_level >	slg;
		BOOST_LOG_SCOPED_THREAD_TAG( "thread_id", boost::this_thread::get_id() );
		BOOST_LOG_SEV( slg, trivial::error ) << log_;
	}

	void	log_tool::fatal_log( const boost::wformat& log_ )
	{
		sources::wseverity_logger_mt< trivial::severity_level >	slg;
		BOOST_LOG_SCOPED_THREAD_TAG( "thread_id", boost::this_thread::get_id() );
		BOOST_LOG_SEV( slg, trivial::fatal ) << log_;
	}
}