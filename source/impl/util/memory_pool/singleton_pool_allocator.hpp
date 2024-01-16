#pragma once

//	sample
//	{
//		struct  test : public impl::util::memory_pool::singleton_pool_allocator< test >
//		{
//			std::string                 val_1;
//			int                         val_2;
//			std::array< char, 1024 >    val_3;
//		};
// 
//		std::vector< test* >    v;
// 
//		impl::util::time_duration   td;
// 
//		v.reserve( 100000 );
// 
//		td.start();
// 
//		for ( size_t i = 0 ; i < 100000 ; ++i )
//			v.emplace_back( new test );
// 
//		std::cout << td.microseconds() << std::endl;
//	}

namespace	impl::util::memory_pool
{
	class	count_test
	{
	public:
		static boost::atomic_uint64_t	count;
	};

	//	락없이 사용 할 때는 lock_type에 boost::details::pool::null_mutex 선언
	template < typename type, typename lock_type = boost::details::pool::default_mutex >
	class	singleton_pool_allocator
	{
	public:
		static	void*	operator new( size_t )
		{
			count_test::count++;
			return	boost::singleton_pool< type, sizeof( type ), boost::default_user_allocator_new_delete, lock_type >::malloc();
		}

		static	void	operator delete( void* p_ )
		{
			count_test::count--;
			boost::singleton_pool< type, sizeof( type ), boost::default_user_allocator_new_delete, lock_type >::free( p_ );
		}

		static	void	release_memory()
		{
			boost::singleton_pool< type, sizeof( type ) >::release_memory();
		}

		static	void	purge_memory()
		{
			boost::singleton_pool< type, sizeof( type ) >::purge_memory();
		}

		[ [ deprecated ( "not supported by new []" ) ] ]
		static	void*	operator new[] ( size_t size_ )
		{
			return	nullptr;
		}

		[ [ deprecated ( "not supported by delete []" ) ] ]
		static	void	operator delete[] ( void* p_ )
		{
		}
	};
}