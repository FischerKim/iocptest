/*!
* \class handler_wrapper.h
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
* \date 2020/2/11
*
* Contact:	muse76@hotmail.com
*			muse4116@gmail.com
*
*/

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