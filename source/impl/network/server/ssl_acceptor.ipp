/*!
* \class ssl_acceptor.ipp
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
	ssl_acceptor< handler_type, session_type >::ssl_acceptor( 
		context& ssl_context_, 
		handler_type& server_handler_ )
		:	acceptor< handler_type, session_type >( server_handler_ ),
			_context( ssl_context_ )
	{
	}

	template< typename handler_type, typename session_type >
	bool	ssl_acceptor< handler_type, session_type >::listen_start( uint16_t listen_port_ )
	{
		boost::asio::ip::tcp::endpoint	end_point( boost::asio::ip::tcp::endpoint( boost::asio::ip::tcp::v4(), listen_port_ ) );
		this->_acceptor.open( end_point.protocol() );
		this->_acceptor.set_option( boost::asio::ip::tcp::acceptor::reuse_address( true ) );

		try
		{
			this->_acceptor.bind( end_point );
		}
		catch ( boost::system::system_error& error_ )
		{
			boost::system::error_code	ec;
			this->_acceptor.close( ec );

			switch ( error_.code().value() )
			{
				case	boost::system::errc::address_in_use:
					_error_log_( boost::format( "( %1% )" ) % __FILE_LINE__ );
					return false;

				default:
					_error_log_( 
						boost::format( "%1% ( %2% )" ) 
							%	error_.code().value() 
							%	__FILE_LINE__ );
					return false;
			}
		}

		this->_acceptor.listen(); 
		this->_server_handler.on_start();

		try
		{
			session_ptr_type	new_session( boost::make_shared< session_type >( _context.get_context(), this->_server_handler ) );
			this->_acceptor.async_accept(	
				new_session->tcp_socket().lowest_layer(),
				boost::bind(	
					&ssl_acceptor::accept,
					this,
					new_session,
					boost::asio::placeholders::error ) );

			this->_listen_port	=	listen_port_;
			return true;
		}
		catch ( const std::exception& e_ )
		{
			_fatal_log_( boost::format( "%1% ( %2% )" ) % e_.what() % __FILE_LINE__ );
		}

		return false;
	}

	template< typename handler_type, typename session_type >
	void	ssl_acceptor< handler_type, session_type >::accept( const session_ptr_type& session_, const boost::system::error_code& error_ )
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
				this->get_io_context(), 
				boost::bind( 
					&session_type::start, 
					session_ ) );
		}
		//	ERROR_OPERATION_ABORTED : The I/O operation has been aborted because of either a thread exit or an application request ( boost::asio::error::operation_aborted )
		//	boost::asio::error::bad_descriptor ( WSAEBADF ) : The file handle supplied is not valid
		else if ( boost::asio::error::operation_aborted != error_.value() && boost::asio::error::bad_descriptor != error_.value() )
		{
			_error_log_(	
				boost::format(	"[ssl_handle_accept] user : %1%:%2% - %3% %4% (%5%)" )
					%	session_->get_remote_ip()
					%	session_->get_remote_port()
					%	error_.value()
					%	error_.message()
					%	__FILE_LINE__ );
		}

		if ( true == this->_is_shutdown )		return;

		try
		{
			//	신규 accept 대기.
			session_ptr_type	new_session( boost::make_shared< session_type >( _context.get_context(), this->_server_handler ) );
			this->_acceptor.async_accept(	
				new_session->tcp_socket().lowest_layer(),
				boost::bind(	
					&ssl_acceptor::accept,
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