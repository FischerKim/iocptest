/*!
* \class log_tool.h
*
* \ingroup
*
* \brief
*
* TODO: 버그 리포트는 아래 메일로 좀 부탁해요!!
*
* \note
*
* \author jay kim
*
* \version 1.0
*
* \date 2020/2/11
*
* Contact:	muse76@hotmail.com
*			muse4116@gmail.com
*
*/

#pragma once

namespace	impl
{
	class	log_tool final : boost::noncopyable
	{
	public:
#ifdef _DEBUG
		static	log_tool*	get_instance(	
			const std::string_view& key_ = "",
			const std::string_view& indirect_path_ = "",
			boost::log::trivial::severity_level severity_level_ = boost::log::trivial::severity_level::trace,
			bool	console_mode_ = false );
#else
		static	log_tool*	get_instance(	
			const std::string_view& key_ = "",
			const std::string_view& indirect_path_ = "",
			boost::log::trivial::severity_level severity_level_ = boost::log::trivial::severity_level::info,
			bool	console_mode_ = false );
#endif

	private:
#ifdef _DEBUG
		explicit	log_tool(	
			const std::string_view& key_ = "",
			const std::string_view& indirect_path_ = "",
			boost::log::trivial::severity_level severity_level_ = boost::log::trivial::severity_level::trace,
			bool	console_mode_ = false );
#else
		explicit	log_tool(	
			const std::string_view& key_ = "",
			const std::string_view& indirect_path_ = "",
			boost::log::trivial::severity_level severity_level_ = boost::log::trivial::severity_level::info,
			bool	console_mode_ = false );
#endif

	public:
		~log_tool()		{}

		void	stop();

		void	trace_log( const std::string_view& log_ );
		void	debug_log( const std::string_view& log_ );
		void	info_log( const std::string_view& log_ );
		void	warning_log( const std::string_view& log_ );
		void	error_log( const std::string_view& log_ );
		void	fatal_log( const std::string_view& log_ );

		void	trace_log( const std::wstring_view& log_ );
		void	debug_log( const std::wstring_view& log_ );
		void	info_log( const std::wstring_view& log_ );
		void	warning_log( const std::wstring_view& log_ );
		void	error_log( const std::wstring_view& log_ );
		void	fatal_log( const std::wstring_view& log_ );

		void	trace_log( const boost::format& log_ );
		void	debug_log( const boost::format& log_ );
		void	info_log( const boost::format& log_ );
		void	warning_log( const boost::format& log_ );
		void	error_log( const boost::format& log_ );
		void	fatal_log( const boost::format& log_ );

		void	trace_log( const boost::wformat& log_ );
		void	debug_log( const boost::wformat& log_ );
		void	info_log( const boost::wformat& log_ );
		void	warning_log( const boost::wformat& log_ );
		void	error_log( const boost::wformat& log_ );
		void	fatal_log( const boost::wformat& log_ );
	};
}

#ifdef	_DEBUG
	#define	TRACE_LOG
	#define	DEBUG_LOG
#endif

#ifdef	TRACE_LOG
	#define		_trace_log_( _l_ )		impl::log_tool::get_instance()->trace_log( _l_ )
#else
	#define		_trace_log_( _l_ )
#endif

#ifdef	DEBUG_LOG
	#define		_debug_log_( _l_ )		impl::log_tool::get_instance()->debug_log( _l_ )
#else
	#define		_debug_log_( _l_ )
#endif

#define		_info_log_( _l_ )		impl::log_tool::get_instance()->info_log( _l_ )
#define		_warning_log_( _l_ )	impl::log_tool::get_instance()->warning_log( _l_ )
#define		_error_log_( _l_ )		impl::log_tool::get_instance()->error_log( _l_ )
#define		_fatal_log_( _l_ )		impl::log_tool::get_instance()->fatal_log( _l_ )