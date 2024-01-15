#include <pch.h>
#include <util/quicklz/quicklz.h>

namespace	impl::util
{
	namespace	memory_pool
	{
		boost::atomic_uint64_t	count_test::count	=	0;
	}

	boost::optional< std::string >
		timestamp_micro()
	{	
		try
		{
			std::string	s( boost::posix_time::to_iso_extended_string( boost::posix_time::microsec_clock::universal_time() ) );
			if ( true == s.empty() )
				return	{};

			return	{ boost::move( s ) };
		}
		catch ( const std::exception& e_ )
		{
			_fatal_log_( boost::format( "%1% ( %2% )" ) % e_.what() % __FILE_LINE__ );
		}

		return	{};
	}

	boost::optional< std::string >
		timestamp()
	{		
		try
		{
		    std::string	s( boost::posix_time::to_iso_extended_string( boost::posix_time::second_clock::universal_time() ) );
			if ( true == s.empty() )
				return	{};

			return	{ boost::move( s ) };
		}
		catch ( const std::exception& e_ )
		{
			_fatal_log_( boost::format( "%1% ( %2% )" ) % e_.what() % __FILE_LINE__ );
		}

		return	{};
	}

	io_context::io_context( uint32_t size_ )
		:	_work( boost::asio::make_work_guard( _io ) )
	{
		run( size_ );
	}

	io_context::io_context() : _work( boost::asio::make_work_guard( _io ) )	{}

	io_context::~io_context()	{	stop();	}

	void	io_context::run( uint32_t size_ /*= boost::thread::hardware_concurrency() + 1*/ )
	{
		for ( size_t i = 0 ; i < size_ ; ++i )
			_thread_group.create_thread( boost::bind( &boost::asio::io_context::run, &_io ) );
	}

	void	io_context::stop()
	{		
		if ( true == _io.stopped() )	return;

		_io.stop();
		_work.reset();
		_thread_group.join_all();
	}

	void	io_context::post( const void_func_type& functor_ )
	{
		if ( true == functor_.empty() )	return;

		boost::asio::post( _io, functor_ );
	}

	void	io_context::dispatch( const void_func_type& functor_ )
	{
		if ( true == functor_.empty() )	return;

		boost::asio::dispatch( _io, functor_ );
	}

	void	io_context::catch_post( const void_func_type& functor_ )
	{
		if ( true == functor_.empty() )	return;

		boost::asio::post( 
			_io,
			[ = ]() -> void
			{	
				try
				{
					functor_();
				}
				catch ( const std::exception& e_ )
				{
					_fatal_log_( boost::format( "%1% ( %2% )" ) % e_.what() % __FILE_LINE__ );
				}
			} );
	}

	void	io_context::catch_dispatch( const void_func_type& functor_ )
	{
		if ( true == functor_.empty() )	return;

		boost::asio::dispatch( 
			_io,
			[ = ]() -> void
			{	
				try
				{
					functor_();
				}
				catch ( const std::exception& e_ )
				{
					_fatal_log_( boost::format( "%1% ( %2% )" ) % e_.what() % __FILE_LINE__ );
				}
			} );
	}

	bool	quicklz_compress( const std::string_view& source_, OUT std::string& dest_ )
	{
		try
		{
			qlz_state_compress	state;
			std::fill( reinterpret_cast< char* >( &state ), reinterpret_cast< char* >( &state ) + sizeof( qlz_state_compress ), 0 );

			size_t	size	=	source_.size() + 400;
			char*	p		=	boost::pool_allocator< char >::allocate( size );
			std::fill_n( p, size, 0 );

			size_t	c		=	qlz_compress( source_.data(), p, source_.size(), &state );
			dest_.assign( p, p + c );
			boost::pool_allocator< char >::deallocate( p, size );

			return true;
		}
		catch ( const std::exception& e_ )
		{
			_fatal_log_( boost::format( "%1% %2% ( %3% )" ) % source_.size() % e_.what() % __FILE_LINE__ );
		}

		return	false;
	}

	bool	quicklz_decompress( const std::string_view& source_, OUT std::string& dest_ )
	{
		try
		{
			qlz_state_decompress	state;
			std::fill( reinterpret_cast< char* >( &state ), reinterpret_cast< char* >( &state ) + sizeof( qlz_state_decompress ), 0 );

			size_t	size	=	qlz_size_decompressed( source_.data() ) + 1;
			char*	p		=	boost::pool_allocator< char >::allocate( size );
			std::fill_n( p, size, 0 );

			size_t	d		=	qlz_decompress( source_.data(), p, &state );
			dest_.assign( p, p + d );
			boost::pool_allocator< char >::deallocate( p, size );

			return	true;
		}
		catch ( const std::exception& e_ )
		{
			_fatal_log_( boost::format( "%1% %2% ( %3% )" ) % source_.size() % e_.what() % __FILE_LINE__ );
		}

		return	false;
	}

	random_generator::random_generator( uint64_t seed_ /*= 0*/ ) : _seed( seed_ )
	{ 
		seed( seed_ );
	}

	void	random_generator::seed( uint64_t seed_ /*= 0*/ )
	{
		_seed	= ( 0 == seed_ ) ? boost::chrono::high_resolution_clock::now().time_since_epoch().count() : seed_;
		_generator.seed( _seed );
	}

	void	spin_lock::lock()
	{
		time_duration	td;
		for ( unsigned k = 0 ; !try_lock() ; ++k )
		{
			boost::detail::yield( k );

			td.end();

			if ( uint64_t duration = td.milliseconds() ; 1 <= duration )
				_fatal_log_( boost::format( "%1% ( %2% )" ) % duration % __FILE_LINE__ );
		}
	}

	void	spin_lock::lock( const std::string_view& file_line_ )
	{
		time_duration	td;
		for ( unsigned k = 0 ; !try_lock() ; ++k )
		{
			boost::detail::yield( k );

			td.end();

			if ( uint64_t duration = td.milliseconds() ; 1 <= duration )
				_fatal_log_( boost::format( "%1% ( %2% )" ) % duration % file_line_ );
		}
	}

	spin_lock_guard::spin_lock_guard( spin_lock& lock_ )
		: _lock( lock_ )
	{
		_lock.lock();
	}

	spin_lock_guard::spin_lock_guard( spin_lock& lock_, const std::string_view& file_line_ )
		: _lock( lock_ )
	{
		_lock.lock( file_line_ );
	}

	spin_lock_guard::~spin_lock_guard()
	{
		unlock();
	}

	void	spin_lock_guard::unlock() 
	{ 
		_lock.unlock();
	}

	time_duration::time_duration()
	{
		start();
	}

	void	time_duration::start()
	{
		_start	=	boost::chrono::steady_clock::now();
	}

	void	time_duration::end()
	{
		_end	=	boost::chrono::steady_clock::now();
	}

	uint64_t	time_duration::seconds()		const
	{
		if ( _start > _end )
			_end	=	boost::chrono::steady_clock::now();

		return	boost::chrono::duration_cast< boost::chrono::seconds >( _end - _start ).count();
	}

	uint64_t	time_duration::milliseconds()	const
	{
		if ( _start > _end )
			_end	=	boost::chrono::steady_clock::now();

		return	boost::chrono::duration_cast< boost::chrono::milliseconds >( _end - _start ).count();
	}

	uint64_t	time_duration::microseconds()	const
	{
		if ( _start > _end )
			_end	=	boost::chrono::steady_clock::now();

		return	boost::chrono::duration_cast< boost::chrono::microseconds >( _end - _start ).count();
	}

	uint64_t	time_duration::nanoseconds()	const
	{
		if ( _start > _end )
			_end	=	boost::chrono::steady_clock::now();

		return	boost::chrono::duration_cast< boost::chrono::nanoseconds >( _end - _start ).count();
	}

	uint64_t	time_duration::to_time_since_epoch_count()
	{
		return	boost::chrono::steady_clock::now().time_since_epoch().count();
	}

	uint64_t	time_duration::seconds( uint64_t time_since_epoch_count_ )
	{
		uint64_t	now	=	to_time_since_epoch_count();
		if ( now <= time_since_epoch_count_ )	return	0;

		return	( now - time_since_epoch_count_ ) / 1000 / 1000 / 1000;
	}

	uint64_t	time_duration::milliseconds( uint64_t time_since_epoch_count_ )
	{
		uint64_t	now	=	to_time_since_epoch_count();
		if ( now <= time_since_epoch_count_ )	return	0;

		return	( now - time_since_epoch_count_ ) / 1000 / 1000;
	}

	uint64_t	time_duration::microseconds( uint64_t time_since_epoch_count_ )
	{
		uint64_t	now	=	to_time_since_epoch_count();
		if ( now <= time_since_epoch_count_ )	return	0;

		return	( now - time_since_epoch_count_ ) / 1000;
	}

	uint64_t	time_duration::nanoseconds( uint64_t time_since_epoch_count_ )
	{
		uint64_t	now	=	to_time_since_epoch_count();
		if ( now <= time_since_epoch_count_ )	return	0;

		return	now - time_since_epoch_count_;
	}

	bool	uuid_generator::gen( OUT std::string& r_ )
	{
		boost::uuids::uuid	u;

		{
#ifdef	TRACE
			_spin_lock_fl_( _gen, __FILE_LINE__ );
#else
			_spin_lock_( _gen );
#endif			
			u	=	_gen();
		}

		if ( true == u.is_nil() )		return	false;

		r_	=	boost::lexical_cast< std::string >( u );
		return	_size_ == r_.size();
	}

	bool	uuid_generator::gen( OUT boost::uuids::uuid& u_ )
	{
		{
#ifdef	TRACE
			_spin_lock_fl_( _gen, __FILE_LINE__ );
#else
			_spin_lock_( _gen );
#endif
			u_	=	_gen();
		}

		return	false == u_.is_nil();
	}

	//	hash uuid
	uint64_t	uuid_generator::gen()
	{
#ifdef	TRACE
		_spin_lock_fl_( _gen, __FILE_LINE__ );
#else
		_spin_lock_( _gen );
#endif
		return	_uuid_hasher( _gen() );
	}

	void	uuid_generator::gen_uuid( OUT std::string& r_ )
	{
		static	boost::uuids::basic_random_generator< boost::mt19937_64 >	g;
		r_	=	boost::lexical_cast< std::string >( g() );
	}

	uint64_t	uuid_generator::gen_hash_uuid()
	{
		static	boost::uuids::basic_random_generator< boost::mt19937_64 >	g;
		static	boost::hash< boost::uuids::uuid >							uuid_hasher;

		return	uuid_hasher( g() );
	}
}

namespace	impl::util::locale
{
	boost::optional< std::string >	to_utf( const std::wstring_view& multi_ )
	{
		try
		{
			return	boost::optional< std::string >( boost::locale::conv::utf_to_utf< char >( multi_.data() ) );
		}
		catch ( const std::exception& e_ )
		{
		    _fatal_log_( boost::format( "%1% ( %2% )" )  % e_.what() % __FILE_LINE__ );
		}

		return	{};
	}

	boost::optional< std::string >	to_utf( const std::string_view& multi_ )
	{
		try
		{
			return	boost::optional< std::string >( boost::locale::conv::utf_to_utf< char >( multi_.data() ) );
		}
		catch ( const std::exception& e_ )
		{
		    _fatal_log_( boost::format( "%1% ( %2% )" )  % e_.what() % __FILE_LINE__ );
		}

		return	{};
	}

	boost::optional< std::wstring >	from_utf_to_wide( const std::string& utf_ )
	{
		try
		{
			return	boost::optional< std::wstring >( boost::locale::conv::utf_to_utf< wchar_t >( utf_.data() ) );
		}
		catch ( const std::exception& e_ )
		{
		    _fatal_log_( boost::format( "%1% ( %2% )" )  % e_.what() % __FILE_LINE__ );
		}

		return	{};
	}

	boost::optional< std::string >	from_utf( const std::string& utf_ )
	{
		try
		{
			return	boost::optional< std::string >( boost::locale::conv::utf_to_utf< char >( utf_.data() ) );
		}
		catch ( const std::exception& e_ )
		{
		    _fatal_log_( boost::format( "%1% ( %2% )" )  % e_.what() % __FILE_LINE__ );
		}

		return	{};
	}
}