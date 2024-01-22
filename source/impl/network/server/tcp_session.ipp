#pragma once

namespace	impl::network::server
{
	template < typename session_type, typename handler_type >
	tcp_session< session_type, handler_type >::tcp_session( handler_type& handler_ ) 
		:	_socket( handler_.get_io_context() ), 
			_handler( handler_ ),
			primitive_session( handler_.get_io_context() )
	{
	}

	template < typename session_type, typename handler_type >
	tcp_session< session_type, handler_type >::~tcp_session()
	{
	}

	template < typename session_type, typename handler_type >
	void	tcp_session< session_type, handler_type >::start()
	{	
		set_ip( tcp_socket().remote_endpoint().address().to_string() );
		set_port( tcp_socket().remote_endpoint().port() );
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
			tcp_socket(),
			boost::asio::buffer( 
				_inbound->header_buffer(), 
				packet::_header_size_ ),
			boost::asio::bind_executor(	
				_strand,
				boost::bind(	
					&tcp_session::read_header,
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
	void	tcp_session< session_type, handler_type >::on_close()
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
	void	tcp_session< session_type, handler_type >::read_header( 
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

			_inbound->init();

			boost::asio::async_read(
				tcp_socket(),
				boost::asio::buffer( 
					_inbound->header_buffer(), 
					packet::_header_size_ ),
				boost::asio::bind_executor(	
					_strand,
					boost::bind(	
						&tcp_session::read_header, 
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
				tcp_socket(),
				boost::asio::buffer( 
					_inbound->raw_buffer(),
					_inbound->body_size() ),
				boost::asio::bind_executor(	
					_strand,
					boost::bind(	
						&tcp_session::read_body, 
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
			tcp_socket(),
			boost::asio::buffer( 
				_inbound->header_buffer(), 
				packet::_header_size_ ),
			boost::asio::bind_executor(
				_strand,
				boost::bind(	
					&tcp_session::read_header,
					this->shared_from_this(),
					boost::asio::placeholders::error,
					boost::asio::placeholders::bytes_transferred ) ) );
	}

	template < typename session_type, typename handler_type >
	void	tcp_session< session_type, handler_type >::read_body( 
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
				_trace_log_( 
					boost::format( "%1% %2% %3% %4% %5% ( %6% )" )
						%	get_remote_ip()
						%	get_remote_port()
						%	ec_.value()
						%	ec_.message()
						%	bytes_transferred_
						%	__FILE_LINE__ );
				break;

			default:
				_error_log_( 
					boost::format( "%1% %2% %3% %4% %5% ( %6% )" )
						%	get_remote_ip()
						%	get_remote_port()
						%	ec_.value()
						%	ec_.message()
						%	bytes_transferred_
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
				in->body_decode();
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
			tcp_socket(),
			boost::asio::buffer( 
				_inbound->header_buffer(), 
				packet::_header_size_ ),
			boost::asio::bind_executor(	
				_strand,
				boost::bind(	
					&tcp_session::read_header,
					this->shared_from_this(),
					boost::asio::placeholders::error,
					boost::asio::placeholders::bytes_transferred ) ) );
	}

	template < typename session_type, typename handler_type >
	void	tcp_session< session_type, handler_type >::route( const packet::inbound_ptr_type& in_ )
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
				&tcp_session::route_poll, 
				this->shared_from_this() ) );
	}

	template < typename session_type, typename handler_type >
	void	tcp_session< session_type, handler_type >::route_poll()
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

		bool	is_success	=	_handler.on_route( this->shared_from_this(), in );
		if ( false == is_success )
		{
			_error_log_(	
				boost::format( "%1% %2% %3% %4% ( %5% )" )
					%	get_remote_ip()
					%	get_remote_port()
					%	in->header_ptr()->id
					%	in->header_ptr()->crc
					%	__FILE_LINE__ );
			on_close();
		}

		dealloc_inbound( in );

		if ( true == _inbound_q.empty() )	return;

		dispatch( 
			boost::bind( 
				&tcp_session::route_poll, 
				this->shared_from_this() ) );
	}

	template < typename session_type, typename handler_type >
	void	tcp_session< session_type, handler_type >::write( 
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
		util::scope_exit_call		exit( 
			[ & ]() -> void 
			{ 
				_handler.get_io_context().post(
					[ this, self = this->shared_from_this(), pk ]() -> void
					{
						dealloc_outbound( pk );
					} );
			} );

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
						&tcp_session::write,
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
					boost::format( "%1% %2% %3% %4% %5% ( %6% )" )
						%	get_remote_ip()
						%	get_remote_port()
						%	ec_.value()
						%	ec_.message()
						%	bytes_transferred_
						%	__FILE_LINE__ );
				break;

			default:
				_error_log_(	
					boost::format( "%1% %2% %3% %4% %5% ( %6% )" )
						%	get_remote_ip()
						%	get_remote_port()
						%	ec_.value()
						%	ec_.message()
						%	bytes_transferred_
						%	__FILE_LINE__ );
		}
	}

	template < typename session_type, typename handler_type >
	const std::string&	
		tcp_session< session_type, handler_type >::on_get_remote_ip()	const
	{
		return	get_remote_ip();
	}

	template < typename session_type, typename handler_type >
	uint16_t	tcp_session< session_type, handler_type >::on_get_remote_port()	const
	{
		return	get_remote_port();
	}

	template < typename session_type, typename handler_type >
	bool	tcp_session< session_type, handler_type >::on_is_connected()	const
	{
		return	is_connected();
	}

	template < typename session_type, typename handler_type >
	bool	tcp_session< session_type, handler_type >::on_is_keep_alive_flag()	const
	{
		return	is_keep_alive_flag();
	}

	template < typename session_type, typename handler_type >
	void	tcp_session< session_type, handler_type >::on_init_keep_alive_flag()
	{
		init_keep_alive_flag();
	}

	template < typename session_type, typename handler_type >
	void	tcp_session< session_type, handler_type >::on_set_keep_alive_flag()
	{
		set_keep_alive_flag();
	}

	template < typename session_type, typename handler_type >
	boost::optional< packet::outbound_ptr_type >
		tcp_session< session_type, handler_type >::on_alloc_outbound()
	{
		return	alloc_outbound();
	}

	template < typename session_type, typename handler_type >
	bool	tcp_session< session_type, handler_type >::on_send( 
		const packet::outbound_ptr_type& out_ )
	{
		if ( nullptr == out_ )
		{
			_fatal_log_( boost::format( "( %1% )" ) % __FILE_LINE__ );
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
							&tcp_session::write,
							this->shared_from_this(),
							boost::asio::placeholders::error,
							boost::asio::placeholders::bytes_transferred ) ) );
			} );

		return	true;
	}

	template < typename session_type, typename handler_type >
	bool	tcp_session< session_type, handler_type >::on_fast_send( 
		const packet::outbound_ptr_type& out_ )
	{
		if ( nullptr == out_ )
		{
			_error_log_(
				boost::format( "( %1% )" )
					%	__FILE_LINE__ );
			return	false;
		}

		if ( false == is_connected() )	return	false;

		dispatch(
			[	this, 
				self = this->shared_from_this(), 
				out_ ]() -> void
			{
				if ( false == is_connected() )	return;

				boost::asio::async_write(	
					tcp_socket(),
					boost::asio::buffer( 
						out_->get_buffer(),
						out_->get_buf_use_size() ),
						[ this, self, out_ ]( 
							const boost::system::error_code& ec_, 
							size_t bytes_transferred_ ) -> void
						{
							if ( nullptr == out_ )
							{
								_error_log_(
									boost::format( "( %1% )" )
										%	__FILE_LINE__ );
								return;
							}

							if ( boost::system::errc::success == ec_ )	return;
							
							switch ( ec_.value() )
							{
								case	boost::asio::error::eof:
								case	boost::asio::error::connection_reset:
								case	boost::asio::error::connection_aborted:
								case	boost::asio::error::operation_aborted:
									_trace_log_(	
										boost::format( "%1% %2% %3% %4% %5% ( %6% )" )
											%	get_remote_ip()
											%	get_remote_port()
											%	ec_.value()
											%	ec_.message()
											%	bytes_transferred_
											%	__FILE_LINE__ );
									break;

								default:
									_error_log_(	
										boost::format( "%1% %2% %3% %4% %5% ( %6% )" )
											%	get_remote_ip()
											%	get_remote_port()
											%	ec_.value()
											%	ec_.message()
											%	bytes_transferred_
											%	__FILE_LINE__ );
							}
						} );
			} );

		return	true;
	}

	template < typename session_type, typename handler_type >
	void	tcp_session< session_type, handler_type >::on_post( const void_func_type& f_ )
	{
		post( f_ );
	}

	template < typename session_type, typename handler_type >
	void	tcp_session< session_type, handler_type >::on_dispatch( const void_func_type& f_ )
	{
		dispatch( f_ );
	}
}