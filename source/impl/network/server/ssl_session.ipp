/*!
* \class ssl_session.ipp
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
	template < typename session_type, typename handler_type >
	ssl_session< session_type, handler_type >::ssl_session( 
		boost::asio::ssl::context& context_, 
		handler_type& handler_ )
		:	_socket( handler_.get_io_context(), context_ ), 
			_handler( handler_ ),
			primitive_session( handler_.get_io_context() )
	{
	}

	template < typename session_type, typename handler_type >
	ssl_session< session_type, handler_type >::~ssl_session()
	{
	}

	template < typename session_type, typename handler_type >
	void	ssl_session< session_type, handler_type >::start()
	{
		set_ip( tcp_socket().remote_endpoint().address().to_string() );
		set_port( tcp_socket().remote_endpoint().port() );

		ssl_stream_socket().async_handshake(	
			boost::asio::ssl::stream_base::server,
			boost::asio::bind_executor(	
				_strand,
				boost::bind(	
					&ssl_session::handshake,
					this->shared_from_this(),
					boost::asio::placeholders::error ) ) );
	}

	template < typename session_type, typename handler_type >
	void	ssl_session< session_type, handler_type >::on_close()
	{
		if ( false == is_connected() )	return;

		set_closed();

		dispatch(	
			[ this, self = this->shared_from_this() ]() -> void
			{
				boost::system::error_code ec;

				tcp_socket().cancel( ec );
				if ( boost::system::errc::success != ec )
				{
					_warning_log_( 
						boost::format( "%1% %2% ( %3% )" ) 
							%	ec.value() 
							%	ec.message() 
							%	__FILE_LINE__ );
				}

				ssl_stream_socket().async_shutdown( 
					boost::asio::bind_executor(	
						_strand,
						[ this, self ]( boost::system::error_code ec_ ) -> void
						{
							switch ( ec_.value() )
							{
							case	boost::system::errc::success:	
								break;

							case	336462231:
							case	boost::asio::error::connection_reset:
							case	boost::asio::error::connection_aborted:
							case	boost::asio::error::operation_aborted:
							case	boost::asio::ssl::error::stream_truncated:
								_trace_log_( 
									boost::format( "%1% %2% ( %3% )" ) 
										%	ec_.value() 
										%	ec_.message() 
										%	__FILE_LINE__ );
								break;

							default:
								_warning_log_( 
									boost::format( "%1% %2% ( %3% )" ) 
										%	ec_.value() 
										%	ec_.message() 
										%	__FILE_LINE__ );
							}

							tcp_socket().close( ec_ );
							if ( boost::system::errc::success != ec_ )
							{
								_warning_log_( 
									boost::format( "%1% %2% ( %3% )" ) 
										%	ec_.value() 
										%	ec_.message() 
										%	__FILE_LINE__ );
							}

							post( 
								[ this, self ]() -> void
								{
									_handler.on_user_leave( self );
								} );							
						} ) );
			} );
	}

	template < typename session_type, typename handler_type >
	void	ssl_session< session_type, handler_type >::handshake( const boost::system::error_code& ec_ )
	{
		if ( boost::system::errc::success != ec_ )
		{
			switch ( ec_.value() )
			{
			case	boost::asio::ssl::error::stream_truncated:
			case	boost::asio::error::eof:
			case	boost::asio::error::connection_reset:
			case	boost::asio::error::connection_aborted:
			case	boost::asio::error::operation_aborted:
			case	335544539:
				_trace_log_( 
					boost::format( "%1%:%2% %3% %4% (%5%)" ) 
						%	get_remote_ip() 
						%	get_remote_port() 
						%	ec_.value() 
						%	ec_.message() 
						%	__FILE_LINE__ );
				break;

			case	336130329:
				_warning_log_( 
					boost::format( "%1%:%2% %3% %4% (%5%)" ) 
						%	get_remote_ip() 
						%	get_remote_port() 
						%	ec_.value() 
						%	ec_.message() 
						%	__FILE_LINE__ );
				break;

			default:
				_error_log_( 
					boost::format( "%1%:%2% %3% %4% (%5%)" ) 
						%	get_remote_ip() 
						%	get_remote_port() 
						%	ec_.value() 
						%	ec_.message() 
						%	__FILE_LINE__ );
			}
					
			set_connected();
			on_close();
			return;
		}

		set_connected();

		auto	in( alloc_inbound() );
		if ( !in )
		{
			_fatal_log_( 
				boost::format( "%1%:%2% ( %3% )" ) 
					%	get_remote_ip() 
					%	get_remote_port() 
					%	__FILE_LINE__ );

			on_close();
			return;
		}

		_inbound	=	boost::move( *in );

		boost::asio::async_read(	
			ssl_stream_socket(),
			boost::asio::buffer( 
				_inbound->header_buffer(), 
				packet::_header_size_ ),
			boost::asio::bind_executor(	
				_strand,
				boost::bind(	
					&ssl_session::read_header, 
					this->shared_from_this(),
					boost::asio::placeholders::error,
					boost::asio::placeholders::bytes_transferred ) ) );

		post(
			[ this, self = this->shared_from_this() ]() -> void
			{
				_handler.on_user_enter( self );
			} );		
	}

	template < typename session_type, typename handler_type >
	void	ssl_session< session_type, handler_type >::read_header( 
		const boost::system::error_code& ec_, 
		size_t bytes_transferred_ )
	{
		if ( nullptr == _inbound )
		{
			_fatal_log_( 
				boost::format( "%1%:%2% %3% %4% %5% (%6%)" ) 
					%	get_remote_ip() 
					%	get_remote_port() 
					%	ec_.value() 
					%	ec_.message() 
					%	bytes_transferred_ 
					%	__FILE_LINE__ );

			on_close();
			return;
		}

		if ( boost::system::errc::success != ec_ )
		{
			on_close();

			switch ( ec_.value() )
			{
			case	boost::asio::error::eof:
			case	boost::asio::error::connection_reset:
			case	boost::asio::error::connection_aborted:
			case	boost::asio::error::operation_aborted:
			case	335544539:
				_trace_log_( 
					boost::format( "%1%:%2% %3% %4% %5% (%6%)" ) 
						%	get_remote_ip() 
						%	get_remote_port() 
						%	ec_.value() 
						%	ec_.message() 
						%	bytes_transferred_ 
						%	__FILE_LINE__ );
				break;

			case	336130329:
				_warning_log_( 
					boost::format( "%1%:%2% %3% %4% %5% (%6%)" ) 
						%	get_remote_ip() 
						%	get_remote_port() 
						%	ec_.value() 
						%	ec_.message() 
						%	bytes_transferred_ 
						%	__FILE_LINE__ );
				break;

			default:
				_error_log_( 
					boost::format( "%1%:%2% %3% %4% %5% (%6%)" ) 
						%	get_remote_ip() 
						%	get_remote_port() 
						%	ec_.value() 
						%	ec_.message() 
						%	bytes_transferred_ 
						%	__FILE_LINE__ );
			}

			return;
		}

		if ( 0 == bytes_transferred_ )	
		{
			if ( false == is_connected() )	return;

			_inbound->init();

			boost::asio::async_read(	
				ssl_stream_socket(),
				boost::asio::buffer( 
					_inbound->header_buffer(), 
					packet::_header_size_ ),
				boost::asio::bind_executor(	
					_strand,
					boost::bind(	
						&ssl_session::read_header, 
						this->shared_from_this(),
						boost::asio::placeholders::error,
						boost::asio::placeholders::bytes_transferred ) ) );
			return;
		}

		if ( packet::_header_size_ != bytes_transferred_ )
		{
			_error_log_( 
				boost::format( "%1% ( %2% )" ) 
					%	bytes_transferred_ 
					%	__FILE_LINE__ );

			on_close();
			return;
		}

		set_keep_alive_flag();

		_inbound->decide();

		if ( 0 < _inbound->body_size() )
		{
			if ( false == is_connected() )	return;

			boost::asio::async_read(	
				ssl_stream_socket(),
				boost::asio::buffer( 
					_inbound->raw_buffer(),
					_inbound->body_size() ),
				boost::asio::bind_executor(	
					_strand,
					boost::bind(	
						&ssl_session::read_body, 
						this->shared_from_this(),
						boost::asio::placeholders::error,
						boost::asio::placeholders::bytes_transferred ) ) );
			return;
		}

		util::scope_exit_call	exit(
			[ &, in( boost::move( _inbound ) ) ]() -> void
			{
				route( in );
			} );

		auto	in( alloc_inbound() );
		if ( !in )
		{
			_fatal_log_( 
				boost::format( "%1%:%2% ( %3% )" ) 
					%	get_remote_ip() 
					%	get_remote_port() 
					%	__FILE_LINE__ );

			on_close();
			return;
		}

		_inbound	=	boost::move( *in );

		if ( false == is_connected() )	return;

		boost::asio::async_read(	
			ssl_stream_socket(),
			boost::asio::buffer( 
				_inbound->header_buffer(), 
				packet::_header_size_ ),
			boost::asio::bind_executor(
				_strand,
				boost::bind(	
					&ssl_session::read_header,
					this->shared_from_this(),
					boost::asio::placeholders::error,
					boost::asio::placeholders::bytes_transferred ) ) );
	}

	template < typename session_type, typename handler_type >
	void	ssl_session< session_type, handler_type >::read_body( 
		const boost::system::error_code& ec_, 
		size_t bytes_transferred_ )
	{
		if ( nullptr == _inbound )
		{
			_fatal_log_( 
				boost::format( "%1%:%2% %3% %4% %5% (%6%)" ) 
					%	get_remote_ip() 
					%	get_remote_port() 
					%	ec_.value() 
					%	ec_.message() 
					%	bytes_transferred_ 
					%	__FILE_LINE__ );

			on_close();
			return;
		}

		if ( boost::system::errc::success != ec_ )
		{
			on_close();

			switch ( ec_.value() )
			{
			case	boost::asio::error::eof:
			case	boost::asio::error::connection_reset:
			case	boost::asio::error::connection_aborted:
			case	boost::asio::error::operation_aborted:
			case	335544539:
				_trace_log_( 
					boost::format( "%1%:%2% %3% %4% %5% %6% %7% %8% %9% %10% %11% (%12%)" )
						%	get_remote_ip()
						%	get_remote_port()
						%	ec_.value()
						%	ec_.message()
						%	bytes_transferred_
						%	_inbound->segment()
						%	_inbound->offset()
						%	_inbound->error()
						%	_inbound->body_size()
						%	_inbound->client_session_key()
						%	_inbound->server_key()
						%	__FILE_LINE__ );
				break;

			case	336130329:
				_warning_log_( 
					boost::format( "%1%:%2% %3% %4% %5% %6% %7% %8% %9% %10% %11% (%12%)" )
						%	get_remote_ip()
						%	get_remote_port()
						%	ec_.value()
						%	ec_.message()
						%	bytes_transferred_
						%	_inbound->segment()
						%	_inbound->offset()
						%	_inbound->error()
						%	_inbound->body_size()
						%	_inbound->client_session_key()
						%	_inbound->server_key()
						%	__FILE_LINE__ );
				break;

			default:
				_error_log_( 
					boost::format( "%1%:%2% %3% %4% %5% %6% %7% %8% %9% %10% %11% (%12%)" )
						%	get_remote_ip()
						%	get_remote_port()
						%	ec_.value()
						%	ec_.message()
						%	bytes_transferred_
						%	_inbound->segment()
						%	_inbound->offset()
						%	_inbound->error()
						%	_inbound->body_size()
						%	_inbound->client_session_key()
						%	_inbound->server_key()
						%	__FILE_LINE__ );
			}
					
			return;
		}

		if ( bytes_transferred_ != _inbound->body_size() )
		{
			_error_log_( 
				boost::format( "%1% %2% ( %3% )" ) 
					%	bytes_transferred_ 
					%	_inbound->body_size() 
					%	__FILE_LINE__ );

			on_close();
			return;
		}

		util::scope_exit_call	exit(
			[ &, in( boost::move( _inbound ) ) ]() -> void
			{
				route( in );
			} );		

		auto	in( alloc_inbound() );
		if ( !in )
		{
			_fatal_log_( 
				boost::format( "%1%:%2% ( %3% )" ) 
					%	get_remote_ip() 
					%	get_remote_port() 
					%	__FILE_LINE__ );

			on_close();
			return;
		}

		_inbound	=	boost::move( *in );

		if ( false == is_connected() )	return;

		boost::asio::async_read(	
			ssl_stream_socket(),
			boost::asio::buffer( 
				_inbound->header_buffer(), 
				packet::_header_size_ ),
			boost::asio::bind_executor(	
				_strand,
				boost::bind(	
					&ssl_session::read_header,
					this->shared_from_this(),
					boost::asio::placeholders::error,
					boost::asio::placeholders::bytes_transferred ) ) );
	}

	template < typename session_type, typename handler_type >
	void	ssl_session< session_type, handler_type >::route( const packet::inbound_ptr_type& in_ )
	{
		if ( nullptr == in_ )
		{
			_fatal_log_( 
				boost::format( "%1%:%2% ( %3% )" ) 
					%	get_remote_ip() 
					%	get_remote_port() 
					%	__FILE_LINE__ );

			on_close();
			return;
		}

		_inbound_q.emplace_back( in_ );
		if ( 1 < _inbound_q.size() )	return;

		dispatch( 
			boost::bind( 
				&ssl_session::route_poll, 
				this->shared_from_this() ) );
	}

	template < typename session_type, typename handler_type >
	void	ssl_session< session_type, handler_type >::route_poll()
	{
		if ( true ==  _inbound_q.empty() )
		{
			_fatal_log_( 
				boost::format( "%1% %2% ( %3% )" ) 
					%	get_remote_ip() 
					%	get_remote_port() 
					%	__FILE_LINE__ );
			return;
		}

		packet::inbound_ptr_type	in( _inbound_q.front() );
		_inbound_q.pop_front();

		bool	is_success	=	false;
		switch ( in->encode_type() )
		{
		case	packet::e_encode_type::_bodyless_:	is_success	=	_handler.on_bodyless_route( this->shared_from_this(), in );	break;
		case	packet::e_encode_type::_flexless_:	is_success	=	_handler.on_flexless_route( this->shared_from_this(), in );	break;
		case	packet::e_encode_type::_flexible_:	is_success	=	_handler.on_flexible_route( this->shared_from_this(), in );	break;
		}

		if ( false == is_success )
		{
			_error_log_(	
				boost::format(	"%1%:%2% %3% %4% %5% %6% %7% %8% (%9%)" )
					%	get_remote_ip()
					%	get_remote_port()
					%	in->segment()
					%	in->offset()
					%	in->error()
					%	in->body_size()
					%	in->client_session_key()
					%	in->server_key()
					%	__FILE_LINE__ );
			on_close();
		}

		dealloc_inbound( in );

		if ( true == _inbound_q.empty() )	return;

		dispatch( 
			boost::bind( 
				&ssl_session::route_poll, 
				this->shared_from_this() ) );
	}

	template < typename session_type, typename handler_type >
	void	ssl_session< session_type, handler_type >::write( 
		const boost::system::error_code& ec_, 
		size_t bytes_transferred_ )
	{
		if ( boost::system::errc::success != ec_ )
			on_close();

		if ( true == _outbound_q.empty() )
		{
			_fatal_log_( 
				boost::format( "%1%:%2% %3% %4% %5% (%6%)" ) 
					%	get_remote_ip() 
					%	get_remote_port() 
					%	ec_.value() 
					%	ec_.message() 
					%	bytes_transferred_ 
					%	__FILE_LINE__ );

			on_close();
			return;
		}

		packet::outbound_ptr_type	pk( _outbound_q.front() );
		_outbound_q.pop_front();
		util::scope_exit_call		exit( [ & ]() -> void { dealloc_outbound( pk ); } );

		if ( boost::system::errc::success == ec_ )
		{
			if ( true == _outbound_q.empty() )	return;
			if ( false == is_connected() )		return;

			boost::asio::async_write(	
				ssl_stream_socket(),
				boost::asio::buffer( 
					_outbound_q.front()->get_buffer(), 
					_outbound_q.front()->get_buf_use_size() ),
				boost::asio::bind_executor(	
					_strand,
					boost::bind(	
						&ssl_session::write,
						this->shared_from_this(),
						boost::asio::placeholders::error,
						boost::asio::placeholders::bytes_transferred ) ) );
			return;
		}

		switch ( ec_.value() )
		{
			case	boost::asio::error::eof:
			case	boost::asio::error::connection_reset:
			case	boost::asio::error::connection_aborted:
			case	boost::asio::error::operation_aborted:
				_trace_log_(	
					boost::format(	"%1%:%2% - %3% %4% %5% %6% %7% %8% %9% %10% %11% (%12%)" )
						%	get_remote_ip()
						%	get_remote_port()
						%	ec_.value()
						%	ec_.message()
						%	bytes_transferred_
						%	pk->segment()
						%	pk->offset()
						%	pk->error()
						%	pk->body_size()
						%	pk->client_session_key()
						%	pk->server_key()
						%	__FILE_LINE__ );
				break;

			default:
				_error_log_(	
					boost::format(	"%1%:%2% - %3% %4% %5% %6% %7% %8% %9% %10% %11% (%12%)" )
						%	get_remote_ip()
						%	get_remote_port()
						%	ec_.value()
						%	ec_.message()
						%	bytes_transferred_
						%	pk->segment()
						%	pk->offset()
						%	pk->error()
						%	pk->body_size()
						%	pk->client_session_key()
						%	pk->server_key()
						%	__FILE_LINE__ );
		}
	}

	template < typename session_type, typename handler_type >
	const std::string&	ssl_session< session_type, handler_type >::on_get_remote_ip()	const
	{
		return	get_remote_ip();
	}

	template < typename session_type, typename handler_type >
	uint16_t			ssl_session< session_type, handler_type >::on_get_remote_port()	const
	{
		return	get_remote_port();
	}

	template < typename session_type, typename handler_type >
	bool	ssl_session< session_type, handler_type >::on_is_connected()		const
	{
		return	is_connected();
	}

	template < typename session_type, typename handler_type >
	bool	ssl_session< session_type, handler_type >::on_is_keep_alive_flag()	const
	{
		return	is_keep_alive_flag();
	}

	template < typename session_type, typename handler_type >
	void	ssl_session< session_type, handler_type >::on_init_keep_alive_flag()
	{
		init_keep_alive_flag();
	}

	template < typename session_type, typename handler_type >
	void	ssl_session< session_type, handler_type >::on_set_keep_alive_flag()
	{
		set_keep_alive_flag();
	}

	template < typename session_type, typename handler_type >
	boost::optional< packet::outbound_ptr_type >
		ssl_session< session_type, handler_type >::on_alloc_outbound()
	{
		return	alloc_outbound();
	}

	template < typename session_type, typename handler_type >
	bool	ssl_session< session_type, handler_type >::on_send(const packet::outbound_ptr_type& out_ )
	{
		if ( nullptr == out_ )
		{
			_fatal_log_( boost::format( "( %1% )" ) % __FILE_LINE__ );
			return false;
		}

		if ( false == out_->is_encode() )
		{
			_error_log_( boost::format( "( %1% )" ) % __FILE_LINE__ );
			return false;
		}

		if ( false == is_connected() )	return	false;

		dispatch(
			[	this, 
				self = this->shared_from_this(), 
				out_ ]() -> void
			{
				_outbound_q.emplace_back( out_ );
				if ( 1 < _outbound_q.size() )	return;

				if ( false == is_connected() )	return;

				boost::asio::async_write(	
					ssl_stream_socket(),
					boost::asio::buffer( 
						_outbound_q.front()->get_buffer(), 
						_outbound_q.front()->get_buf_use_size() ),
					boost::asio::bind_executor(	
						_strand,
						boost::bind(	
							&ssl_session::write,
							this->shared_from_this(),
							boost::asio::placeholders::error,
							boost::asio::placeholders::bytes_transferred ) ) );
			} );

		return	true;
	}

	template < typename session_type, typename handler_type >
	void	ssl_session< session_type, handler_type >::on_post( const void_func_type& f_ )
	{
		post( f_ );
	}

	template < typename session_type, typename handler_type >
	void	ssl_session< session_type, handler_type >::on_dispatch( const void_func_type& f_ )
	{
		dispatch( f_ );
	}

	template < typename session_type, typename handler_type >
	e_protocol_type	ssl_session< session_type, handler_type >::on_protocol_type()	const
	{
		return	e_protocol_type::_ssl_;
	}
}