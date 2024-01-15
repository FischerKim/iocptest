/*!
* \class handler.hpp
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

namespace	impl::network::server
{
	template< typename session_type >
	class handler : private boost::noncopyable
	{
	private:
		enum
		{
#ifdef _DEBUG
			_keep_alive_time_		=	600000,		/* 600 sec */
#else
			_keep_alive_time_		=	300000,		/* 300 sec */
#endif
		};

	protected:
		using	session_ptr_type	=	boost::shared_ptr< session_type >;

		//	functions
	public:
		handler( boost::asio::io_context& io_ );
		virtual	~handler();

		virtual void	on_start();
		virtual void	on_stop();

		virtual	bool	on_user_enter( const session_ptr_type& session_ );
		virtual	bool	on_user_leave( const session_ptr_type& session_ );

		virtual bool	on_route( 
			const session_ptr_type& session_, 
			const packet::inbound_ptr_type& in_ )	=	0;

	public:
		boost::asio::io_context&	
			get_io_context();

		size_t	get_user_size()		const;

	protected:
		bool	is_session( const session_ptr_type& session_ )	const;

		boost::optional< std::set< session_ptr_type > >
			all_sessions()	const;

	protected:
		class	lock_user_set : public std::set< session_ptr_type >, public boost::shared_mutex	{};
		mutable	lock_user_set	_user_set;

	private:
		boost::asio::io_context&			_io;
		util::timer::repeat_task_ptr_type	_keep_alive_task;
	};
}