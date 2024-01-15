/*!
* \class bufferd_object_pool.ipp
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
	bufferd_object_pool< type >::bufferd_object_pool( size_t reserve_size_ /*= _default_size_*/ )
	{
		reserve( reserve_size_ );
	}

	template < typename type > requires std::is_class_v< type >
	bufferd_object_pool< type >::~bufferd_object_pool()
	{
		std::for_each( 
			std::deque< type* >::begin(), 
			std::deque< type* >::end(), 
			[ this ]( type* p_ ) -> void 
			{ 
				destroy( p_ ); 
			} );
	}

	template < typename type > requires std::is_class_v< type >
	void	bufferd_object_pool< type >::reserve( size_t size_ /*= _add_size_*/ )
	{
		size_	=	( 0 == size_ || _max_size_ < size_ ) ? _add_size_ : size_;

		type*	p	=	nullptr;
		for ( size_t i = 0 ; i < size_ ; ++i )
		{
			if ( false == object_pool< type >::construct( &p ) )
			{
				_fatal_log_( 
					boost::format( "%1% %2% ( %3% )" ) 
						%	i 
						%	size_ 
						%	__FILE_LINE__ );
				continue;
			}

			std::deque< type* >::emplace_back( p );
		}
	}

	template < typename type > requires std::is_class_v< type >
	bool	bufferd_object_pool< type >::construct( type** p_ )
	{
		if ( true == std::deque< type* >::empty() )
		{
			type*	p	=	nullptr;
			for ( size_t i = 0 ; i < _add_size_ ; ++i )
			{
				if ( false == object_pool< type >::construct( &p ) )
				{
					_fatal_log_( 
						boost::format( "%1% ( %2% )" ) 
							%	i 
							%	__FILE_LINE__ );
					continue;
				}

				if ( _add_size_ - 1 == i )
				{
					*p_	=	p;
					return	true;
				}

				std::deque< type* >::emplace_back( p );
			}
		}

		*p_	=	std::deque< type* >::front();
		std::deque< type* >::pop_front();
		return	nullptr != *p_;
	}

	template < typename type > requires std::is_class_v< type >
	void	bufferd_object_pool< type >::destroy( type* p_ )
	{
		if ( _max_size_ <= std::deque< type* >::size() )
		{
			object_pool< type >::destroy( p_ );
			return;
		}

		std::deque< type* >::emplace_back( p_ );
	}
}