#pragma once

//	내부적으로 쌍방향 리스트 사용이라 개수가 적을 땐 상관없는데 많아지면 해제할 시 느려짐.
//	singleton_pool_allocator 사용하도록 하자.
namespace	impl::util::memory_pool
{
	template < typename type > requires std::is_class_v< type >
	class	object_pool	:	private	boost::noncopyable, 
							private	boost::object_pool< type >
	{
	public:
		object_pool()	=	default;

		virtual bool	construct( type** p_ );
		virtual void	destroy( type* p_ );
	};
}