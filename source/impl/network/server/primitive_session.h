/*!
* \class primitive_sessi
.h
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

namespace impl::network::server
{
	class	primitive_session : private boost::noncopyable
	{
	protected:
		enum
		{
			_default_pool_size_	=	4,
			_max_pool_size_		=	8,
		};

	public:
		explicit	primitive_session( 
			boost::asio::io_context& io_, 
			size_t default_pool_size_ = _default_pool_size_ );

		virtual ~primitive_session();
				
		bool				is_connected()			const		{	return	_is_connected;				}

		void				set_keep_alive_flag()				{	_keep_alive_flag	=	true;		}
		void				init_keep_alive_flag()				{	_keep_alive_flag	=	false;		}
		bool				is_keep_alive_flag()	const		{	return	_keep_alive_flag;			}

		const std::string&	get_remote_ip()			const		{	return	_socket_ip;					}
		uint16_t			get_remote_port()		const		{	return	_socket_port;				}

	protected:
		void	set_ip( const std::string_view& socket_ip_ )
		{
			_socket_ip.assign( socket_ip_.data(), socket_ip_.size() );
		}

		void	set_port( uint16_t socket_port_ )
		{	
			_socket_port	=	socket_port_;
		}

		void	set_connected()		{	_is_connected	=	true;			}
		void	set_closed()		{	_is_connected	=	false;			}

		boost::optional< packet::outbound_ptr_type >	
			alloc_outbound();

		void	dealloc_outbound( const packet::outbound_ptr_type& p_ );

		boost::optional< packet::inbound_ptr_type >		
			alloc_inbound();

		void	dealloc_inbound( const packet::inbound_ptr_type& p_ );

		void	post( const void_func_type& f_ );
		void	dispatch( const void_func_type& f_ );

	protected:
		using	strand		=	boost::asio::strand< boost::asio::io_context::executor_type >;
		strand					_strand;

		using	outbound_q	=	std::deque< packet::outbound_ptr_type >;
		outbound_q				_outbound_q;

		using	inbound_q	=	std::deque< packet::inbound_ptr_type >;
		inbound_q				_inbound_q;

	private:
		std::string				_socket_ip;
		uint16_t				_socket_port;

		boost::atomic_bool		_keep_alive_flag;

		boost::atomic_bool		_is_connected;
		
		using	inbound_deque	=	std::deque< packet::inbound_ptr_type >;
		inbound_deque			_inbound_pool;

		class	lock_outbound_deque : public std::deque< packet::outbound_ptr_type >, public boost::mutex	{};
		lock_outbound_deque		_outbound_pool;
	};
}