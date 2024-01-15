/*!
* \class random_generator.ipp
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

namespace	impl::util
{
	template< typename type >
	type	random_generator::rand()
	{
		return	rand< type >( 0, 99 );
	}

	template< typename type >
	type	random_generator::rand( type low_, type hi_ )
	{
		if ( low_ >= hi_ )		return	low_;

		boost::uniform_int< type >	dist( low_, hi_ );
		return	boost::variate_generator< boost::mt19937_64&, boost::uniform_int< type > >( _generator, dist )();
	}

	template< typename type >
	type	random_generator::real_rand( type low_, type hi_ )
	{
		if ( low_ >= hi_ )		return	low_;

		boost::uniform_real< type >	dist( low_, hi_ );
		return	boost::variate_generator< boost::mt19937_64&, boost::uniform_real< type > >( _generator, dist )();
	}
}