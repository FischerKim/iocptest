/*!
* \class ssl_acceptor.hpp
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
	template< typename handler_type, typename session_type >
	class ssl_acceptor : public acceptor< handler_type, session_type >
	{
	private:
		using	session_ptr_type	=	boost::shared_ptr< session_type >;

	public:
		explicit	 ssl_acceptor( 
			context& ssl_context_, 
			handler_type& server_handler_ );

		virtual ~ssl_acceptor()	{}

		bool	listen_start( uint16_t listen_port_ );

	private:
		void	accept( 
			const session_ptr_type& session_, 
			const boost::system::error_code& error_ );

	private:
		context&	_context;
	};
}