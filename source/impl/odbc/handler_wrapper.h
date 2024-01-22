#pragma once

namespace	impl::odbc
{
	class	handler_wrapper : private boost::noncopyable
	{
	public:
		explicit	handler_wrapper( uint64_t delay_query_ = handler::_delay_query_ );		
		virtual	~handler_wrapper();

		bool	connect( 
			const std::string_view& dsn_, 
			const std::string_view& id_, 
			const std::string_view& auth_ );

		void	close();

		void	set_delay_time( uint64_t delay_query_ );

	private:
		handler	_handler;
		friend	class stored_procedure;
	};

	using handler_wrapper_ptr_type	=	boost::shared_ptr< handler_wrapper >;
}