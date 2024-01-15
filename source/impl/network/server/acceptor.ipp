/*!
* \class acceptor.ipp
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
	template< typename handler_type, typename session_type >
	acceptor< handler_type, session_type >::acceptor( handler_type& server_handler_ )
		:	_acceptor( server_handler_.get_io_context() ),
			_server_handler( server_handler_ ),
			_listen_port( 0 ),
			_is_shutdown( false )
	{
	}

	template< typename handler_type, typename session_type >
	acceptor< handler_type, session_type >::~acceptor()
	{
	}

	template< typename handler_type, typename session_type >
	bool	acceptor< handler_type, session_type >::listen_start( uint16_t listen_port_ )
	{
		using	tcp	=	boost::asio::ip::tcp;

		tcp::endpoint	ep( tcp::endpoint( tcp::v4(), listen_port_ ) );
		_acceptor.open( ep.protocol() );
		_acceptor.set_option( tcp::acceptor::reuse_address( true ) );

		try
		{
			_acceptor.bind( ep );
		}
		catch ( boost::system::system_error& error_ )
		{
			boost::system::error_code	ec;
			_acceptor.close( ec );

			switch ( error_.code().value() )
			{
				case	boost::system::errc::address_in_use:
					_error_log_( 
						boost::format( "( %1% )" ) 
							%	__FILE_LINE__ );
					return false;

				default:
					_error_log_( 
						boost::format( "%1% ( %2% )" ) 
							%	error_.code().value() 
							%	__FILE_LINE__ );
					return false;
			}
		}

		_acceptor.listen(); 
		_server_handler.on_start();

		try
		{
			session_ptr_type	new_session( boost::make_shared< session_type >( _server_handler ) );
			_acceptor.async_accept( 
				new_session->tcp_socket().lowest_layer(),
				boost::bind(	
					&acceptor::accept,
					this,
					new_session,
					boost::asio::placeholders::error ) );

			_listen_port	=	listen_port_;
			return true;
		}
		catch ( const std::exception& e_ )
		{
			_fatal_log_( 
				boost::format( "%1% ( %2% )" ) 
					%	e_.what() 
					%	__FILE_LINE__ );
		}

		return false;
	}

	template< typename handler_type, typename session_type >
	void	acceptor< handler_type, session_type >::stop()
	{
		_is_shutdown	=	true;
		_server_handler.on_stop();

		if ( true == _acceptor.is_open() )
		{
			boost::system::error_code	err;
			_acceptor.close( err );

			if ( boost::system::errc::success != err )
			{
				_error_log_( 
					boost::format( "[stop] %1% %2% (%3%)" ) 
						%	err.value() 
						%	err.message() 
						%	__FILE_LINE__ );
			}
		}
	}

	template< typename handler_type, typename session_type >
	uint16_t		acceptor< handler_type, session_type >::get_listen_port()	const
	{
		return	_listen_port;	
	}

	template< typename handler_type, typename session_type >
	std::string		acceptor< handler_type, session_type >::get_host_ip()	const
	{
		using resolver	=	boost::asio::ip::tcp::resolver;

		resolver			rv( get_io_context() );
		resolver::iterator	pos( rv.resolve( resolver::query( boost::asio::ip::host_name(), "" ) ) );
		resolver::iterator	end;

		while ( pos != end )
		{
			boost::asio::ip::tcp::endpoint	cur = *pos++;
			if ( cur.address().is_v4() )
				return cur.address().to_string();
		}

		return "";
	}

	template< typename handler_type, typename session_type >
	boost::asio::io_context&	acceptor< handler_type, session_type >::get_io_context()
	{	
		return	_server_handler.get_io_context();	
	}

	template< typename handler_type, typename session_type >
	void	acceptor< handler_type, session_type >::accept( 
		const session_ptr_type& session_, 
		const boost::system::error_code& error_ )
	{
		if ( boost::system::errc::success == error_ )
		{
			//	options
			{
				session_->tcp_socket().non_blocking( true );

				using namespace boost::asio;

				session_->tcp_socket().set_option( socket_base::send_buffer_size( packet::_packet_size_ ) );
				session_->tcp_socket().set_option( socket_base::receive_buffer_size( packet::_packet_size_ ) );
				session_->tcp_socket().set_option( ip::tcp::no_delay( true ) );
				session_->tcp_socket().set_option( socket_base::keep_alive( true ) );

#ifdef _DEBUG
				session_->tcp_socket().set_option( socket_base::debug( true ) );
#endif
			}

			//	accept 성공건은 IO에 넘기고
			boost::asio::dispatch( 
				get_io_context(), 
				boost::bind( 
					&session_type::start, 
					session_ ) );
		}
		//	ERROR_OPERATION_ABORTED : The I/O operation has been aborted because of either a thread exit or an application request ( boost::asio::error::operation_aborted )
		//	boost::asio::error::bad_descriptor ( WSAEBADF ) : The file handle supplied is not valid
		else if ( boost::asio::error::operation_aborted != error_.value() && boost::asio::error::bad_descriptor != error_.value() )
		{
			_error_log_(	
				boost::format(	"[handle_accept] user : %1%:%2% - %3% %4% (%5%)" )
					%	session_->get_remote_ip()
					%	session_->get_remote_port()
					%	error_.value()
					%	error_.message()
					%	__FILE_LINE__ );
		}

		if ( true == _is_shutdown )		return;

		try
		{
			//	신규 accept 대기.
			session_ptr_type	new_session( boost::make_shared< session_type >( _server_handler ) );
			_acceptor.async_accept( 
				new_session->tcp_socket().lowest_layer(),
				boost::bind(	
					&acceptor::accept,
					this,
					new_session,
					boost::asio::placeholders::error ) );
			return;
		}
		catch ( const std::exception& e_ )
		{
			_fatal_log_( boost::format( "%1% ( %2% )" ) % e_.what() % __FILE_LINE__ );
		}
	}
}