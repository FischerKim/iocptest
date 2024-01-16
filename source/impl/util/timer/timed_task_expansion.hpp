#pragma once

namespace	impl::util::timer
{
	template < typename key > requires ( std::is_integral_v< key > && 4 >= sizeof( key ) )
	class	timed_task_expansion final : public timed_task
	{
	public:
		timed_task_expansion( boost::asio::io_context& io_ ) 
			:	timed_task( io_ )	
		{
		}

		virtual ~timed_task_expansion()
		{
			this->cancel();
		}

		bool	invoke( const key& key_, size_t expire_ /* ms */, const void_func_type& func_ );

		void	cancel( const key& key_ );

		virtual void	cancel()	final;

	private:
		class lock_map_type : public std::unordered_map< key, timed_task::timer_shared_ptr_type >, public util::spin_lock	{};
		lock_map_type	_map;
	};
}