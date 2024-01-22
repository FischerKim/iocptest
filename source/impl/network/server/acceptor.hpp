#pragma once

namespace	impl::network::server
{
	template< typename handler_type, typename session_type >
	class acceptor : private boost::noncopyable
	{
	private:
		using	session_ptr_type	=	boost::shared_ptr< session_type >;

	protected:
		using	tcp_acceptor		=	boost::asio::ip::tcp::acceptor;

	public:
		acceptor( handler_type& server_handler_ );
		virtual ~acceptor();

		bool		listen_start( uint16_t listen_port_ );
		void		stop();

		uint16_t	get_listen_port()	const;
		std::string	get_host_ip()		const;

	protected:
		boost::asio::io_context&	
			get_io_context();

	private:
		void	accept( 
			const session_ptr_type& session_, 
			const boost::system::error_code& error_ );

	protected:	
		uint16_t			_listen_port;
		tcp_acceptor		_acceptor;
		handler_type&		_server_handler;
		boost::atomic_bool	_is_shutdown;
	};
}