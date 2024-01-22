#pragma once

namespace	impl::odbc
{
	class	fetch_buffer final : private boost::noncopyable
	{
	public:
		fetch_buffer();
		virtual	~fetch_buffer();

		void	init();
		bool	alloc_buf( size_t size_ );

		size_t	size()	const	{	return	_size;	}

	private:
		SQLULEN		_column_size;
		SQLLEN		_out_len;
		char*		_buf;
		__int64		_value_buf;
		double		_actual_buf;	
		size_t		_size;

		friend	class	handler;
	};

	using	fetch_buffer_ptr_type	=	boost::shared_ptr< fetch_buffer >;
}