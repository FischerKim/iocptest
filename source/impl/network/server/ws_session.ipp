/*!
* \class ws_session.ipp
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
	ws_session< session_type, handler_type >::ws_session( handler_type& handler_ )
		:	_socket( handler_.get_io_context() ), 
			_handler( handler_ ),
			primitive_session( handler_.get_io_context() )
	{
	}

	template < typename session_type, typename handler_type >
	ws_session< session_type, handler_type >::~ws_session()
	{
	}

	template < typename session_type, typename handler_type >
	void	ws_session< session_type, handler_type >::start()
	{
		set_ip( tcp_socket().remote_endpoint().address().to_string() );
		set_port( tcp_socket().remote_endpoint().port() );

		if ( 0 < _inbound_ws_stream.size() )
			_inbound_ws_stream.consume( _inbound_ws_stream.size() );

		boost::asio::async_read_until(	
			tcp_socket(),
			_inbound_ws_stream,
			"\r\n\r\n",
			boost::asio::bind_executor(	
				_strand,
				boost::bind(	
					&ws_session::websocket_handshake_read,
					this->shared_from_this(),
					boost::asio::placeholders::error,
					boost::asio::placeholders::bytes_transferred ) ) );
	}

	template < typename session_type, typename handler_type >
	void	ws_session< session_type, handler_type >::on_close()
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

				tcp_socket().shutdown( boost::asio::socket_base::shutdown_both, ec );
				switch ( ec.value() )
				{
				case	boost::system::errc::success:
					break;

				case	boost::asio::error::connection_reset:
				case	boost::asio::error::connection_aborted:
				case	boost::asio::error::operation_aborted:
					_trace_log_( 
						boost::format( "%1% %2% ( %3% )" ) 
							%	ec.value() 
							%	ec.message() 
							%	__FILE_LINE__ );
					break;

				default:
					_warning_log_( 
						boost::format( "%1% %2% ( %3% )" ) 
							%	ec.value() 
							%	ec.message() 
							%	__FILE_LINE__ );
				}

				tcp_socket().close( ec );
				if ( boost::system::errc::success != ec )
				{
					_warning_log_( 
						boost::format( "%1% %2% ( %3% )" ) 
							%	ec.value() 
							%	ec.message() 
							%	__FILE_LINE__ );
				}

				post( 
					[ this, self ]() -> void
					{
						_handler.on_user_leave( self );
					} );
			} );
	}

	template < typename session_type, typename handler_type >
	void	ws_session< session_type, handler_type >::websocket_handshake_read(	
		const boost::system::error_code& ec_, 
		size_t bytes_transferred_ )
	{
		if ( boost::system::errc::success != ec_ )
		{
			switch ( ec_.value() )
			{
			case	boost::asio::error::eof:
			case	boost::asio::error::connection_reset:
			case	boost::asio::error::connection_aborted:
			case	boost::asio::error::operation_aborted:
				_trace_log_( 
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

			set_connected();
			on_close();
			return;
		}

		std::vector< std::string >	v;
		std::istream	is( &_inbound_ws_stream );
		std::string		tmp;
		while ( std::getline( is, tmp ) )
			v.emplace_back( tmp );

		boost::optional< websocket::header >	h( websocket::server::parse_handshake( v ) );
		if ( !h )
		{
			_error_log_( 
				boost::format( "%1%:%2% ( %3% )" ) 
					%	get_remote_ip() 
					%	get_remote_port() 
					%	__FILE_LINE__ );

			set_connected();
			on_close();
			return;
		}

		_ws_header	=	boost::move( *h );

		auto	res( alloc_outbound() );
		if ( !res )
		{
			_error_log_( 
				boost::format( "%1%:%2% ( %3% )" ) 
					%	get_remote_ip() 
					%	get_remote_port() 
					%	__FILE_LINE__ );

			set_connected();
			on_close();
			return;
		}

		auto	ob( boost::move( *res ) );
		if ( false == websocket::server::generate_handshake( _ws_header, ob->get_tmp() ) )
		{
			_error_log_( 
				boost::format( "%1%:%2% ( %3% )" ) 
					%	get_remote_ip() 
					%	get_remote_port() 
					%	__FILE_LINE__ );

			set_connected();
			on_close();
			return;
		}

		boost::asio::async_write(	
			tcp_socket(),
			boost::asio::buffer( ob->get_tmp() ),
			boost::asio::bind_executor(	
				_strand,
				boost::bind(	
					&ws_session::websocket_handshake_write,
					this->shared_from_this(),
					ob,
					boost::asio::placeholders::error,
					boost::asio::placeholders::bytes_transferred ) ) );
	}

	template < typename session_type, typename handler_type >
	void	ws_session< session_type, handler_type >::websocket_handshake_write( 
		const packet::outbound_ptr_type& out_, 
		const boost::system::error_code& ec_, 
		size_t bytes_transferred_ )
	{
		util::scope_exit_call	exit( [ & ]() -> void { dealloc_outbound( out_ ); } );

		if ( boost::system::errc::success != ec_ )
		{
			switch ( ec_.value() )
			{
			case	boost::asio::error::eof:
			case	boost::asio::error::connection_reset:
			case	boost::asio::error::connection_aborted:
			case	boost::asio::error::operation_aborted:
				_trace_log_( 
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
					
			on_close();
			return;
		}

		if ( 0 < _inbound_ws_stream.size() )
			_inbound_ws_stream.consume( _inbound_ws_stream.size() );

		boost::asio::async_read(	
			tcp_socket(),
			_inbound_ws_stream,
			boost::asio::transfer_exactly( 2 ),
			boost::asio::bind_executor(	
				_strand,
				boost::bind(	
					&ws_session::read_header,
					this->shared_from_this(),
					boost::asio::placeholders::error,
					boost::asio::placeholders::bytes_transferred ) ) );

		set_connected();

		post(
			[ this, self = this->shared_from_this() ]() -> void
			{
				_handler.on_user_enter( self );
			} );
	}

	template < typename session_type, typename handler_type >
	void	ws_session< session_type, handler_type >::read_header( 
		const boost::system::error_code& ec_, 
		size_t bytes_transferred_ )
	{
		if ( boost::system::errc::success != ec_ )
		{
			on_close();

			switch ( ec_.value() )
			{
			case	boost::asio::error::eof:
			case	boost::asio::error::connection_reset:
			case	boost::asio::error::connection_aborted:
			case	boost::asio::error::operation_aborted:
				_trace_log_( 
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

			boost::asio::async_read(	
				tcp_socket(),
				_inbound_ws_stream,
				boost::asio::transfer_exactly( 2 ),
				boost::asio::bind_executor(	
					_strand,
					boost::bind(
						&ws_session::read_header,
						this->shared_from_this(),
						boost::asio::placeholders::error,
						boost::asio::placeholders::bytes_transferred ) ) );
			return;
		}

		set_keep_alive_flag();

		std::istream	is( &_inbound_ws_stream );
		uint8_t			first_bytes[ 2 ]	=	{ 0 };
		is.read( reinterpret_cast< char* >( first_bytes ), 2 );

		uint8_t			fin_rsv_opcode	=	first_bytes[ 0 ];
		if ( 128 > first_bytes[ 1 ] )
		{
			_error_log_( 
				boost::format( "%1%:%2% %3% (%4%)" ) 
					%	get_remote_ip() 
					%	get_remote_port() 
					%	bytes_transferred_ 
					%	__FILE_LINE__ );

			on_close();
			return;
		}

		if ( false == is_connected() )	return;

		size_t	length	=	( first_bytes[ 1 ] & 127 );
		switch ( length )
		{
		case	126:
			boost::asio::async_read(	
				tcp_socket(),
				_inbound_ws_stream,
				boost::asio::transfer_exactly( 2 ),
				boost::asio::bind_executor(	
					_strand,
					boost::bind(	
						&ws_session::read_header_126,
						this->shared_from_this(),
						fin_rsv_opcode,
						boost::asio::placeholders::error,
						boost::asio::placeholders::bytes_transferred ) ) );
			break;

		case	127:
			boost::asio::async_read(	
				tcp_socket(),
				_inbound_ws_stream,
				boost::asio::transfer_exactly( 8 ),
				boost::asio::bind_executor(	
					_strand,
					boost::bind(
						&ws_session::read_header_127,
						this->shared_from_this(),
						fin_rsv_opcode,
						boost::asio::placeholders::error,
						boost::asio::placeholders::bytes_transferred ) ) );
			break;

		default:
			boost::asio::async_read(	
				tcp_socket(),
				_inbound_ws_stream,
				boost::asio::transfer_exactly( 4 + length ),
				boost::asio::bind_executor(	
					_strand,
					boost::bind(	
						&ws_session::read,
						this->shared_from_this(),
						length,
						fin_rsv_opcode,
						boost::asio::placeholders::error,
						boost::asio::placeholders::bytes_transferred ) ) );
		}
	}

	template < typename session_type, typename handler_type >
	void	ws_session< session_type, handler_type >::read_header_126( 
		uint8_t fin_rsv_opcode_, 
		const boost::system::error_code& ec_, 
		size_t bytes_transferred_ )
	{
		if ( boost::system::errc::success != ec_ )
		{
			on_close();

			switch ( ec_.value() )
			{
			case	boost::asio::error::eof:
			case	boost::asio::error::connection_reset:
			case	boost::asio::error::connection_aborted:
			case	boost::asio::error::operation_aborted:
				_trace_log_(	
					boost::format(	"%1%:%2% %3% %4% %5% %6% (%7%)" ) 
						%	get_remote_ip()
						%	get_remote_port() 
						%	ec_.value()
						%	ec_.message()
						%	static_cast< uint16_t >( fin_rsv_opcode_ )	
						%	bytes_transferred_
						%	__FILE_LINE__ );
				break;

			default:
				_error_log_(	
					boost::format(	"%1%:%2% %3% %4% %5% %6% (%7%)" ) 
						%	get_remote_ip()
						%	get_remote_port() 
						%	ec_.value()
						%	ec_.message()
						%	static_cast< uint16_t >( fin_rsv_opcode_ )	
						%	bytes_transferred_
						%	__FILE_LINE__ );
			}

			return;
		}

		std::istream	is( &_inbound_ws_stream );
		uint8_t			length_bytes[ 2 ]	=	{ 0 };
		is.read( reinterpret_cast< char* >( &length_bytes ), 2 );

		size_t	length		=	0;
		int		num_bytes	=	2;
		for ( int i = 0 ; i < num_bytes ; ++i )
			length	+=	length_bytes[ i ] << ( 8 * ( num_bytes - 1 - i ) );

		if ( false == is_connected() )	return;

		boost::asio::async_read(	
			tcp_socket(),
			_inbound_ws_stream,
			boost::asio::transfer_exactly( 4 + length ),
			boost::asio::bind_executor(	
				_strand,
				boost::bind(	
					&ws_session::read,
					this->shared_from_this(),
					length,
					fin_rsv_opcode_,
					boost::asio::placeholders::error,
					boost::asio::placeholders::bytes_transferred ) ) );
	}

	template < typename session_type, typename handler_type >
	void	ws_session< session_type, handler_type >::read_header_127( 
		uint8_t fin_rsv_opcode_, 
		const boost::system::error_code& ec_, 
		size_t bytes_transferred_ )
	{
		if ( boost::system::errc::success != ec_ )
		{
			on_close();

			switch ( ec_.value() )
			{
			case	boost::asio::error::eof:
			case	boost::asio::error::connection_reset:
			case	boost::asio::error::connection_aborted:
			case	boost::asio::error::operation_aborted:
				_trace_log_( 
					boost::format( "%1%:%2% %3% %4% %5% %6% (%7%)" )
						%	get_remote_ip()
						%	get_remote_port()
						%	ec_.value()
						%	ec_.message()
						%	static_cast< uint16_t >( fin_rsv_opcode_ )
						%	bytes_transferred_
						%	__FILE_LINE__ );
				break;

			default:
				_error_log_( 
					boost::format( "%1%:%2% %3% %4% %5% %6% (%7%)" )
						%	get_remote_ip()
						%	get_remote_port()
						%	ec_.value()
						%	ec_.message()
						%	static_cast< uint16_t >( fin_rsv_opcode_ )
						%	bytes_transferred_
						%	__FILE_LINE__ );
			}

			return;
		}

		std::istream	is( &_inbound_ws_stream );
		uint8_t			length_bytes[ 8 ]	=	{ 0 };
		is.read( reinterpret_cast< char* >( &length_bytes ), 8 );

		size_t	length		=	0;
		int		num_bytes	=	8;
		for ( int i = 0 ; i < num_bytes ; ++i )
			length += length_bytes[ i ] << ( 8 * ( num_bytes - 1 - i ) );

		boost::asio::async_read(	
			tcp_socket(),
			_inbound_ws_stream,
			boost::asio::transfer_exactly( 4 + length ),
			boost::asio::bind_executor(	
				_strand,
				boost::bind(	
					&ws_session::read,
					this->shared_from_this(),
					length,
					fin_rsv_opcode_,
					boost::asio::placeholders::error,
					boost::asio::placeholders::bytes_transferred ) ) );
	}

	template < typename session_type, typename handler_type >
	void	ws_session< session_type, handler_type >::read( 
		size_t length_, 
		uint8_t fin_rsv_opcode_, 
		const boost::system::error_code& ec_, 
		size_t bytes_transferred_ )
	{
		//	disconnect from client
		if ( boost::system::errc::success == ec_ && 2 == length_ && 6 == bytes_transferred_ )
		{
			on_close();
			return;
		}

		if ( ( 4 + packet::_header_size_ ) > bytes_transferred_ || 
			 ( 4 + length_ ) != bytes_transferred_ || 
			 packet::_packet_size_ < bytes_transferred_ )
		{
			_error_log_( 
				boost::format( "%1%:%2% %3% %4% %5% %6% %7% ( %8% )" )
					%	get_remote_ip()
					%	get_remote_port()
					%	ec_.value()
					%	ec_.message()
					%	bytes_transferred_
					%	length_
					%	static_cast< uint16_t >( fin_rsv_opcode_ )
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
				_trace_log_( 
					boost::format( "%1%:%2% %3% %4% %5% %6% %7% ( %8% )" )
						%	get_remote_ip()
						%	get_remote_port()
						%	ec_.value()
						%	ec_.message()
						%	bytes_transferred_
						%	length_
						%	static_cast< uint16_t >( fin_rsv_opcode_ )
						%	__FILE_LINE__ );
				break;

			default:
				_error_log_( 
					boost::format( "%1%:%2% %3% %4% %5% %6% %7% ( %8% )" )
						%	get_remote_ip()
						%	get_remote_port()
						%	ec_.value()
						%	ec_.message()
						%	bytes_transferred_
						%	length_
						%	static_cast< uint16_t >( fin_rsv_opcode_ )
						%	__FILE_LINE__ );
			}
					
			return;
		}

		packet::inbound_ptr_type	in;

		{
			auto	res( alloc_inbound() );
			if ( !res )
			{
				_fatal_log_( 
					boost::format( "%1%:%2% ( %3% )" ) 
						%	get_remote_ip() 
						%	get_remote_port() 
						%	__FILE_LINE__ );

				on_close();
				return;
			}

			in	=	*res;
		}

		std::istream	is( &_inbound_ws_stream );

		//	read mask
		uint8_t			mask[ 4 ]	=	{ 0 };
		is.read( reinterpret_cast< char* >( mask ), 4 );

		in->init();

		//	read header
		for ( size_t i = 0 ; i < packet::_header_size_ ; ++i )
			in->header_buffer()[ i ] = ( is.get() ^ mask[ i % 4 ] );

		in->decide();

		//	read body
		if ( length_ > packet::_header_size_ )
		{
			for ( size_t i = 0 ; i < length_ - packet::_header_size_ ; ++i )
				in->raw_buffer()[ i ]	=	( is.get() ^ mask[ ( i + packet::_header_size_ ) % 4 ] );
		}

		if ( 8 == ( fin_rsv_opcode_ & 0x0f ) )
		{
			_error_log_( 
				boost::format( "%1%:%2% %3% %4% %5% ( %6% )" ) 
					%	get_remote_ip() 
					%	get_remote_port() 
					%	bytes_transferred_ 
					%	length_ 
					%	static_cast< uint16_t >( fin_rsv_opcode_ ) 
					%	__FILE_LINE__ );

			on_close();
			return;
		}

		util::scope_exit_call	exit(
			[ & ]() -> void
			{
				( 9 == ( fin_rsv_opcode_ & 0x0f ) ) ? 
					pong( fin_rsv_opcode_ ) : 
					route( in );
			} );

		if ( 0 < _inbound_ws_stream.size() )
			_inbound_ws_stream.consume( _inbound_ws_stream.size() );

		if ( false == is_connected() )	return;

		boost::asio::async_read(	
			tcp_socket(),
			_inbound_ws_stream,
			boost::asio::transfer_exactly( 2 ),
			boost::asio::bind_executor(	
				_strand,
				boost::bind(
					&ws_session::read_header,
					this->shared_from_this(),
					boost::asio::placeholders::error,
					boost::asio::placeholders::bytes_transferred ) ) );
	}

	template < typename session_type, typename handler_type >
	void	ws_session< session_type, handler_type >::pong( uint8_t fin_rsv_opcode_ )
	{
		auto	res( alloc_outbound() );
		if ( !res )
		{
			_error_log_( 
				boost::format(	"%1%:%2% %3% ( %4% )" ) 
					%	get_remote_ip() 
					%	get_remote_port() 
					%	static_cast< uint16_t >( fin_rsv_opcode_ ) 
					%	__FILE_LINE__ );
			return;
		}

		auto	ob( boost::move( *res ) );
		if ( false == ob->ws_encode( fin_rsv_opcode_ + 1 ) )
		{
			_error_log_( 
				boost::format(	"%1%:%2% %3% ( %4% )" ) 
					%	get_remote_ip() 
					%	get_remote_port() 
					%	static_cast< uint16_t >( fin_rsv_opcode_ ) 
					%	__FILE_LINE__ );
			return;
		}

		on_send( ob );
	}

	template < typename session_type, typename handler_type >
	void	ws_session< session_type, handler_type >::route( const packet::inbound_ptr_type& in_ )
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
				&ws_session::route_poll, 
				this->shared_from_this() ) );
	}

	template < typename session_type, typename handler_type >
	void	ws_session< session_type, handler_type >::route_poll()
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
				&ws_session::route_poll, 
				this->shared_from_this() ) );
	}

	template < typename session_type, typename handler_type >
	void	ws_session< session_type, handler_type >::write( 
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
				tcp_socket(),
				boost::asio::buffer( 
					_outbound_q.front()->get_buffer(), 
					_outbound_q.front()->get_buf_use_size() ),
				boost::asio::bind_executor(	
					_strand,
					boost::bind(	
						&ws_session::write,
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
	const std::string&	ws_session< session_type, handler_type >::on_get_remote_ip()		const
	{
		return	get_remote_ip();
	}

	template < typename session_type, typename handler_type >
	uint16_t	ws_session< session_type, handler_type >::on_get_remote_port()		const
	{
		return	get_remote_port();
	}

	template < typename session_type, typename handler_type >
	bool	ws_session< session_type, handler_type >::on_is_connected()			const
	{
		return	is_connected();
	}

	template < typename session_type, typename handler_type >
	bool	ws_session< session_type, handler_type >::on_is_keep_alive_flag()	const
	{
		return	is_keep_alive_flag();
	}

	template < typename session_type, typename handler_type >
	void	ws_session< session_type, handler_type >::on_init_keep_alive_flag()
	{
		init_keep_alive_flag();
	}

	template < typename session_type, typename handler_type >
	void	ws_session< session_type, handler_type >::on_set_keep_alive_flag()
	{
		set_keep_alive_flag();
	}

	template < typename session_type, typename handler_type >
	boost::optional< packet::outbound_ptr_type >
		ws_session< session_type, handler_type >::on_alloc_outbound()
	{
		return	alloc_outbound();
	}

	template < typename session_type, typename handler_type >
	bool	ws_session< session_type, handler_type >::on_send( const packet::outbound_ptr_type& out_ )
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
					tcp_socket(),
					boost::asio::buffer( 
						_outbound_q.front()->get_buffer(), 
						_outbound_q.front()->get_buf_use_size() ),
					boost::asio::bind_executor(	
						_strand,
						boost::bind(	
							&ws_session::write,
							this->shared_from_this(),
							boost::asio::placeholders::error,
							boost::asio::placeholders::bytes_transferred ) ) );
			} );

		return	true;
	}

	template < typename session_type, typename handler_type >
	void			ws_session< session_type, handler_type >::on_post( const void_func_type& f_ )
	{
		post( f_ );
	}

	template < typename session_type, typename handler_type >
	void			ws_session< session_type, handler_type >::on_dispatch( const void_func_type& f_ )
	{
		dispatch( f_ );
	}

	template < typename session_type, typename handler_type >
	e_protocol_type	ws_session< session_type, handler_type >::on_protocol_type()	const
	{
		return	e_protocol_type::_ws_;
	}
}