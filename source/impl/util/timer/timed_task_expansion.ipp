#pragma once

namespace	impl::util::timer
{
	template < typename key > requires ( std::is_integral_v< key > && 4 >= sizeof( key ) )
	bool	timed_task_expansion< key >::invoke( const key& key_, size_t expire_ /* ms */, const void_func_type& func_ )
	{
		if ( 0 == key_ || 0 == expire_ || true == func_.empty() )		return	false;

		auto	sp( boost::make_shared< boost::asio::deadline_timer >( _io ) );
			
		{
#ifdef	TRACE
			_spin_lock_fl_( _map, __FILE_LINE__ );
#else
			_spin_lock_( _map );
#endif

			if ( false == _map.try_emplace( key_, sp ).second )
			{
				_error_log_( boost::format( "%1% ( %2% )" ) % key_ % __FILE_LINE__ );
				return	false;
			}
		}

		sp->expires_from_now( boost::posix_time::milliseconds( expire_ ) );

		sp->async_wait(	
			[	this, 
				self = shared_from_this(), 
				wp( timed_task::timer_weak_ptr_type( sp ) ), 
				func_, 
				key_ ]( const boost::system::error_code& ec_ ) -> void
			{
				if ( ec_ )
				{
					if ( boost::asio::error::operation_aborted != ec_.value() )
						_error_log_( boost::format( "%1% ( %2% )" ) % ec_.value() % __FILE_LINE__ );

					return;
				}

				timed_task::timer_shared_ptr_type	t( wp.lock() );
				if ( nullptr == t )	return;

				if ( false == func_.empty() )
					func_();

#ifdef	TRACE
				_spin_lock_fl_( _map, __FILE_LINE__ );
#else
				_spin_lock_( _map );
#endif
				_map.erase( key_ );
			} );

		return	true;
	}

	template < typename key > requires ( std::is_integral_v< key > && 4 >= sizeof( key ) )
	void	timed_task_expansion< key >::cancel( const key& key_ )
	{
#ifdef	TRACE
		_spin_lock_fl_( _map, __FILE_LINE__ );
#else
		_spin_lock_( _map );
#endif

		typename lock_map_type::iterator	pos	=	_map.find( key_ );

		if ( pos == _map.end() )	return;

		pos->second->cancel();

		_map.erase( pos );
	}

	template < typename key >  requires ( std::is_integral_v< key > && 4 >= sizeof( key ) )
	void	timed_task_expansion< key >::cancel()
	{
		timed_task::cancel();

#ifdef	TRACE
		_spin_lock_fl_( _map, __FILE_LINE__ );
#else
		_spin_lock_( _map );
#endif

		for ( const lock_map_type::value_type& r_ : _map )
			r_.second->cancel();

		_map.clear();
	}
}