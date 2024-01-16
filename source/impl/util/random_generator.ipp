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