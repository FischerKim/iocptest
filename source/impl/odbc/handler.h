#pragma once

namespace	impl::odbc
{
	class handler	:	private boost::noncopyable,
						private	boost::mutex
	{
	public:
		enum 
		{	
			_single_row_max_col_	=	100,
			_max_col_				=	10000,
			_defalut_timeout_		=	2,
			_delay_query_			=	50,
			_max_text_				=	8000,
			_big_buffer_size_		=	0xffff,
		};

	public:
		explicit	handler( uint64_t delay_query_ = _delay_query_ );
		~handler();

	public:
		bool	db_connect( 
			const std::string_view& dsn_, 
			const std::string_view& id_, 
			const std::string_view& pass_ );

		void	db_close();
		bool	db_reconnect();
		bool	ping();

		bool	exec(	
			const command& cmd_, 
			OUT int16_t& sql_ret_,
			OUT param_vector_type& out_v_, 
			uint16_t time_out_ = _defalut_timeout_ );

		bool	exec(	
			const command& cmd_, 
			OUT int16_t& sql_ret_,
			OUT param_vector_rows_type& out_v_, 
			uint16_t time_out_ = _defalut_timeout_ );

		void	set_delay_time( uint64_t delay_query_ )		{	_delay_query	=	delay_query_;	}

	private:
		bool	connect(	
			const std::wstring_view& dsn_, 
			const std::wstring_view& id_, 
			const std::wstring_view& pass_ );

		void	close();

		enum class e_diag_rec { _env_, _hdbc_, _stmt_ };
		SQLRETURN	diag_rec(	
			e_diag_rec type_, 
			OUT std::wstring& state_, 
			OUT std::wstring& message_ );

		bool	bind_parameters( 
			const command& cmd_, 
			uint16_t time_out_ = _defalut_timeout_ );

		bool	fetch( 
			const command& cmd_, 
			OUT param_vector_type& out_v_ );

		using optional_fb	=	boost::optional< fetch_buffer_ptr_type >;
		optional_fb	
			alloc_fetch_buffer();

		void	return_fetch_buffer( 
			MOVE IN std::vector< fetch_buffer_ptr_type >& v_ );

	private:
		SQLHANDLE								_env;
		SQLHANDLE								_hdbc;
		SQLHANDLE								_stmt;			

		using	connect_info_type	=	boost::tuple< std::wstring, std::wstring, std::wstring >;
		connect_info_type						_connect_info;

		bool									_is_connected;

		util::time_duration						_call_query_time;
		util::time_duration						_total_call_query_time;

		std::deque< fetch_buffer_ptr_type >		_buffer_pool;
		std::vector< fetch_buffer_ptr_type >	_buf_vec;

		std::array< char, _big_buffer_size_ >	_big_buffer;

		uint64_t								_delay_query;
	};
}