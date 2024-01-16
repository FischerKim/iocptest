#pragma once

namespace	impl::util
{
	class	scope_exit_call final : private boost::noncopyable
	{
	public:
		scope_exit_call( const void_func_type& f_ ) : _f( f_ )	{}
		~scope_exit_call()
		{
			if ( true == _f.empty() )	return;

			_f();
		}

	private:
		void_func_type	_f;
	};
}