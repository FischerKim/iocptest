/*!
* \class timed_task.h
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

namespace	impl::util::timer
{
	class	timed_task	:	private	boost::noncopyable,
							public	boost::enable_shared_from_this< timed_task >
	{
	protected:
		using	timer_shared_ptr_type	=	boost::shared_ptr< boost::asio::deadline_timer >;
		using	timer_weak_ptr_type		=	boost::weak_ptr< boost::asio::deadline_timer >;

	public:
		timed_task( boost::asio::io_context& io_ );
		virtual	~timed_task();
			
		//	call once
		bool	invoke( size_t expire_ /* ms */, const void_func_type& func_ );

		virtual void	cancel();

		//	not support cancel
		static	bool	invoke( boost::asio::io_context& io_, size_t expire_ /* ms */, const void_func_type& func_ );
			
	protected:
		boost::asio::io_context&	_io;
			
	private:
		class lock_set_type : public std::unordered_set< timer_shared_ptr_type >, public util::spin_lock	{};
		lock_set_type				_set;
	};

	using	timed_task_ptr_type	=	boost::shared_ptr< timed_task >;
}