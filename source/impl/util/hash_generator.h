/*!
* \class hash_generator.h
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

namespace	impl::util
{
	struct    hash_generator
	{
		template < typename type > requires	( std::is_integral_v< type > && 4 >= sizeof( type ) )
		static void         hash_combine( INOUT uint64_t& seed_, const type& value_ )
		{
			boost::hash< type > hs;
			seed_	^=	hs( value_ ) + 0x9e3779b9 + ( seed_ << 6 ) + ( seed_ >> 2 ); //	TEA( tiny encryption algorithm )
		}

		template< typename type > requires	( std::is_integral_v< type > && 4 >= sizeof( type ) )
		static uint64_t    make_hash_key( const std::vector< type >& vec_ )
		{
			uint64_t	seed	=	0;
			hash_combine( seed, boost::chrono::high_resolution_clock::now().time_since_epoch().count() );

 			for ( const type& r_ : vec_ )
 				hash_combine( seed, r_ );

			return   seed;
		}
	};
}