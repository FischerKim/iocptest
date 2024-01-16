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