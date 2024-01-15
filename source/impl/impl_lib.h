/*!
* \class impl_lib.h
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

#define	TRACE

#ifndef		OUT
	#define		OUT
#endif

#ifndef		IN
	#define		IN
#endif

#ifndef		INOUT
	#define		INOUT
#endif

#ifndef		MOVE
	#define		MOVE
#endif

//	컴파일 출력 매크로
#ifndef		_line1
	#define _line1( x )		#x
#endif

#ifndef		_line
	#define	_line( x )		_line1( x )
#endif

#ifndef		_todo_
	#define	_todo_( msg )	message( __FILE__"(" _line( __LINE__ ) "): [TODO] " msg )	//	ex ) #pragma _todo_( msg )
#endif

#if defined( WINDOW ) && defined( USE_IMPL_LIB )
	#pragma message( "***************************************  LOAD IMPL **********************************************************" )
	#ifndef		_DEBUG		
		#pragma _todo_( "link impl_lib.lib" )
		#pragma comment( lib, "impl_lib.lib" )		
	#else
		#pragma _todo_( "link impl_lib_debug.lib" )
		#pragma comment( lib, "impl_lib_debug.lib" )		
	#endif
	#pragma message( "*************************************************************************************************************" )
	#pragma message( "\r\n" )

	#pragma message( "***************************************  LOAD ODBC **********************************************************" )
	#pragma _todo_( "link odbc32.lib" )
	#pragma comment( lib, "odbc32.lib" )
	#pragma _todo_( "link odbccp32.lib" )
	#pragma comment( lib, "odbccp32.lib" )
	#pragma message( "*************************************************************************************************************" )
	#pragma message( "\r\n" )
#endif

#ifdef VERSION_DISPLAY
	#define _ver		"0.1.0.0"
	#define _fix_day	"2020/03/30"

	#ifdef _DEBUG
		#ifndef _solution
			#define _solution	"debug  "
		#endif
	#else
		#ifndef _solution
			#define _solution	"release"
		#endif
	#endif

	#ifndef _WIN64
		#ifndef _plaform
			#define _plaform	"x86"
		#endif
	#else
		#ifndef _plaform
			#define _plaform	"x64"
		#endif
	#endif

	#pragma message( "****************************************"             "*********************************************************************" )
	#pragma message( "***************************************     impl_lib "             "                  **************************************" )
	#pragma message( "***************************************     ver (" _ver               ")              **************************************" )
	#pragma message( "***************************************     fix day (" _fix_day              ")       **************************************" )
	#pragma message( "***************************************     " _plaform " : " _solution "              **************************************" )
	#pragma message( "****************************************"             "*********************************************************************" )
	#pragma message( "\r\n" )
#endif

#pragma warning ( disable : 4996 )

#ifdef	WINDOW
	#define	WIN32_LEAN_AND_MEAN
	#include <SDKDDKVer.h>
#endif

#include <memory.h>
#include <algorithm>
#include <array>
#include <cassert>
#include <cctype>
#include <concepts>
#include <cstdint>
#include <cwctype>
#include <deque>
#include <exception>
#include <iterator>
#include <list>
#include <memory.h>
#include <queue>
#include <ranges>
#include <set>
#include <sstream>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#pragma warning ( default : 4996 )

#define	BOOST_ALLOW_DEPRECATED_HEADERS
//BOOST_HEADER_DEPRECATED("<boost/integer/integer_log2.hpp>");
#define BOOST_BIND_GLOBAL_PLACEHOLDERS

//	asio debugging
//	#define BOOST_ASIO_ENABLE_HANDLER_TRACKING

//  boost/bind.hpp:36
#define BOOST_BIND_GLOBAL_PLACEHOLDERS

#include <boost/asio.hpp>
#include <boost/process/environment.hpp>
#include <boost/noncopyable.hpp>
#include <boost/format.hpp>
#include <boost/move/unique_ptr.hpp>
#include <boost/make_unique.hpp>
#include <boost/smart_ptr.hpp>
#include <boost/smart_ptr/bad_weak_ptr.hpp>
#include <boost/smart_ptr/atomic_shared_ptr.hpp>
#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <boost/chrono.hpp>
#include <boost/random.hpp>
#include <boost/tokenizer.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/make_shared.hpp>
#include <boost/range/algorithm.hpp>
#include <boost/range/algorithm_ext.hpp>
#include <boost/thread.hpp>
#include <boost/atomic.hpp>
#include <boost/pool/pool_alloc.hpp>
#include <boost/pool/object_pool.hpp>
#include <boost/pool/singleton_pool.hpp>
#include <boost/filesystem.hpp>
#include <boost/locale.hpp>
#include <boost/variant.hpp>
#include <boost/any.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/uuid/random_generator.hpp>
#include <boost/functional/hash.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/json.hpp>
#include <boost/log/trivial.hpp>
#include <boost/optional.hpp>