/*!
* \class bufferd_object_pool.hpp
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

//	singleton_pool_allocator 사용하도록 하자.
namespace	impl::util::memory_pool
{
	template < typename type > requires std::is_class_v< type >
	class bufferd_object_pool : private object_pool< type >, 
								private	std::deque< type* >
	{
	public:
		enum
		{
			_default_size_	=	0x400,
			_add_size_		=	_default_size_,
			_max_size_		=	0xffffffff,
		};

	public:
		explicit	bufferd_object_pool( size_t reserve_size_ = _default_size_ );
		virtual	~bufferd_object_pool();

		void	reserve( size_t size_ = _add_size_ );

		virtual bool	construct( type** p_ )	override;
		virtual void	destroy( type* p_ )		override;
	};
}