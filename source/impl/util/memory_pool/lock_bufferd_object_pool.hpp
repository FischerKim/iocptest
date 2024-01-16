#pragma once

//	singleton_pool_allocator 사용하도록 하자.
namespace	impl::util::memory_pool
{
	template < typename type > requires std::is_class_v< type >
	class lock_bufferd_object_pool final :	private	bufferd_object_pool< type >,
											private spin_lock
	{
	public:
		explicit	lock_bufferd_object_pool( size_t reserve_size_ = bufferd_object_pool< type >::_default_size_ );
		virtual		~lock_bufferd_object_pool();

		void	reserve( size_t size_ = bufferd_object_pool< type >::_add_size_ );

		virtual bool	construct( type** p_ )	final;
		virtual void	destroy( type* p_ )		final;
	};
}