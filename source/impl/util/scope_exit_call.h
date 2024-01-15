/*!
* \class uuid_generator.h
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