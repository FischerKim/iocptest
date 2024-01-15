/*!
* \class wss_session.hpp
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
	class	wss_session 
		:	public	session_type, 
			public	primitive_session,
			public	boost::enable_shared_from_this< wss_session< session_type, handler_type > >
	{
	public:
		wss_session( 
			boost::asio::ssl::context& context_, 
			handler_type& handler_ );

		virtual 	~wss_session();

		void			start();

		virtual void	on_close()	override;

	private:
		//	certificate handshake
		void	handshake( const boost::system::error_code& ec_ );

		//	websocket protocol handshake
		void	websocket_handshake_read( 
			const boost::system::error_code& ec_, 
			size_t bytes_transferred_ );

		void	websocket_handshake_write( 
			const packet::outbound_ptr_type& out_, 
			const boost::system::error_code& ec_, 
			size_t bytes_transferred_ );

		//	websocket protocol header parse
		void	read_header( 
			const boost::system::error_code& ec_, 
			size_t bytes_transferred_ );

		void	read_header_126( 
			uint8_t fin_rsv_opcode_, 
			const boost::system::error_code& ec_, 
			size_t bytes_transferred_ );

		void	read_header_127( 
			uint8_t fin_rsv_opcode_, 
			const boost::system::error_code& ec_, 
			size_t bytes_transferred_ );

		void	read( 
			size_t length_, 
			uint8_t fin_rsv_opcode_, 
			const boost::system::error_code& ec_, 
			size_t bytes_transferred_ );

		void	pong( uint8_t fin_rsv_opcode_ );

		void	route( const packet::inbound_ptr_type& in_ );
		void	route_poll();

		void	write( 
			const boost::system::error_code& ec_, 
			size_t bytes_transferred_ );

	private:
		using	socket_ssl_type	=	boost::asio::ssl::stream< boost::asio::ip::tcp::socket >;
		socket_ssl_type&	ssl_stream_socket()	{	return	_socket;	}

	public:
		socket_ssl_type::lowest_layer_type&		tcp_socket()	{	return	_socket.lowest_layer();	}

		virtual const std::string&	on_get_remote_ip()		const	override;
		virtual uint16_t			on_get_remote_port()	const	override;

		virtual bool	on_is_connected()			const	override;
		virtual bool	on_is_keep_alive_flag()		const	override;
		virtual void	on_init_keep_alive_flag()	override;
		virtual void	on_set_keep_alive_flag()	override;

	protected:
		virtual boost::optional< packet::outbound_ptr_type >
			on_alloc_outbound() override;

		virtual bool	on_send( const packet::outbound_ptr_type& out_ )	override;

		virtual void	on_post( const void_func_type& f_ )		override;
		virtual void	on_dispatch( const void_func_type& f_ )	override;
		virtual	e_protocol_type
			on_protocol_type()	const	override;

	private:
		socket_ssl_type			_socket;
		handler_type&			_handler;

		boost::asio::streambuf	_inbound_ws_stream;
		websocket::header		_ws_header;
	};
}