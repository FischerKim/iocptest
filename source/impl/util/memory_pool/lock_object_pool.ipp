#pragma once

namespace	impl::util::memory_pool
{
	template < typename type > requires std::is_class_v< type >
	bool	lock_object_pool< type >::construct( type** p_ )
	{
#ifdef	TRACE
		_spin_lock_fl_( *this, __FILE_LINE__ );
#else
		_spin_lock_( *this );
#endif

		return	object_pool< type >::construct( p_ );
	}

	template < typename type > requires std::is_class_v< type >
	void	lock_object_pool< type >::destroy( type* p_ )
	{
#ifdef	TRACE
		_spin_lock_fl_( *this, __FILE_LINE__ );
#else
		_spin_lock_( *this );
#endif

		object_pool< type >::destroy( p_ );
	}
}