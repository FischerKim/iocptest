/*!
* \class lock_object_pool.hpp
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
	class	lock_object_pool :	protected	object_pool< type >, 
								protected	spin_lock
	{
	public:
		lock_object_pool()	=	default;

		virtual bool	construct( type** p_ )	override;
		virtual	void	destroy( type* p_ )		override;
	};
}