/*!
* \class handler.ipp
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
* \date 2021/1/18
*
* Contact:	muse76@hotmail.com
*			muse4116@gmail.com
*
*/

#pragma once

namespace	impl::network::server
{
	template< typename session_type >
	handler< session_type >::handler( boost::asio::io_context& io_ )
		:	_io( io_ ),
			_keep_alive_task( boost::make_shared< util::timer::repeat_task >( io_ ) )
	{
	}

	template< typename session_type >
	handler< session_type >::~handler()
	{
	}

	template< typename session_type >
	void	handler< session_type >::on_start()
	{
		_keep_alive_task->start(	
			_keep_alive_time_,
			[ this ]() -> bool
			{
				_non_exclusive_lock_( _user_set );

				for ( const session_ptr_type& r_ : _user_set )
				{
					if ( nullptr == r_ )
					{
						_fatal_log_( boost::format( "( %1% )" ) % __FILE_LINE__ );
						continue;
					}

					if ( true == r_->on_is_keep_alive_flag() )		
					{
						r_->on_init_keep_alive_flag();
						continue;
					}

					r_->on_close();

					_trace_log_(
						boost::format( "%1%] %2% %3% ( %4% )" )
							%	__FUNCTION__
							%	r_->on_get_remote_ip()
							%	r_->on_get_remote_port()
							%	__FILE_LINE__ );
				}

				return	true;
			} );
	}

	template< typename session_type >
	void	handler< session_type >::on_stop()
	{
		{
			_exclusive_lock_( _user_set );
			_user_set.clear();
		}

		_keep_alive_task->stop();
	}

	template< typename session_type >
	bool	handler< session_type >::on_user_enter( const session_ptr_type& session_ )
	{
		if ( nullptr == session_ )
		{
			_fatal_log_( boost::format( "( %1% )" ) % __FILE_LINE__ );
			return false;
		}

		size_t	size	=	0;

		{
			_exclusive_lock_( _user_set );

			if ( true == is_session( session_ ) )
			{
				_error_log_( 
					boost::format(	"[on_user_enter] user : %1% %2%:%3% (%4%)" ) 
						%	session_->on_get_remote_ip() 
						%	session_->on_get_remote_port() 
						%	session_->on_is_connected() 
						%	__FILE_LINE__ );

				return false;
			}

			if ( false == _user_set.emplace( session_ ).second )
			{
				_error_log_( 
					boost::format(	"[on_user_enter] user : %1% %2%:%3% (%4%)" ) 
						%	session_->on_get_remote_ip() 
						%	session_->on_get_remote_port() 
						%	session_->on_is_connected() 
						%	__FILE_LINE__ );

				return false;
			}

			session_->on_set_keep_alive_flag();

			size	=	_user_set.size();
		}

		_info_log_( 
			boost::format( "%1% ] %2%" ) 
				%	__FUNCTION__ 
				%	size );

		return true;
	}

	template< typename session_type >
	bool	handler< session_type >::on_user_leave( const session_ptr_type& session_ )
	{
		if ( nullptr == session_ )
		{
			_fatal_log_( boost::format( "( %1% )" ) % __FILE_LINE__ );
			return false;
		}

		size_t	size	=	0;

		{
			_exclusive_lock_( _user_set );

			if ( false == is_session( session_ ) )
			{
				if ( true == session_->on_is_connected() )
				{
					_error_log_( 
						boost::format(	"[on_user_leave] user : %1% %2%:%3% (%4%)" ) 
							%	session_->on_get_remote_ip() 
							%	session_->on_get_remote_port() 
							%	session_->on_is_connected() 
							%	__FILE_LINE__ );
				}

				return false;
			}

			_user_set.erase( session_ );
			size	=	_user_set.size();
		}

		_info_log_( 
			boost::format( "%1% ] %2%" ) 
				%	__FUNCTION__ 
				%	size );

		return true;
	}

	template< typename session_type >
	boost::asio::io_context&	handler< session_type >::get_io_context()
	{
		return	_io;
	}

	template< typename session_type >
	size_t	handler< session_type >::get_user_size()	const
	{
		_non_exclusive_lock_( _user_set );
		return _user_set.size();
	}

	template< typename session_type >
	bool	handler< session_type >::is_session( const session_ptr_type& session_ )	const
	{
		if ( nullptr == session_ )
		{
			_fatal_log_( boost::format( "( %1% )" ) % __FILE_LINE__ );
			return false;
		}

		return _user_set.end() != _user_set.find( session_ );
	}

	template< typename session_type >
	boost::optional< std::set< boost::shared_ptr< session_type > > >	
		handler< session_type >::all_sessions()	const
	{
		_non_exclusive_lock_( _user_set );
		return
		{
			{ _user_set }
		};
	}
}