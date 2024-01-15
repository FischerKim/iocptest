/*!
* \class random_generator.hpp
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
	class random_generator : private boost::noncopyable
	{
	public:
		explicit	random_generator( uint64_t seed_ = 0 );

		void		seed( uint64_t seed_ = 0 );

		template< typename type >
		type		rand();

		template< typename type >
		type		rand( type low_, type hi_ );

		template< typename type >
		type		real_rand( type low_, type hi_ );

		uint64_t	get_seed()	const	{	return	_seed;	}

	private:
		boost::mt19937_64	_generator;
		uint64_t			_seed;
	};
}