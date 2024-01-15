#include <pch.h>

namespace	impl::util::timer
{
	repeat_task::repeat_task( boost::asio::io_context& io_ )
		:	_interval( boost::posix_time::milliseconds( static_cast< size_t >( _default_time_ ) ) ),
			_timer( io_ )
	{
	}

	repeat_task::~repeat_task()
	{
		stop();
	}

	//	return true == call repeat, false == end
	bool	repeat_task::start( size_t expire_ /* ms */, const bool_func_type& func_ )
	{
#ifdef	TRACE
		_spin_lock_fl_( _lock, __FILE_LINE__ );
#else
		_spin_lock_( _lock );
#endif

		if ( true == func_.empty() || false == _functor.empty() )	return	false;

		expire_	=	( _min_time_ > expire_ ) ? _min_time_ : expire_;

		_interval	=	boost::posix_time::milliseconds( expire_ );
		_functor	=	func_;

		_timer.expires_from_now( _interval );
		_timer.async_wait( boost::bind( &repeat_task::handler, shared_from_this(), _1 ) );

		return	true;
	}

	void	repeat_task::stop()
	{
#ifdef	TRACE
		_spin_lock_fl_( _lock, __FILE_LINE__ );
#else
		_spin_lock_( _lock );
#endif

		if ( true == _functor.empty() )		return;

		boost::system::error_code	ec;
		_timer.cancel( ec );
		_functor.clear();
	}

	bool	repeat_task::is_running()	const
	{
#ifdef	TRACE
		_spin_lock_fl_( _lock, __FILE_LINE__ );
#else
		_spin_lock_( _lock );
#endif
		return	false == _functor.empty();
	}

	void	repeat_task::handler( const boost::system::error_code& error_ )
	{
		if ( error_ )
		{
			if ( boost::asio::error::operation_aborted != error_.value() )
			{
				_error_log_( boost::format( "%1% ( %2% )" ) % error_.value() % __FILE_LINE__ );
			}

			return;
		}

#ifdef	TRACE
		_spin_lock_fl_( _lock, __FILE_LINE__ );
#else
		_spin_lock_( _lock );
#endif

		if ( true == _functor.empty() )		return;

		//	repeat end
		if ( false == _functor() )
		{
			_functor.clear();
			return;
		}

		_timer.expires_from_now( _interval );
		_timer.async_wait( boost::bind( &repeat_task::handler, shared_from_this(), _1 ) );
	}

	timed_task::timed_task( boost::asio::io_context& io_ ) : _io( io_ )	{}

	timed_task::~timed_task()
	{
		cancel();
	}

	void	timed_task::cancel()
	{
#ifdef	TRACE
		_spin_lock_fl_( _set, __FILE_LINE__ );
#else
		_spin_lock_( _set );
#endif

		if ( true == _set.empty() )		return;

		for ( const timer_shared_ptr_type& r_ : _set )
			r_->cancel();

		_set.clear();
	}

	bool	timed_task::invoke( size_t expire_ /* ms */, const void_func_type& func_ )
	{
		if ( 0 == expire_ || true == func_.empty() )		return	false;

		auto	sp( boost::make_shared< boost::asio::deadline_timer >( _io ) );
			
		{
#ifdef	TRACE
			_spin_lock_fl_( _set, __FILE_LINE__ );
#else
			_spin_lock_( _set );
#endif

			if ( false == _set.emplace( sp ).second )
			{
				_error_log_( boost::format( "( %1% )" ) % __FILE_LINE__ );
				return	false;
			}	
		}

		sp->expires_from_now( boost::posix_time::milliseconds( expire_ ) );

		sp->async_wait(	
			[	this, 
				self = shared_from_this(), 
				wp( timer_weak_ptr_type( sp ) ), 
				func_ ]( const boost::system::error_code& ec_ ) -> void 
			{
				if ( ec_ )
				{
					if ( boost::asio::error::operation_aborted != ec_.value() )
						_error_log_( boost::format( "%1% ( %2% )" ) % ec_.value() % __FILE_LINE__ );

					return;
				}

				timer_shared_ptr_type	sp( wp.lock() );
				if ( nullptr == sp )	return;

				if ( false == func_.empty() )
					func_();

#ifdef	TRACE
				_spin_lock_fl_( _set, __FILE_LINE__ );
#else
				_spin_lock_( _set );
#endif
				_set.erase( sp );
			} );

		return true;
	}

	bool	timed_task::invoke( boost::asio::io_context& io_, size_t expire_ /* ms */, const void_func_type& func_ )
	{
		if ( 0 == expire_ || true == func_.empty() )		return	false;

		auto	sp( boost::make_shared< boost::asio::deadline_timer >( io_ ) );

		sp->expires_from_now( boost::posix_time::milliseconds( expire_ ) );

		sp->async_wait(	
			[	sp = boost::move( sp ), 
				func_ ]( const boost::system::error_code& error_ ) -> void
			{
				if ( error_ )
				{
					if ( boost::asio::error::operation_aborted != error_.value() )
						_error_log_( boost::format( "%1% ( %2% )" ) % error_.value() % __FILE_LINE__ );

					return;
				}

				if ( true == func_.empty() )	return;

				func_();
			} );

		return	true;
	}
}