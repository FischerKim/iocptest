#pragma once

//	singleton_pool_allocator 사용하도록 하자.
namespace	impl::util::memory_pool
{
	template < typename type > requires std::is_class_v< type >
	class	lock_object_pool :	protected	object_pool< type >, 
								protected	spin_lock
	{
	public:
		lock_object_pool()	=	default;

		virtual bool	construct( type** p_ )	override;
		virtual	void	destroy( type* p_ )		override;
	};
}