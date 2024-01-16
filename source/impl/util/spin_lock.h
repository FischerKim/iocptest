#pragma once

//	락처리는 해야 하지만 사용량이 극히 적을 때 사용하도록 하자.
namespace	impl::util
{
	class spin_lock_guard;

	class spin_lock : public boost::detail::spinlock
	{
	public:
		void	lock();
		void	lock( const std::string_view& file_line_ );

	private:
		friend	class spin_lock_guard;
	};

	class spin_lock_guard : private boost::noncopyable
	{
	public:
		explicit	spin_lock_guard( spin_lock& lock_ );
		explicit	spin_lock_guard( spin_lock& lock_, const std::string_view& file_line_ );

		~spin_lock_guard();

		void	unlock();

	private:
		spin_lock&	_lock;
	};
}

#define	_spin_lock_( _lock_ )					impl::util::spin_lock_guard		guard( _lock_ )
#define _spin_lock_ex_( _v_, _lock_ )			impl::util::spin_lock_guard		_v_( _lock_ )

#define	_spin_lock_fl_( _lock_, _fl_ )			impl::util::spin_lock_guard		guard( _lock_, _fl_ )
#define _spin_lock_fl_ex_( _v_, _lock_, _fl_ )	impl::util::spin_lock_guard		_v_( _lock_, _fl_ )

#define	_const_cast_spin_lock_( _lock_ )		( *const_cast< impl::util::spin_lock* >( &_lock_ ) )