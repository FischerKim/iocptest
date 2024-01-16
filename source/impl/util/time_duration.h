#pragma once

namespace	impl::util
{
	class	time_duration final : private boost::noncopyable
	{
	public:
		time_duration();

		void		start();
		void		end();

		uint64_t	seconds()		const;
		uint64_t	milliseconds()	const;
		uint64_t	microseconds()	const;
		uint64_t	nanoseconds()	const;

		//	utc
		static	uint64_t	to_time_since_epoch_count();
		static	uint64_t	seconds( uint64_t time_since_epoch_count_ );
		static	uint64_t	milliseconds( uint64_t time_since_epoch_count_ );
		static	uint64_t	microseconds( uint64_t time_since_epoch_count_ );
		static	uint64_t	nanoseconds( uint64_t time_since_epoch_count_ );

	private:
		boost::chrono::steady_clock::time_point				_start;
		mutable	boost::chrono::steady_clock::time_point		_end;
	};
}