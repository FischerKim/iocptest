/*!
* \class lock_bufferd_object_pool.ipp
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
* \date 2021/1/19
*
* Contact:	muse76@hotmail.com
*			muse4116@gmail.com
*
*/

#pragma once

namespace	impl::util::memory_pool
{
	template < typename type > requires std::is_class_v< type >
	lock_bufferd_object_pool< type >::lock_bufferd_object_pool( size_t reserve_size_ /*= bufferd_object_pool< type >::_default_size_*/ )
	{
		bufferd_object_pool< type >::reserve( reserve_size_ );
	}

	template < typename type > requires std::is_class_v< type >
	lock_bufferd_object_pool< type >::~lock_bufferd_object_pool()
	{
	}

	template < typename type > requires std::is_class_v< type >
	void	lock_bufferd_object_pool< type >::reserve( size_t size_ /*= bufferd_object_pool< type >::_add_size_*/ )
	{
#ifdef	TRACE
		_spin_lock_fl_( *this, __FILE_LINE__ );
#else
		_spin_lock_( *this );
#endif

		bufferd_object_pool< type >::reserve();
	}

	template < typename type > requires std::is_class_v< type >
	bool	lock_bufferd_object_pool< type >::construct( type** p_ )
	{
#ifdef	TRACE
		_spin_lock_fl_( *this, __FILE_LINE__ );
#else
		_spin_lock_( *this );
#endif

		return	bufferd_object_pool< type >::construct( p_ );
	}

	template < typename type > requires std::is_class_v< type >
	void	lock_bufferd_object_pool< type >::destroy( type* p_ )
	{
#ifdef	TRACE
		_spin_lock_fl_( *this, __FILE_LINE__ );
#else
		_spin_lock_( *this );
#endif

		bufferd_object_pool< type >::destroy( p_ );
	}
}