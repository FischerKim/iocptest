#pragma once

namespace	impl::odbc
{
	class	stored_procedure : private boost::noncopyable
	{
	private:
		enum
		{
			_retry_count_		=	5,
			_reconnect_count_	=	5,
		};

	public:
		stored_procedure( const std::string_view& query_ );
		virtual ~stored_procedure()	{}

		virtual	void	init();

		const std::string&	get_query()	const	{	return	_query;	}

	protected:
		void	bind( uint8_t v_ );
		void	bind( short v_ );
		void	bind( int v_ );
		void	bind( long v_ );
		void	bind( float v_ );
		void	bind( double v_ );
		void	bind( __int64 v_ );
		void	bind( const std::string& v_ );
		void	bind( const std::wstring& v_ );

		boost::optional< param_vector_type >		
			single_row( uint16_t time_out_ = handler::_defalut_timeout_ );

		boost::optional< param_vector_rows_type >	
			multi_row( uint16_t time_out_ = handler::_defalut_timeout_ );

	private:			
		command				_command;
		const std::string	_query;

	protected:
		handler_wrapper_ptr_type	_handler;
	};
}