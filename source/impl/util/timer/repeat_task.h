/*!
* \class repeat_task.h
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
	class	repeat_task final	:	private	boost::noncopyable,
									public	boost::enable_shared_from_this< repeat_task >
	{
	public:
		enum
		{
			_default_time_	=	1000,	//	ms
			_min_time_		=	10,
		};

	public:
		repeat_task( boost::asio::io_context& io_ );
		~repeat_task();

		//	return true == call repeat, false == end
		bool	start( size_t expire_ /* ms */, const bool_func_type& func_ );

		void	stop();

		bool	is_running()	const;

	private:
		void	handler( const boost::system::error_code& error_ );

	private:
		boost::posix_time::time_duration	_interval;
		boost::asio::deadline_timer			_timer;

		mutable	spin_lock					_lock;
		bool_func_type						_functor;
	};

	using	repeat_task_ptr_type	=	boost::shared_ptr< repeat_task >;
}