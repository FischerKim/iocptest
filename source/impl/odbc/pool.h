/*!
* \class pool.h
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
* \date 2022/3/24
*
* Contact:	muse76@hotmail.com
*           muse4116@gmail.com
*
*/

#pragma once

namespace	impl::odbc
{
	class	pool : private boost::mutex
	{
	protected:
		enum
		{
			_default_db_pool_size_	=	2,
			_max_db_pool_size_		=	16,
		};

	protected:
		pool() : _early_pool_size( _default_db_pool_size_ )		{}

	public:
		bool	connect( 
			const std::string_view& dsn_, 
			const std::string_view& id_, 
			const std::string_view& auth_, 
			uint8_t pool_size_ = _default_db_pool_size_ );

		bool	alloc_odbc_ptr( OUT handler_wrapper_ptr_type& ptr_ );
		bool	compulsion_alloc_odbc_ptr( OUT handler_wrapper_ptr_type& ptr_ );
		void	release_odbc_ptr( const handler_wrapper_ptr_type& ptr_ );

		void	set_delay_time( uint64_t delay_query_ );

	private:
		std::deque< handler_wrapper_ptr_type >	_q;

		std::string		_dsn;
		std::string		_id;
		std::string		_auth;

		uint8_t			_early_pool_size;
	};
}