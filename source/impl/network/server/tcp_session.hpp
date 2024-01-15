/*!
* \class tcp_session.hpp
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

namespace	impl::network::server
{
	template< typename session_type, typename handler_type >
	class	tcp_session 
		:	public	session_type, 
			public	primitive_session,
			public	boost::enable_shared_from_this< tcp_session< session_type, handler_type > >
	{
	public:
		tcp_session( handler_type& handler_ );
		virtual	~tcp_session();

		void			start();

		virtual void	on_close()	override;

	private:
		void	read_header( 
			const boost::system::error_code& ec_, 
			size_t bytes_transferred_ );

		void	read_body( 
			const boost::system::error_code& ec_, 
			size_t bytes_transferred_ );

		void	route( const packet::inbound_ptr_type& in_ );
		void	route_poll();

		void	write( 
			const boost::system::error_code& ec_, 
			size_t bytes_transferred_ );

	public:
		using socket_type	=	boost::asio::ip::tcp::socket;
		socket_type&	tcp_socket()	{	return	_socket;	}

		virtual const std::string&	on_get_remote_ip()		const	override;
		virtual uint16_t			on_get_remote_port()	const	override;

		virtual bool	on_is_connected()			const	override;
		virtual bool	on_is_keep_alive_flag()		const	override;
		virtual void	on_init_keep_alive_flag()	override;
		virtual void	on_set_keep_alive_flag()	override;

	protected:
		virtual boost::optional< packet::outbound_ptr_type >
			on_alloc_outbound() override;

		virtual bool	on_send( const packet::outbound_ptr_type& out_ )		override;
		virtual bool	on_fast_send( const packet::outbound_ptr_type& out_ )	override;

		virtual void	on_post( const void_func_type& f_ )		override;
		virtual void	on_dispatch( const void_func_type& f_ )	override;

	private:
		socket_type					_socket;
		handler_type&				_handler;

		packet::inbound_ptr_type	_inbound;
	};
}