/*!
* \class functional.h
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
* \date 2022/2/20
*
* Contact:	muse76@hotmail.com
*           muse4116@gmail.com
*
*/

#pragma once

namespace	impl::util
{
	extern	boost::optional< std::string >	timestamp_micro();
	extern	boost::optional< std::string >	timestamp();

	template < typename type >
	static	bool	is_equal( const boost::shared_ptr< type >& _1_, const boost::shared_ptr< type >& _2_ )
	{
		if ( nullptr == _1_ || nullptr == _2_ )
		{
			_error_log_( 
				boost::format( "%1% %2% ( %3% )" ) 
					%	( nullptr == _1_ ? "nullptr" : "not nullptr" )
					%	( nullptr == _2_ ? "nullptr" : "not nullptr" )
					%	__FILE_LINE__ );

			return	false;
		}

		return	*_1_.get() == *_2_.get();
	}
}