/*!
* \class object_pool.ipp
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
	bool	object_pool< type >::construct( type** p_ )
	{
		try
		{
			*p_	=	boost::object_pool< type >::construct();
		}
		catch ( const std::exception& e_ )
		{
			_fatal_log_( boost::format( "%1% (%2%)" ) % e_.what() % __FILE_LINE__ );
			*p_	=	nullptr;
		}

		return	nullptr != *p_;
	}

	template < typename type > requires std::is_class_v< type >
	void	object_pool< type >::destroy( type* p_ )
	{
		if ( nullptr == p_ || false == boost::object_pool< type >::is_from( p_ ) )	return;

		boost::object_pool< type >::destroy( p_ );
	}
}