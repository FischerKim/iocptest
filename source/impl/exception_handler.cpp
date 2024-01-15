#include <pch.h>
#include <windows.h>

#pragma warning ( disable : 4091 4191 )	//	dbghelp.h
#include <dbghelp.h>

typedef BOOL ( WINAPI *minidump_write_ptr )(	
	HANDLE,
	uint32_t,
	HANDLE,
	MINIDUMP_TYPE,
	const PMINIDUMP_EXCEPTION_INFORMATION,
	const PMINIDUMP_USER_STREAM_INFORMATION,
	const PMINIDUMP_CALLBACK_INFORMATION );

LPTOP_LEVEL_EXCEPTION_FILTER	g_previous_exception_filter	=	nullptr;

long	CALLBACK	unhandled_exception_filter( struct _EXCEPTION_POINTERS* exception_pointer_ptr )
{
	HMODULE	dll_handle = ::LoadLibraryA( "DBGHELP.DLL" );
	if ( nullptr == dll_handle )	return	EXCEPTION_CONTINUE_SEARCH;

	minidump_write_ptr	dump	=	( minidump_write_ptr )( ::GetProcAddress( dll_handle, "MiniDumpWriteDump" ) );
	if ( nullptr == dump )			return	EXCEPTION_CONTINUE_SEARCH;

	boost::posix_time::ptime	time( boost::posix_time::second_clock::local_time() );
	std::string					dump_file( boost::str( 
		boost::format( "%1%%2%.dmp" ) 
			%	impl::exception_handler::get_instance()->get_directory() 
			%	boost::posix_time::to_iso_string( time ) ) );

	HANDLE	file_handle = ::CreateFileA(	
		dump_file.c_str(),
		GENERIC_WRITE,
		FILE_SHARE_WRITE,
		NULL,
		CREATE_ALWAYS,
		FILE_ATTRIBUTE_NORMAL,
		NULL );

	if ( INVALID_HANDLE_VALUE == file_handle )		
		return	EXCEPTION_CONTINUE_SEARCH;

	_MINIDUMP_EXCEPTION_INFORMATION	minidump_exception_info;
	minidump_exception_info.ThreadId			=	::GetCurrentThreadId();
	minidump_exception_info.ExceptionPointers	=	exception_pointer_ptr;
	minidump_exception_info.ClientPointers		=	FALSE;

	BOOL	is_success = dump(	
		::GetCurrentProcess(),
		::GetCurrentProcessId(),
		file_handle,
		MiniDumpNormal,
		&minidump_exception_info,
		nullptr,
		nullptr );

	::CloseHandle( file_handle );

	return	( TRUE == is_success ) ? 
		EXCEPTION_EXECUTE_HANDLER : 
		EXCEPTION_CONTINUE_SEARCH;
}

namespace	impl
{
	exception_handler*	exception_handler::get_instance()
	{
		static	boost::movelib::unique_ptr< exception_handler >	inst;
		if ( nullptr == inst )
			inst.reset( new exception_handler() );

		return	inst.get();
	}

	exception_handler::exception_handler()
	{
	}

	exception_handler::~exception_handler()
	{
		end_dump();
	}

	void	exception_handler::begin_dump( const std::string_view& dir_ )
	{
		_dir	=	dir_;
		if ( true == dir_.empty() )
			_dir	=	"./";
		else
		{
			if ( false == boost::filesystem::exists( dir_.data() ) )
				boost::filesystem::create_directories( dir_.data() );
		}

		if ( _dir.size() - 1 != _dir.find_last_of( "/" ) )
			_dir	+=	"/";	

		g_previous_exception_filter = ::SetUnhandledExceptionFilter( unhandled_exception_filter );
	}

	void	exception_handler::end_dump()
	{
		::SetUnhandledExceptionFilter( g_previous_exception_filter );
	}
}

#pragma warning ( default : 4091 )