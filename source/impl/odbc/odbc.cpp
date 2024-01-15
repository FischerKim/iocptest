#include <pch.h>

namespace	impl::odbc
{
	void	command::init()
	{
		_sp.clear();
		_param_v.clear();
		_query.clear();
	}

	bool	command::make_query( const std::string_view& sp_ )
	{
		if ( true == sp_.empty() )		return false;

		_sp.assign( sp_.data(), sp_.size() );

		if ( true == _param_v.empty() )
		{
			_query	=	boost::str( boost::wformat( L"{ CALL %1% }" ) % sp_.data() );
			return true;
		}
		else
		{
			_query	=	boost::str( boost::wformat( L"{ CALL %1% (" ) % sp_.data() );
			size_t	size = _param_v.size();
			for ( size_t i = 0 ; i < size ; ++i )
				_query += L"?,";

			_query.erase( _query.find_last_of( L"," ) );
			_query += L") }";
		}

		return	true;
	}

	void	command::ping_query()
	{
		_query	=	L"{ select 0 }";
	}

	void	command::get_params_string( OUT std::wstring& params_ )	const
	{
		params_.clear();

		if ( true == _param_v.empty() )		return;

		try
		{
			for ( const boost::any& r_ : _param_v )
			{
				if ( r_.type() == typeid( uint8_t ) )			params_	+=	boost::lexical_cast< std::wstring >( static_cast< short >( boost::any_cast< uint8_t >( r_ ) ) );
				else if ( r_.type() == typeid( short ) )		params_	+=	boost::lexical_cast< std::wstring >( boost::any_cast< short >( r_ ) );
				else if ( r_.type() == typeid( int ) )			params_	+=	boost::lexical_cast< std::wstring >( boost::any_cast< int >( r_ ) );
				else if ( r_.type() == typeid( long ) )			params_	+=	boost::lexical_cast< std::wstring >( boost::any_cast< long >( r_ ) );
				else if ( r_.type() == typeid( float ) )		params_	+=	boost::lexical_cast< std::wstring >( boost::any_cast< float >( r_ ) );
				else if ( r_.type() == typeid( double ) )		params_	+=	boost::lexical_cast< std::wstring >( boost::any_cast< double >( r_ ) );
				else if ( r_.type() == typeid( __int64 ) )		params_	+=	boost::lexical_cast< std::wstring >( boost::any_cast< __int64 >( r_ ) );
				else if ( r_.type() == typeid( std::string ) )	params_	+=	boost::str( boost::wformat( L"%1%" ) % boost::any_cast< std::string >( r_ ).c_str() );
				else if ( r_.type() == typeid( std::wstring ) )	params_	+=	boost::any_cast< std::wstring >( r_ );						

				params_	+=	L" ";
			}

			params_.erase( params_.find_last_of( L" " ) );
		}
		catch ( const std::exception& e_ )
		{
			_fatal_log_( boost::format( "%1% ( %2% )" ) % e_.what() % __FILE_LINE__ );
		}
	}

	fetch_buffer::fetch_buffer() 
		:	_column_size( 0 ), 
			_out_len( 0 ), 
			_value_buf( 0 ), 
			_actual_buf( 0 ), 
			_buf( nullptr ), 
			_size( 0 )
	{
	}

	fetch_buffer::~fetch_buffer()
	{
		if ( nullptr == _buf )	return;

		try
		{
		    boost::pool_allocator< char >::deallocate( _buf, _size );
		}
		catch ( const std::exception& e_ )
		{
			_fatal_log_( boost::format( "%1% ( %2% )" ) % e_.what() % __FILE_LINE__ );
		}
	}

	void	fetch_buffer::init()
	{
		_column_size	=	0;
		_out_len		=	0;
		_value_buf		=	0;
		_actual_buf		=	0;
	}

	bool	fetch_buffer::alloc_buf( size_t size_ )
	{
		if ( nullptr == _buf )
		{
			_size	=	size_;

			try
			{
				_buf	=	boost::pool_allocator< char >::allocate( _size );
				std::fill_n( _buf, _size, 0 );
				return true;
			}
			catch ( const std::exception& e_ )
			{
				_fatal_log_( boost::format( "%1% %2% ( %3% )" ) % _size % e_.what() % __FILE_LINE__ );
			}

			return false;
		}

		if ( _size < size_ )
		{
			if ( nullptr != _buf )
				boost::pool_allocator< char >::deallocate( _buf, _size );

			_size	=	size_;

			try
			{
				_buf	=	boost::pool_allocator< char >::allocate( _size );
				std::fill_n( _buf, _size, 0 );
				return true;
			}
			catch ( const std::exception& e_ )
			{
				_fatal_log_( boost::format( "%1% %2% ( %3% )" ) % _size % e_.what() % __FILE_LINE__ );
			}

			return false;
		}

		std::fill_n( _buf, _size, 0 );
		return	true;
	}

	handler::handler( uint64_t delay_query_ /*= _delay_query_*/ ) 
		:	_env( nullptr ), 
			_hdbc( nullptr ), 
			_stmt( nullptr ), 
			_is_connected( false ),
			_connect_info( { L"", L"", L"" } ),
			_delay_query( delay_query_ )
	{
		_big_buffer.fill( 0 );

		for ( size_t i = 0 ; i < _max_col_ ; ++i )
		{
			try
			{
				fetch_buffer_ptr_type	p( boost::make_shared< fetch_buffer >() );
				if ( nullptr == p )
				{
					_fatal_log_( boost::format( "( %1% )" ) % __FILE_LINE__ );
					continue;
				}

				p->init();
				_buffer_pool.emplace_back( p );
			}
			catch ( const std::exception& e_ )
			{
				_fatal_log_( boost::format( "%1% ( %2% )" ) % e_.what() % __FILE_LINE__ );	
			}
		}
	}

	handler::~handler()	
	{
		close();
	}

	bool	handler::db_connect(	
		const std::string_view& dsn_, 
		const std::string_view& id_, 
		const std::string_view& pass_ )
	{
		try
		{
			std::wstring	dsn( boost::locale::conv::utf_to_utf< wchar_t >( dsn_.data() ) );
			std::wstring	id( boost::locale::conv::utf_to_utf< wchar_t >( id_.data() ) );
			std::wstring	pass( boost::locale::conv::utf_to_utf< wchar_t >( pass_.data() ) );

			_unique_lock_( *this );
			return connect( dsn, id, pass );
		}
		catch ( const std::exception& e_ )
		{
			_fatal_log_( 
				boost::format( "%1% %2% %3% %4%( %5% )" ) 
					%	dsn_
					%	id_ 
					%	pass_ 
					%	e_.what() 
					%	__FILE_LINE__ );
		}

		return false;			
	}

	void	handler::db_close()
	{
		_unique_lock_( *this );
		close();
	}

	bool	handler::db_reconnect()
	{
		if ( true == ping() )	return true;

		close();
		return	connect( 
			_connect_info.get< 0 >(), 
			_connect_info.get< 1 >(), 
			_connect_info.get< 2 >() );
	}

	bool	handler::ping()
	{
		command				cmd;
		param_vector_type	o;
		cmd.ping_query();
		int16_t	sql_ret	=	0;
		return	exec( cmd, sql_ret, o );
	}

	bool	handler::connect(	
		const std::wstring_view& dsn_, 
		const std::wstring_view& id_, 
		const std::wstring_view& pass_ )
	{
		if ( true == dsn_.empty() || true == id_.empty() || true == pass_.empty() )
		{
			_error_log_( 
				boost::wformat( L"%1% %2% %3%( %4% )" ) 
					%	dsn_ 
					%	id_ 
					%	pass_ 
					%	__L_FILE_LINE__ );

			return false;
		}

		close();

		SQLRETURN	ret	=	SQL_ERROR;

		if ( SQL_SUCCESS != ( ret = ::SQLAllocEnv( &_env ) ) )
		{
			_error_log_( boost::format( "%1% ( %2% )" ) % ret % __FILE_LINE__ );
			return false;
		}

		if ( SQL_SUCCESS != ( ret = ::SQLAllocHandle( SQL_HANDLE_DBC, _env, &_hdbc ) ) )
		{
			_error_log_( boost::format( "%1% ( %2% )" ) % ret % __FILE_LINE__ );
			return false;
		}

		uint32_t	timeout = 5;
		if ( SQL_SUCCESS != ( ret = ::SQLSetConnectAttr( 
			_hdbc, 
			SQL_ATTR_CONNECTION_TIMEOUT, 
			reinterpret_cast< SQLPOINTER >( &timeout ), 
			SQL_IS_UINTEGER ) ) )
		{
			_error_log_( boost::format( "%1% ( %2% )" ) % ret % __FILE_LINE__ );
			return false;
		}

		ret	=	::SQLConnect( 
			_hdbc, 
			const_cast< TCHAR* >( dsn_.data() ), 
			SQL_NTS, 
			const_cast< TCHAR* >( id_.data() ), 
			SQL_NTS, 
			const_cast< TCHAR* >( pass_.data() ), 
			SQL_NTS );

		switch ( ret )
		{
		case	SQL_SUCCESS:
		case	SQL_SUCCESS_WITH_INFO:
			break;

		default:
			{
				std::wstring	state;
				std::wstring	message;
				( SQL_SUCCESS == diag_rec( e_diag_rec::_hdbc_, state, message ) ) ? 
					_error_log_( 
						boost::wformat( L"%1% %2% %3% ( %4% )" ) 
							%	ret 
							%	state 
							%	message 
							%	__L_FILE_LINE__ ) :
					_error_log_( 
						boost::wformat( L"%1% ( %2% )" ) 
							%	ret 
							%	__L_FILE_LINE__ );
			}
			return false;			
		}

		_connect_info.get< 0 >().assign( dsn_.data(), dsn_.size() );
		_connect_info.get< 1 >().assign( id_.data(), id_.size() );
		_connect_info.get< 2 >().assign( pass_.data(), pass_.size() );

		return	_is_connected = true;
	}

	void	handler::close()
	{
		if ( nullptr != _stmt )
		{
			::SQLFreeHandle( SQL_HANDLE_STMT, _stmt );
			_stmt = nullptr;
		}

		if ( nullptr != _hdbc )
		{
			::SQLDisconnect( _hdbc );
			::SQLFreeHandle( SQL_HANDLE_DBC, _hdbc );
			_hdbc = nullptr;
		}

		if ( nullptr != _env )
		{
			::SQLFreeHandle( SQL_HANDLE_ENV, _env );
			_env = nullptr;
		}

		_is_connected = false;
	}

	bool	handler::exec(	
		const command& cmd_, 
		OUT int16_t& sql_ret_,
		OUT param_vector_type& out_v_, 
		uint16_t time_out_ /*= defalut_timeout*/ )
	{
		sql_ret_	=	0;

		_unique_lock_( *this );

		return_fetch_buffer( _buf_vec );

		if ( false == _is_connected )
		{
			if ( false == connect( 
				_connect_info.get< 0 >(), 
				_connect_info.get< 1 >(), 
				_connect_info.get< 2 >() ) )
				return false;
		}

		SQLINTEGER	dead	=	SQL_CD_TRUE;
		sql_ret_			=	::SQLGetConnectAttr( 
			_hdbc, 
			SQL_ATTR_CONNECTION_DEAD, 
			&dead, 
			0, 
			nullptr );

		switch ( sql_ret_ )
		{
		case	SQL_SUCCESS:
		case	SQL_SUCCESS_WITH_INFO:
			if ( SQL_CD_TRUE == dead )
			{
				close();
				connect( _connect_info.get< 0 >(), _connect_info.get< 1 >(), _connect_info.get< 2 >() );
				sql_ret_ = ::SQLAllocHandle( SQL_HANDLE_STMT, _hdbc, &_stmt );

				switch ( sql_ret_ )
				{
				case	SQL_SUCCESS:
				case	SQL_SUCCESS_WITH_INFO:
					break;

				default:
					{
						std::wstring	state;
						std::wstring	message;
						( SQL_SUCCESS == diag_rec( e_diag_rec::_stmt_, state, message ) ) ?
							_error_log_( 
								boost::wformat( L"%1% %2% %3% ( %4% )" ) 
									%	sql_ret_ 
									%	state 
									%	message 
									%	__L_FILE_LINE__ ) :
							_error_log_( 
								boost::format( "%1% ( %2% )" ) 
									%	sql_ret_
									%	__FILE_LINE__ );
					}
					return false;
				}
			}
			break;

		default:
			{
				close();
				connect( _connect_info.get< 0 >(), _connect_info.get< 1 >(), _connect_info.get< 2 >() );
				sql_ret_ = ::SQLAllocHandle( SQL_HANDLE_STMT, _hdbc, &_stmt );
				if ( SQL_INVALID_HANDLE == sql_ret_ || SQL_ERROR == sql_ret_ )
				{
					std::wstring	state;
					std::wstring	message;
					( SQL_SUCCESS == diag_rec( e_diag_rec::_stmt_, state, message ) ) ?
						_error_log_( 
							boost::wformat( L"%1% %2% %3% ( %4% )" ) 
								%	sql_ret_ 
								%	state 
								%	message 
								%	__L_FILE_LINE__ ) :
						_error_log_( 
							boost::format( "%1% ( %2% )" ) 
								%	sql_ret_ 
								%	__FILE_LINE__ );

					return false;
				}
			}
		}

		if ( nullptr != _stmt )
		{
			::SQLFreeHandle( SQL_HANDLE_STMT, _stmt );
			_stmt = nullptr;
		}

		sql_ret_	=	::SQLAllocHandle( SQL_HANDLE_STMT, _hdbc, &_stmt );
		if ( SQL_INVALID_HANDLE == sql_ret_ || SQL_ERROR == sql_ret_ )
		{
			close();
			connect( _connect_info.get< 0 >(), _connect_info.get< 1 >(), _connect_info.get< 2 >() );

			sql_ret_ = ::SQLAllocHandle( SQL_HANDLE_STMT, _hdbc, &_stmt );
			if ( SQL_INVALID_HANDLE == sql_ret_ || SQL_ERROR == sql_ret_ )
			{
				std::wstring	state;
				std::wstring	message;
				( SQL_SUCCESS == diag_rec( e_diag_rec::_stmt_, state, message ) ) ? 
					_error_log_( 
						boost::wformat( L"%1% %2% %3% ( %4% )" ) 
							%	sql_ret_ 
							%	state 
							%	message 
							%	__L_FILE_LINE__ ) : 
					_error_log_( 
						boost::format( "%1% ( %2% )" ) 
							%	sql_ret_ 
							%	__FILE_LINE__ );

				return false;
			}
		}

		try
		{
			if ( false == bind_parameters( cmd_, time_out_ ) )		return false;

			SQLULEN			row_size = 0;
			SQLUSMALLINT	status = 0;
			sql_ret_		=		::SQLExtendedFetch( _stmt, SQL_FETCH_NEXT, 0, &row_size, &status );
			switch ( sql_ret_ )
			{
			case	SQL_SUCCESS:
			case	SQL_SUCCESS_WITH_INFO:
				break;

			default:
				{
					std::wstring	state;
					std::wstring	message;
					( SQL_SUCCESS == diag_rec( e_diag_rec::_stmt_, state, message ) ) ? 
						_error_log_( 
							boost::wformat( L"%1% %2% %3% ( %4% )" ) 
								%	sql_ret_ 
								%	state 
								%	message 
								%	__L_FILE_LINE__ ) : 
						_error_log_( 
							boost::format( "%1% ( %2% )" ) 
								%	sql_ret_ 
								%	__FILE_LINE__ );
				}

				return false;
			}

			if ( 0 == row_size )
			{
				_error_log_( 
					boost::format( "%1% %2% ( %3% )" ) 
						%	sql_ret_ 
						%	status 
						%	__FILE_LINE__ );

				return false;
			}

			return	fetch( cmd_, out_v_ );
		}
		catch ( const std::exception& e_ )
		{
			_fatal_log_( boost::format( "%1% ( %2% )" ) % e_.what() % __FILE_LINE__ );
		}

		return false;
	}

	bool	handler::exec(	
		const command& cmd_, 
		OUT int16_t& sql_ret_,
		OUT param_vector_rows_type& out_v_, 
		uint16_t time_out_ /*= defalut_timeout*/ )
	{
		sql_ret_	=	0;

		_unique_lock_( *this );

		return_fetch_buffer( _buf_vec );

		if ( false == _is_connected )
		{
			if ( false == connect( 
				_connect_info.get< 0 >(), 
				_connect_info.get< 1 >(), 
				_connect_info.get< 2 >() ) )
				return false;
		}

		SQLINTEGER	dead	=	SQL_CD_TRUE;
		sql_ret_			=	::SQLGetConnectAttr( 
			_hdbc, 
			SQL_ATTR_CONNECTION_DEAD, 
			&dead, 
			0, 
			nullptr );

		switch ( sql_ret_ )
		{
		case	SQL_SUCCESS:
		case	SQL_SUCCESS_WITH_INFO:
			if ( SQL_CD_TRUE == dead )
			{
				close();
				connect( _connect_info.get< 0 >(), _connect_info.get< 1 >(), _connect_info.get< 2 >() );
				sql_ret_ = ::SQLAllocHandle( SQL_HANDLE_STMT, _hdbc, &_stmt );
				switch ( sql_ret_ )
				{
				case	SQL_SUCCESS:
				case	SQL_SUCCESS_WITH_INFO:
					break;

				default:
					{
						std::wstring	state;
						std::wstring	message;
						( SQL_SUCCESS == diag_rec( e_diag_rec::_stmt_, state, message ) ) ?
							_error_log_( 
								boost::wformat( L"%1% %2% %3% ( %4% )" ) 
									%	sql_ret_ 
									%	state 
									%	message 
									%	__L_FILE_LINE__ ) :
							_error_log_( 
								boost::format( "%1% ( %2% )" ) 
									%	sql_ret_ 
									%	__FILE_LINE__ );
					}
					return false;
				}
			}
			break;

		default:
			{
				close();
				connect( _connect_info.get< 0 >(), _connect_info.get< 1 >(), _connect_info.get< 2 >() );
				sql_ret_ = ::SQLAllocHandle( SQL_HANDLE_STMT, _hdbc, &_stmt );
				if ( SQL_INVALID_HANDLE == sql_ret_ || SQL_ERROR == sql_ret_ )
				{
					std::wstring	state;
					std::wstring	message;
					( SQL_SUCCESS == diag_rec( e_diag_rec::_stmt_, state, message ) ) ?
						_error_log_( 
							boost::wformat( L"%1% %2% %3% ( %4% )" ) 
								%	sql_ret_ 
								%	state 
								%	message 
								%	__L_FILE_LINE__ ) :
						_error_log_( 
							boost::format( "%1% ( %2% )" ) 
								%	sql_ret_ 
								%	__FILE_LINE__ );

					return false;
				}
			}
		}

		if ( nullptr != _stmt )
		{
			::SQLFreeHandle( SQL_HANDLE_STMT, _stmt );
			_stmt = nullptr;
		}

		sql_ret_ = ::SQLAllocHandle( SQL_HANDLE_STMT, _hdbc, &_stmt );
		if ( SQL_INVALID_HANDLE == sql_ret_ || SQL_ERROR == sql_ret_ )
		{
			close();
			connect( _connect_info.get< 0 >(), _connect_info.get< 1 >(), _connect_info.get< 2 >() );

			sql_ret_ = ::SQLAllocHandle( SQL_HANDLE_STMT, _hdbc, &_stmt );
			if ( SQL_INVALID_HANDLE == sql_ret_ || SQL_ERROR == sql_ret_ )
			{
				std::wstring	state;
				std::wstring	message;
				( SQL_SUCCESS == diag_rec( e_diag_rec::_stmt_, state, message ) ) ? 
					_error_log_( 
						boost::wformat( L"%1% %2% %3% ( %4% )" ) 
							%	sql_ret_ 
							%	state 
							%	message 
							%	__L_FILE_LINE__ ) : 
					_error_log_( 
						boost::format( "%1% ( %2% )" ) 
							%	sql_ret_ 
							%	__FILE_LINE__ );

				return false;
			}
		}

		try
		{
			if ( false == bind_parameters( cmd_, time_out_ ) )		return false;

			SQLULEN			row_size = 0;
			SQLUSMALLINT	status = 0;
			while ( true )
			{
				sql_ret_	=	::SQLExtendedFetch( 
					_stmt, 
					SQL_FETCH_NEXT, 
					0, 
					&row_size, 
					&status );

				switch ( sql_ret_ )
				{
				case	SQL_SUCCESS:
				case	SQL_SUCCESS_WITH_INFO:
					break;

				case	SQL_NO_DATA_FOUND:
					return true;

				default:
					{
						std::wstring	state;
						std::wstring	message;
						( SQL_SUCCESS == diag_rec( e_diag_rec::_stmt_, state, message ) ) ? 
							_error_log_( 
								boost::wformat( L"%1% %2% %3% ( %4% )" ) 
									%	sql_ret_ 
									%	state 
									%	message 
									%	__L_FILE_LINE__ ) : 
							_error_log_( 
								boost::format( "%1% ( %2% )" ) 
									%	sql_ret_ 
									%	__FILE_LINE__ );
					}
					return false;
				}

				if ( 0 == row_size )
				{
					_error_log_( 
						boost::format( "%1% %2% ( %3% )" ) 
							%	sql_ret_ 
							%	status 
							%	__FILE_LINE__ );

					return false;
				}

				out_v_.emplace_back();
				if ( false == fetch( cmd_, out_v_.back() ) )	return false;
			}

			return true;
		}
		catch ( const std::exception& e_ )
		{
			_fatal_log_( boost::format( "%1% ( %2% )" ) % e_.what() % __FILE_LINE__ );
		}

		return false;
	}

	SQLRETURN	handler::diag_rec( 
		e_diag_rec type_, 
		OUT std::wstring& state_, 
		OUT std::wstring& message_ )
	{
		SQLSMALLINT	rec_num								=	1;
		SQLWCHAR	sql_state[ 6 ]						=	{ 0 };
		SQLINTEGER	native_error						=	0;
		SQLWCHAR	msg[ SQL_MAX_MESSAGE_LENGTH + 1 ]	=	{ 0 };
		SQLSMALLINT	text_length							=	0;

		SQLRETURN	ret	=	SQL_ERROR;
		switch ( type_ )
		{
		case e_diag_rec::_env_:
			ret = ::SQLGetDiagRec( 
				SQL_HANDLE_ENV, 
				_env, 
				rec_num, 
				sql_state, 
				&native_error, 
				msg, 
				SQL_MAX_MESSAGE_LENGTH + 1, 
				&text_length );
			break;

		case e_diag_rec::_hdbc_:
			ret = ::SQLGetDiagRec( 
				SQL_HANDLE_DBC, 
				_hdbc, 
				rec_num, 
				sql_state, 
				&native_error, 
				msg, 
				SQL_MAX_MESSAGE_LENGTH + 1, 
				&text_length );
			break;

		case e_diag_rec::_stmt_:
			ret = ::SQLGetDiagRec( 
				SQL_HANDLE_STMT, 
				_stmt, 
				rec_num, 
				sql_state, 
				&native_error, 
				msg, 
				SQL_MAX_MESSAGE_LENGTH + 1, 
				&text_length );
			break;
		}

		if ( SQL_SUCCESS == ret )
		{
			state_		=	sql_state;
			message_	=	msg;
		}

		return	ret;
	}

	bool	handler::bind_parameters( const command& cmd_, uint16_t time_out_ /*= defalut_timeout*/ )
	{
		SQLRETURN	ret	=	SQL_ERROR;

		_total_call_query_time.start();

		if ( 0 < time_out_ )
		{
			::SQLSetStmtAttr( 
				_stmt, 
				SQL_ATTR_QUERY_TIMEOUT, 
				reinterpret_cast< SQLPOINTER >( time_out_ ), 
				SQL_IS_UINTEGER );
		}

		SQLUSMALLINT	count	=	0;
		SQLLEN			param	=	0;

		for ( const boost::any& r_ : cmd_.get_param_vec() )
		{
			auto	fb( alloc_fetch_buffer() );
			if ( !fb )	return	false;

			_buf_vec.emplace_back( *fb );

			if ( r_.type() == typeid( uint8_t ) )
			{
				_buf_vec.back()->_value_buf		=	boost::any_cast< uint8_t >( r_ );
				ret	=	::SQLBindParameter( 
					_stmt, 
					++count, 
					SQL_PARAM_INPUT, 
					SQL_C_UTINYINT, 
					SQL_TINYINT, 
					0, 
					0, 
					&_buf_vec.back()->_value_buf, 
					0, 
					&param );
			}
			else if ( r_.type() == typeid( short ) )
			{
				_buf_vec.back()->_value_buf		=	boost::any_cast< short >( r_ );
				ret	=	::SQLBindParameter( 
					_stmt, 
					++count, 
					SQL_PARAM_INPUT, 
					SQL_C_USHORT, 
					SQL_SMALLINT, 
					0, 
					0, 
					&_buf_vec.back()->_value_buf, 
					0, 
					&param );
			}
			else if ( r_.type() == typeid( int ) )
			{
				_buf_vec.back()->_value_buf		=	boost::any_cast< int >( r_ );
				ret	=	::SQLBindParameter( 
					_stmt, 
					++count, 
					SQL_PARAM_INPUT, 
					SQL_C_LONG, 
					SQL_INTEGER, 
					0, 
					0, 
					&_buf_vec.back()->_value_buf, 
					0, 
					&param );
			}
			else if ( r_.type() == typeid( long ) )
			{
				_buf_vec.back()->_value_buf	=	boost::any_cast< long >( r_ );
				ret	=	::SQLBindParameter( 
					_stmt, 
					++count, 
					SQL_PARAM_INPUT, 
					SQL_C_LONG, 
					SQL_INTEGER, 
					0, 
					0, 
					&_buf_vec.back()->_value_buf, 
					0, 
					&param );
			}
			else if ( r_.type() == typeid( float ) )
			{
				_buf_vec.back()->_actual_buf	=	boost::any_cast< float >( r_ );
				ret =	::SQLBindParameter( 
					_stmt, 
					++count, 
					SQL_PARAM_INPUT, 
					SQL_C_FLOAT, 
					SQL_FLOAT, 
					0, 
					0, 
					&_buf_vec.back()->_actual_buf, 
					0, 
					&param );
			}
			else if ( r_.type() == typeid( double ) )
			{
				_buf_vec.back()->_actual_buf	=	boost::any_cast< double >( r_ );
				ret =	::SQLBindParameter( 
					_stmt, 
					++count, 
					SQL_PARAM_INPUT, 
					SQL_C_DOUBLE, 
					SQL_DOUBLE, 
					0, 
					0, 
					&_buf_vec.back()->_actual_buf, 
					0, 
					&param );
			}
			else if ( r_.type() == typeid( __int64 ) )
			{	
				_buf_vec.back()->_value_buf		=	boost::any_cast< __int64 >( r_ );
				ret =	::SQLBindParameter( 
					_stmt, 
					++count, 
					SQL_PARAM_INPUT, 
					SQL_C_SBIGINT, 
					SQL_BIGINT, 
					0, 
					0, 
					&_buf_vec.back()->_value_buf, 
					0, 
					&param );
			}
			else if ( r_.type() == typeid( std::string ) )
			{
				param	=	SQL_NTS;

				const	std::string&	d	=	boost::any_cast< std::string >( r_ );
				_buf_vec.back()->alloc_buf( d.size() + sizeof( char ) );
				std::copy( d.begin(), d.end(), _buf_vec.back()->_buf );

				ret =	( 8000 < d.size() ) ?
						::SQLBindParameter( 
							_stmt, 
							++count, 
							SQL_PARAM_INPUT, 
							SQL_C_CHAR, 
							SQL_LONGVARCHAR, 
							_buf_vec.back()->size(), 
							0, 
							_buf_vec.back()->_buf, 
							0, 
							&param ) :
						::SQLBindParameter( 
							_stmt, 
							++count, 
							SQL_PARAM_INPUT, 
							SQL_C_CHAR, 
							SQL_VARCHAR, 
							_buf_vec.back()->size(), 
							0, 
							_buf_vec.back()->_buf, 
							0, 
							&param );
			}
			else if ( r_.type() == typeid( std::wstring ) )
			{
				param = SQL_NTS;

				const	std::wstring&	d	=	boost::any_cast< std::wstring >( r_ );
				_buf_vec.back()->alloc_buf( ( d.size() * sizeof( wchar_t ) + sizeof( wchar_t ) ) );
				std::copy( d.begin(), d.end(), reinterpret_cast< wchar_t* >( _buf_vec.back()->_buf ) );

				ret =	( 4000 < d.size() ) ?
					::SQLBindParameter( 
						_stmt, 
						++count, 
						SQL_PARAM_INPUT, 
						SQL_UNICODE, 
						SQL_WLONGVARCHAR, 
						_buf_vec.back()->size(), 
						0, 
						_buf_vec.back()->_buf, 
						0, 
						&param ) :
					::SQLBindParameter( 
						_stmt, 
						++count, 
						SQL_PARAM_INPUT, 
						SQL_UNICODE, 
						SQL_WVARCHAR, 
						_buf_vec.back()->size(), 
						0, 
						_buf_vec.back()->_buf, 
						0, 
						&param );
			}

			if ( SQL_ERROR == ret || SQL_INVALID_HANDLE == ret )
			{
				std::wstring	tmp;
				cmd_.get_params_string( tmp );

				std::wstring	state;
				std::wstring	message;
				( SQL_SUCCESS == diag_rec( e_diag_rec::_stmt_, state, message ) ) ? 
					_error_log_( 
						boost::wformat( L"%1% %2% %3% %4% ( %5% )" ) 
							%	state 
							%	message 
							%	cmd_.get_query() 
							%	tmp 
							%	__L_FILE_LINE__ ) : 
					_error_log_( 
						boost::wformat( L"%1% %2% %3% ( %4% )" ) 
							%	cmd_.get_query() 
							%	tmp 
							%	ret 
							%	__L_FILE_LINE__ );

				return	false;
			}
		}

		_call_query_time.start();

		ret	=	::SQLExecDirect( 
			_stmt, 
			const_cast< wchar_t* >( cmd_.get_query().c_str() ), 
			SQL_NTS );

		_call_query_time.end();
		_total_call_query_time.end();

		if ( _delay_query < _total_call_query_time.milliseconds() )
		{
			std::wstring	tmp;
			cmd_.get_params_string( tmp );

			_warning_log_( 
				boost::wformat( L"%1% %2% %3% %4% ( %5% )" ) 
					%	cmd_.get_query()
					%	tmp
					%	_total_call_query_time.milliseconds()
					%	_call_query_time.milliseconds()
					%	__L_FILE_LINE__ );
		}

		if ( SQL_ERROR == ret || SQL_INVALID_HANDLE == ret )
		{
			std::wstring	tmp;
			cmd_.get_params_string( tmp );

			std::wstring	state;
			std::wstring	message;
			( SQL_SUCCESS == diag_rec( e_diag_rec::_stmt_, state, message ) ) ? 
				_error_log_( 
					boost::wformat( L"%1% %2% %3% %4% ( %5% )" ) 
						%	state 
						%	message 
						%	cmd_.get_query()
						%	tmp
						%	__L_FILE_LINE__ ) : 
				_error_log_( 
					boost::wformat( L"%1% %2% %3% ( %4% )" ) 
						%	cmd_.get_query()
						%	tmp
						%	ret
						%	__L_FILE_LINE__ );
			return false;
		}

		return	true;
	}

	bool	handler::fetch( const command& cmd_, OUT param_vector_type& out_v_ )
	{
		if ( nullptr == _stmt )
		{
			std::wstring	tmp;
			cmd_.get_params_string( tmp );
			_fatal_log_( 
				boost::wformat( L"%1% %2% ( %3% )" ) 
					%	cmd_.get_query() 
					%	tmp
					%	__L_FILE_LINE__ );

			return false;
		}

		SQLCHAR			desc[ 256 ]	=	{ 0 };
		SQLSMALLINT		desc_type	=	0;
		SQLLEN			col_size	=	0;		
		SQLRETURN		ret			=	::SQLColAttributes( 
			_stmt, 
			0, 
			SQL_COLUMN_COUNT, 
			desc, 
			256, 
			&desc_type, 
			&col_size );

		switch ( ret )
		{
		case SQL_SUCCESS:
		case SQL_SUCCESS_WITH_INFO:
			break;

		default:
			{
				std::wstring	tmp;
				cmd_.get_params_string( tmp );

				std::wstring	state;
				std::wstring	message;
				( SQL_SUCCESS == diag_rec( e_diag_rec::_stmt_, state, message ) ) ? 
					_error_log_( 
						boost::wformat( L"%1% %2% %3% %4% ( %5% )" ) 
							%	state
							%	message
							%	cmd_.get_query()
							%	tmp
							%	__L_FILE_LINE__ ) : 
					_error_log_( 
						boost::wformat( L"%1% %2% %3% ( %4% )" ) 
							%	cmd_.get_query()
							%	tmp
							%	ret
							%	__L_FILE_LINE__ );
			}
			return false;
		}

		if ( _single_row_max_col_ < col_size )
		{
			std::wstring	tmp;
			cmd_.get_params_string( tmp );
			_error_log_( 
				boost::wformat( L"very may cols %1% %2% %3% ( %4% )" )
					%	col_size
					%	cmd_.get_query()
					%	tmp	
					%	__L_FILE_LINE__ );

			return false;
		}

		SQLWCHAR        col_name[ 32 ]		=	{ 0 };
		SQLSMALLINT     data_type			=	0;
		SQLSMALLINT     col_name_len		=	0;
		SQLSMALLINT     nullable			=	0;
		SQLSMALLINT     scale				=	0;
		SQLCHAR         errmsg[ 256 ]		=	{ 0 };
		SQLRETURN       rc					=	SQL_ERROR;

		for ( size_t i = 0 ; i < static_cast< size_t >( col_size ) ; ++i ) 
		{
			auto	fb( alloc_fetch_buffer() );
			if ( !fb )
			{
				_fatal_log_( boost::format( "%1% ( %2% )" ) % i % __FILE_LINE__ );
				return	false;
			}

			_buf_vec.emplace_back( *fb );

			fetch_buffer_ptr_type&	buf			=	_buf_vec.back();
			SQLUSMALLINT			index		=	static_cast< SQLUSMALLINT >( i + 1 );
			SQLULEN					column_size	=	0;
			ret		=	::SQLDescribeCol(
				_stmt, 
				index, 
				col_name, 
				static_cast< SQLSMALLINT >( sizeof( col_name ) ), 
				&col_name_len, 
				&data_type, 
				&column_size, 
				&scale, 
				&nullable );

			switch ( ret )
			{
			case SQL_SUCCESS:
			case SQL_SUCCESS_WITH_INFO:
				break;

			default:
				{
					std::wstring	tmp;
					cmd_.get_params_string( tmp );

					std::wstring	state;
					std::wstring	message;
					( SQL_SUCCESS == diag_rec( e_diag_rec::_stmt_, state, message ) ) ? 
						_error_log_( 
							boost::wformat( L"%1% %2% %3% %4% ( %5% )" ) 
								%	state
								%	message
								%	cmd_.get_query()
								%	tmp
								%	__L_FILE_LINE__ ) : 
						_error_log_( 
							boost::wformat( L"%1% %2% %3% ( %4% )" ) 
								%	cmd_.get_query() 
								%	tmp 
								%	ret 
								%	__L_FILE_LINE__ );
				}
				return false;
			}

			SQLSMALLINT	input_data_type	=	data_type;
			switch ( input_data_type )
			{
			case	SQL_WCHAR:
			case	SQL_WVARCHAR:
			case	SQL_WLONGVARCHAR:
				buf->_column_size	*=	2;
				input_data_type		=	SQL_UNICODE;
				break;

			case	SQL_BINARY:
			case	SQL_VARBINARY:
			case	SQL_LONGVARBINARY:
				input_data_type		=	SQL_C_BINARY;
				break;

			default:
				input_data_type		=	SQL_C_CHAR;
			}

			SQLLEN	len	= 0;
			if ( _max_text_ < column_size || SQL_C_BINARY == input_data_type )
			{
				_big_buffer.fill( 0 );
				rc	=	::SQLGetData( 
					_stmt, 
					index, 
					input_data_type, 
					_big_buffer.data(), 
					_big_buffer.size(), 
					&len );
				buf->_column_size	=	len;
				buf->alloc_buf( buf->_column_size + 1 );
				std::copy( 
					_big_buffer.begin(), 
					_big_buffer.begin() + len, 
					buf->_buf );
			}
			else
			{
				buf->_column_size	=	column_size;
				buf->alloc_buf( buf->_column_size + 1 );
				rc	=	::SQLGetData( 
					_stmt, 
					index, 
					input_data_type, 
					buf->_buf, 
					buf->size(), 
					&len );
			}		

			switch ( ret )
			{
			case SQL_SUCCESS:
			case SQL_SUCCESS_WITH_INFO:
				break;

			default:
				{
					std::wstring	tmp;
					cmd_.get_params_string( tmp );

					std::wstring	state;
					std::wstring	message;
					( SQL_SUCCESS == diag_rec( e_diag_rec::_stmt_, state, message ) ) ? 
						_error_log_( 
							boost::wformat( L"%1% %2% %3% %4% ( %5% )" ) 
								%	state
								%	message
								%	cmd_.get_query()
								%	tmp
								%	__L_FILE_LINE__ ) : 
						_error_log_( 
							boost::wformat( L"%1% %2% %3% ( %4% )" ) 
								%	cmd_.get_query() 
								%	tmp
								%	ret
								%	__L_FILE_LINE__ );
				}
				return false;
			}

			switch ( data_type )
			{
			case	SQL_TINYINT:
				{
					uint8_t	data	=	0;
					if ( nullptr != buf->_buf && 0 < len )
						data	=	static_cast< uint8_t >( boost::lexical_cast< uint16_t >( buf->_buf ) );

					out_v_.emplace_back( data );
				}
				break;

			case	SQL_SMALLINT:
				{
					short	data = 0;
					if ( nullptr != buf->_buf && 0 < len )
						data = boost::lexical_cast< short >( buf->_buf );

					out_v_.emplace_back( data );
				}
				break;

			case	SQL_INTEGER:
				{
					int		data = 0;
					if ( nullptr != buf->_buf && 0 < len )
						data = boost::lexical_cast< int >( buf->_buf );

					out_v_.emplace_back( data );
				}
				break;

			case	SQL_FLOAT:
			case	SQL_REAL:
				{
					float	data = 0.00;
					if ( nullptr != buf->_buf && 0 < len )
						data = boost::lexical_cast< float >( buf->_buf );

					out_v_.emplace_back( data );
				}
				break;

			case	SQL_NUMERIC:
			case	SQL_DECIMAL:
			case	SQL_DOUBLE:
				{
					double	data = 0.00;
					if ( nullptr != buf->_buf && 0 < len )
						data = boost::lexical_cast< double >( buf->_buf );

					out_v_.emplace_back( data );
				}
				break;

			case	SQL_BIGINT:
				{
					__int64	data = 0;
					if ( nullptr != buf->_buf && 0 < len )
						data = boost::lexical_cast< __int64 >( buf->_buf );

					out_v_.emplace_back( data );
				}
				break;

			case	SQL_CHAR:
			case	SQL_VARCHAR:
			case	SQL_LONGVARCHAR:
			case	SQL_BINARY:
			case	SQL_VARBINARY:
			case	SQL_LONGVARBINARY:
			case	SQL_TIMESTAMP:
				{
					std::string	data;
					if ( nullptr != buf->_buf && 0 < len )
					{
						data.resize( len );
						std::copy_n( buf->_buf, len, data.data() );
					}

					out_v_.emplace_back( data );
				}
				break;

			case	SQL_WCHAR:
			case	SQL_WVARCHAR:
			case	SQL_WLONGVARCHAR:
				{
					std::wstring	data;
					if ( nullptr != buf->_buf && 0 < len )
					{
						data.resize( len / 2 );
						std::copy_n( reinterpret_cast< wchar_t* >( buf->_buf ), ( len / 2 ), data.data() );
					}

					out_v_.emplace_back( data );
				}
				break;
			}			
		}

		return true;
	}

	boost::optional< fetch_buffer_ptr_type >	
		handler::alloc_fetch_buffer()
	{
		fetch_buffer_ptr_type	p;
		if ( true == _buffer_pool.empty() )
		{
			try
			{
				p	=	boost::make_shared< fetch_buffer >();
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

		p	=	_buffer_pool.front();
		_buffer_pool.pop_front();

		if ( nullptr == p )
		{
			_fatal_log_( boost::format( "( %1% )" ) % __FILE_LINE__ );
			return	{};
		}
			
		p->init();
		return	{ p };
	}

	void	handler::return_fetch_buffer( MOVE IN std::vector< fetch_buffer_ptr_type >& v_ )
	{
		if ( true == v_.empty() )	return;
		if ( _max_col_ <= _buffer_pool.size() )
		{
			v_.clear();
			return;
		}

		std::move( 
			std::begin( v_ ), 
			std::end( v_ ), 
			std::back_inserter( _buffer_pool ) );

		v_.clear();
	}

	handler_wrapper::handler_wrapper( uint64_t delay_query_ /*= handler::_delay_query_*/ ) 
		:	_handler( delay_query_ )
	{
	}

	handler_wrapper::~handler_wrapper()
	{
		close();
	}

	bool	handler_wrapper::connect( 
		const std::string_view& dsn_, 
		const std::string_view& id_, 
		const std::string_view& auth_ )
	{
		if ( false == _handler.db_connect( dsn_, id_, auth_ ) )
		{
			_error_log_( 
				boost::format( "%1% %2% %3%( %4% )" ) 
					%	dsn_ 
					%	id_ 
					%	auth_ 
					%	__FILE_LINE__ );

			return	false;
		}

		return	true;
	}

	void	handler_wrapper::close()
	{
		_handler.db_close();
	}

	void	handler_wrapper::set_delay_time( uint64_t delay_query_ )	
	{
		if ( handler::_delay_query_ > delay_query_ )		return;
		_handler.set_delay_time( delay_query_ );
	}

	bool	pool::connect( 
		const std::string_view& dsn_, 
		const std::string_view& id_, 
		const std::string_view& auth_, 
		uint8_t pool_size_ /*= _default_db_pool_size_*/ )
	{
		if ( true == dsn_.empty() || true == id_.empty() || true == auth_.empty() )
		{
			_error_log_( 
				boost::format( "%1% %2% %3% %4% ( %5% )" ) 
					%	dsn_ 
					%	id_ 
					%	auth_ 
					%	pool_size_ 
					%	__FILE_LINE__ );

			return	false;
		}

		if ( _default_db_pool_size_ > pool_size_ )
			pool_size_	=	_default_db_pool_size_;

		if ( _max_db_pool_size_ < pool_size_ )
			pool_size_	=	_max_db_pool_size_;

		_unique_lock_( *this );

		_early_pool_size	=	pool_size_;

		_q.clear();
		for ( uint8_t i = 0 ; i < pool_size_ ; ++i )
		{
			_q.emplace_back( boost::make_shared< handler_wrapper >() );

			if ( false == _q.back()->connect( dsn_, id_, auth_ ) )	
			{
				_error_log_( boost::format( "%1% %2% ( %3% )" ) % dsn_ % id_ % __FILE_LINE__ );
				_q.clear();
				return	false;
			}
		}

		_dsn.assign( dsn_.data(), dsn_.size() );
		_id.assign( id_.data(), id_.size() );
		_auth.assign( auth_.data(), auth_.size() );

		return true;
	}

	bool	pool::alloc_odbc_ptr( OUT handler_wrapper_ptr_type& ptr_ )
	{
		_unique_lock_( *this );

		if ( true == _q.empty() )
		{
			if ( _max_db_pool_size_ <= _early_pool_size )	return false;

			try
			{
				_early_pool_size++;
				ptr_	=	boost::make_shared< handler_wrapper >();
			}
			catch ( const std::exception& e_ )
			{
				_fatal_log_( boost::format( "%1% ( %2% )" ) % e_.what() % __FILE_LINE__ );
				_early_pool_size--;
				return false;
			}
				
			return	ptr_->connect( _dsn, _id, _auth );
		}

		ptr_	=	_q.front();
		_q.pop_front();
		return true;
	}

	bool	pool::compulsion_alloc_odbc_ptr( OUT handler_wrapper_ptr_type& ptr_ )
	{
		try
		{
			ptr_	=	boost::make_shared< handler_wrapper >();
		}
		catch ( const std::exception& e_ )
		{
			_fatal_log_( boost::format( "%1% ( %2% )" ) % e_.what() % __FILE_LINE__ );
			return false;
		}

		return	ptr_->connect( _dsn, _id, _auth );
	}

	void	pool::release_odbc_ptr( const handler_wrapper_ptr_type& ptr_ )
	{
		{
			_unique_lock_( *this );

			if ( _max_db_pool_size_ > _q.size() )
			{
				_q.emplace_back( ptr_ );
				return;
			}
		}
			
		ptr_->close();
	}

	void	pool::set_delay_time( uint64_t delay_query_ )
	{
		_unique_lock_( *this );

		for ( const handler_wrapper_ptr_type& r_ : _q )
			r_->set_delay_time( delay_query_ );
	}

	boost::optional< param_vector_type >		
		stored_procedure::single_row( uint16_t time_out_ /*= handler::_defalut_timeout_*/ )
	{
		if ( false == _command.make_query( get_query() ) )		return {};

		param_vector_type	outs;
		size_t				retry	=	1;
		int16_t				sql_ret	=	0;
		while ( false == _handler->_handler.exec( _command, sql_ret, outs, time_out_ ) )
		{
			if ( _retry_count_ < retry )
				return	{};

			_error_log_( 
				boost::format( "%1% %2% %3% ( %4% )" ) 
					%	get_query() 
					%	sql_ret 
					%	( retry++ ) 
					%	__FILE_LINE__ );

			if ( SQL_NO_DATA == sql_ret )
				return	{};

			size_t	reconnect_count	=	1;
			while ( false == _handler->_handler.db_reconnect() )
			{
				if ( _reconnect_count_ < reconnect_count )	return	{};

				_error_log_( 
					boost::format( "%1% %2% ( %3% )" ) 
						%	get_query() 
						%	( reconnect_count++ ) 
						%	__FILE_LINE__ );

				boost::this_thread::sleep( boost::posix_time::milliseconds( 5 ) );
				continue;
			}
				
			outs.clear();
			_command.make_query( get_query() );
		}

		return	{ boost::move( outs ) };
	}

	boost::optional< param_vector_rows_type >		
		stored_procedure::multi_row( uint16_t time_out_ /*= handler::_defalut_timeout_*/ )
	{
		if ( false == _command.make_query( get_query() ) )		return {};

		param_vector_rows_type	outs;
		size_t					retry	=	1;
		int16_t					sql_ret	=	0;
		while ( false == _handler->_handler.exec( _command, sql_ret, outs, time_out_ ) )
		{
			if ( _retry_count_ < retry )	
				return {};

			_error_log_( 
				boost::format( "%1% %2% %3% ( %4% )" ) 
					%	get_query() 
					%	sql_ret 
					%	( retry++ ) 
					%	__FILE_LINE__ );

			if ( SQL_NO_DATA == sql_ret )
				return	{};

			size_t	reconnect_count = 1;
			while ( false == _handler->_handler.db_reconnect() )
			{
				if ( _reconnect_count_ < reconnect_count )	return	{};

				_error_log_( 
					boost::format( "%1% %2% ( %3% )" ) 
						%	get_query()
						%	( reconnect_count++ )
						%	__FILE_LINE__ );

				boost::this_thread::sleep( boost::posix_time::milliseconds( 5 ) );
				continue;
			}

			outs.clear();
			_command.make_query( get_query() );
		}

		return	{ boost::move( outs ) };
	}

	stored_procedure::stored_procedure( const std::string_view& query_ ) 
		:	_query( query_.data(), query_.size() )
	{
		if ( true == query_.empty() )
			_error_log_( boost::format( "( %1% )" ) % __FILE_LINE__ );
	}

	void	stored_procedure::init()
	{
		_command.init();
	}

	void	stored_procedure::bind( uint8_t v_ )
	{
		_command.bind_parameter< uint8_t >( v_ );
	}

	void	stored_procedure::bind( short v_ )
	{
		_command.bind_parameter< short >( v_ );
	}

	void	stored_procedure::bind( int v_ )
	{
		_command.bind_parameter< int >( v_ );
	}

	void	stored_procedure::bind( long v_ )
	{
		_command.bind_parameter< long >( v_ );
	}

	void	stored_procedure::bind( float v_ )
	{
		_command.bind_parameter< float >( v_ );
	}

	void	stored_procedure::bind( double v_ )
	{
		_command.bind_parameter< double >( v_ );
	}

	void	stored_procedure::bind( __int64 v_ )
	{
		_command.bind_parameter< __int64 >( v_ );
	}

	void	stored_procedure::bind( const std::string& v_ )
	{
		_command.bind_parameter< std::string >( v_ );
	}

	void	stored_procedure::bind( const std::wstring& v_ )
	{
		_command.bind_parameter< std::wstring >( v_ );
	}
}