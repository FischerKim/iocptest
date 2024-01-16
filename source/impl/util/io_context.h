#pragma once

namespace	impl::util
{
	class io_context final : private boost::noncopyable
	{	
	public:
		explicit	io_context( uint32_t size_ );

		io_context();

		~io_context();

	public:
		void						run( uint32_t size_ = boost::thread::hardware_concurrency() + 1 );
		void						stop();
		void						post( const void_func_type& functor_ );
		void						dispatch( const void_func_type& functor_ );

		//	logger 셋팅 후 사용
		void						catch_post( const void_func_type& functor_ );

		//	logger 셋팅 후 사용
		void						catch_dispatch( const void_func_type& functor_ );

		boost::asio::io_context&	get_io_context()			{	return	_io;					}

		size_t						get_thread_size()	const	{	return	_thread_group.size();	}	

	private:
		boost::asio::io_context		_io;

		using	work_type	=	boost::asio::executor_work_guard< boost::asio::io_context::executor_type >;
		work_type					_work;
		boost::thread_group			_thread_group;
	};
}