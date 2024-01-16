#pragma once

#define	_make64_( _h_, _l_ )	( static_cast< uint64_t >( _h_ & 0xffffffff ) << 32 | ( _l_ & 0xffffffff ) )
#define	_makelong_( _h_, _l_ )	( static_cast< uint32_t >( _h_ & 0xffff ) << 16 | ( _l_ & 0xffff ) )
#define	_makeshort( _h_, _l_ )	( static_cast< uint16_t >( _h_ & 0xff ) << 8 | ( _l_ & 0xff ) )
#define	_lolong_( _l_ )			( static_cast< uint32_t >( _l_ & 0xffffffff ) )
#define	_hilong_( _h_ )			( static_cast< uint32_t >( ( _h_ >> 32 ) & 0xffffffff ) )
#define	_loshort_( _l_ )		( static_cast< uint16_t >( _l_ & 0xffff ) )
#define	_hishort_( _h_ )		( static_cast< uint16_t >( ( _h_ >> 16 ) & 0xffff ) )
#define	_lobyte_( _l_ )			( static_cast< uint8_t >( _l_ & 0xff ) )
#define	_hibyte_( _h_ )			( static_cast< uint8_t >( ( _h_ >> 8 ) & 0xff ) )

using	void_func_type				=	boost::function< void() >;
using	bool_func_type				=	boost::function< bool() >;
using	timed_lock_type				=	boost::unique_lock< boost::timed_mutex >;
using	exclusive_lock_type			=	boost::unique_lock< boost::shared_mutex >;
using	non_exclusive_lock_type		=	boost::shared_lock< boost::shared_mutex >;
using	upgrade_lock_type			=	boost::upgrade_lock< boost::shared_mutex >;
using	upgrade_to_unique_lock_type	=	boost::upgrade_to_unique_lock< boost::shared_mutex >;

#define _unique_lock_( _lock_ )					boost::mutex::scoped_lock	ul( _lock_ )
#define	_unique_lock_ex_( _v_, _lock_ )			boost::mutex::scoped_lock	_v_( _lock_ )
#define _timed_lock_( _lock_ )					timed_lock_type				tl( _lock_, boost::try_to_lock );
#define _timed_lock_ex_( _v_, _lock_ )			timed_lock_type				_v_( _lock_, boost::try_to_lock );
#define _exclusive_lock_( _lock_ )				exclusive_lock_type			el( _lock_ )
#define	_exclusive_lock_ex_( _v_, _lock_ )		exclusive_lock_type			_v_( _lock_ )
#define _non_exclusive_lock_( _lock_ )			non_exclusive_lock_type		nel( _lock_ )
#define	_non_exclusive_lock_ex_( _v_, _lock_ )	non_exclusive_lock_type		_v_( _lock_ )
#define _upgrade_lock_( _lock_ )				upgrade_lock_type			ul( _lock_ )
#define	_upgrade_to_unique_lock_( _ul_ )		upgrade_to_unique_lock_type	utul( _ul_ )

#define _const_cast_mutex_( ref_ )				( *const_cast< boost::mutex* >( &ref_ ) )
#define	_const_cast_shared_mutex_( ref_ )		( *const_cast< boost::shared_mutex* >( &ref_ ) )

#define __FILE_LINE__			boost::str( boost::format( "%1%( %2% )" ) % __FILE__ % __LINE__ )
#define	__LINE_STRING__			boost::str( boost::format( "( %1% )" ) % __LINE__ )

#define	__L_FILE_LINE__			boost::str( boost::wformat( L"%1%( %2% )" ) % __FILE__ % __LINE__ )
#define __L_LINE_STRING__		boost::str( boost::wformat( L"( %1% )" ) % __LINE__ )

#ifndef interface
	#define interface class
#endif
