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