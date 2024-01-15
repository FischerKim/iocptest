#include <pch.h>

namespace	impl::network::server
{
	bool	interface_session::send(
		uint16_t packet_id_,
		const void* body_,
		size_t size_ )
	{
		if ( false == on_is_connected() )	return	false;

		packet::outbound_ptr_type	out;

		{
			auto	res( on_alloc_outbound() );
			if ( !res )
			{
				_error_log_( 
					boost::format( "%1% %2% ( %3% )" ) 
						%	packet_id_ 
						%	size_ 
						%	__FILE_LINE__ );

				return	false;
			}

			out	=	*res;
		}

		out->encode( packet_id_, body_, size_ );

		return	on_send( out );
	}

	bool	interface_session::fast_send(
		uint16_t packet_id_,
		const void* body_,
		size_t size_ )
	{
		if ( false == on_is_connected() )	return	false;

		packet::outbound_ptr_type	out( new packet::outbound() );
		if ( nullptr == out )
		{
			_error_log_(
				boost::format( "%1% %2% ( %3% )" )
			 		%	packet_id_
					%	size_
					%	__FILE_LINE__ );
			return	false;
		}

		out->encode( packet_id_, body_, size_ );

		return	on_fast_send( out );
	}

	primitive_session::primitive_session( 
		boost::asio::io_context& io_, 
		size_t default_pool_size_ /*= _default_pool_size_*/ )
		:	_strand( boost::asio::make_strand( io_ ) ),
			_socket_port( 0 ),
			_keep_alive_flag( false ),
			_is_connected( false )
	{
		default_pool_size_	=	
			( _default_pool_size_ > default_pool_size_ || 
			  _max_pool_size_ < default_pool_size_ ) ? 
			_default_pool_size_ : 
			default_pool_size_;

		for ( size_t i = 0 ; i < default_pool_size_ ; ++i )
		{
			try
			{
				packet::outbound_ptr_type	out( new packet::outbound() );
				if ( nullptr == out )
				{
					_fatal_log_( boost::format( "( %1% )" ) % __FILE_LINE__ );
					continue;
				}

				out->init();
				_outbound_pool.emplace_back( out );

				packet::inbound_ptr_type	in( new packet::inbound() );
				if ( nullptr == in )
				{
					_fatal_log_( boost::format( "( %1% )" ) % __FILE_LINE__ );
					continue;
				}

				in->init();
				_inbound_pool.emplace_back( in );
			}
			catch ( const std::exception& e_ )
			{
				_fatal_log_( boost::format( "%1% ( %2% )" ) % e_.what() % __FILE_LINE__ );
			}
		}
	}

	primitive_session::~primitive_session()
	{
	}

	boost::optional< packet::outbound_ptr_type >
		primitive_session::alloc_outbound()
	{
		packet::outbound_ptr_type	p;

		{
			_unique_lock_( _outbound_pool );

			if ( false == _outbound_pool.empty() )
			{
				p	=	_outbound_pool.front();
				_outbound_pool.pop_front();
				return	{ p };
			}
		}

		try
		{
			p.reset( new packet::outbound() );
			if ( nullptr == p )
			{
				_fatal_log_( boost::format( "( %1% )" ) % __FILE_LINE__ );
				return	{};
			}

			p->init();
			return	{ p };
		}
		catch ( const std::exception& e_ )
		{
			_fatal_log_( boost::format( "%1% ( %2% )" ) % e_.what() % __FILE_LINE__ );
		}

		return	{};
	}

	void	primitive_session::dealloc_outbound( const packet::outbound_ptr_type& p_ )
	{
		if ( nullptr == p_ )
		{
			_fatal_log_( boost::format( "( %1% )" ) % __FILE_LINE__ );
			return;
		}

		p_->init();

		_unique_lock_( _outbound_pool );

		if ( _default_pool_size_ <= _outbound_pool.size() )	return;

		_outbound_pool.emplace_back( p_ );
	}

	boost::optional< packet::inbound_ptr_type >
		primitive_session::alloc_inbound()
	{
		packet::inbound_ptr_type	p;

		if ( false == _inbound_pool.empty() )
		{
			p	=	_inbound_pool.front();
			_inbound_pool.pop_front();
			return	{ p };
		}

		try
		{
			p.reset( new packet::inbound() );
			if ( nullptr == p )
			{
				_fatal_log_( boost::format( "( %1% )" ) % __FILE_LINE__ );
				return	{};
			}

			p->init();
			return	{ p };
		}
		catch ( const std::exception& e_ )
		{
			_fatal_log_( boost::format( "%1% ( %2% )" ) % e_.what() % __FILE_LINE__ );
		}

		return	{};
	}

	void	primitive_session::dealloc_inbound( const packet::inbound_ptr_type& p_ )
	{
		if ( nullptr == p_ )
		{
			_fatal_log_( boost::format( "( %1% )" ) % __FILE_LINE__ );
			return;
		}

		if ( _max_pool_size_ <= _inbound_pool.size() )	return;

		p_->init();
		_inbound_pool.emplace_back( p_ );
	}

	void	primitive_session::post( const void_func_type& f_ )
	{
		if ( true == f_.empty() )	return;

		boost::asio::post( boost::asio::bind_executor( _strand, f_ ) );
	}

	void	primitive_session::dispatch( const void_func_type& f_ )
	{
		if ( true == f_.empty() )	return;

		boost::asio::dispatch( boost::asio::bind_executor( _strand, f_ ) );
	}
}