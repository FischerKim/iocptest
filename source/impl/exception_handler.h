/*!
* \class exception_handler.h
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

namespace	impl
{
	class exception_handler final : boost::noncopyable
	{
	private:
		exception_handler();

	public:
		~exception_handler();

		static	exception_handler*	get_instance();

		void	begin_dump( const std::string_view& dir_ );

		const std::string&	get_directory()	const	{	return	_dir;	}

	private:
		void	end_dump();

		std::string	_dir;
	};
}