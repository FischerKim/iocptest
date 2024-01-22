#pragma once

namespace impl::network::server
{
	class interface_session : private boost::noncopyable
	{
	public:
		interface_session()				{}
		virtual ~interface_session()	{}

		virtual void	on_close()				=	0;

		virtual bool	on_route( const packet::inbound_ptr_type& in_ )	=	0;

		virtual const std::string&	
			on_get_remote_ip()		const	=	0;

		virtual uint16_t
			on_get_remote_port()	const	=	0;

		virtual bool	on_is_connected()			const	=	0;
		virtual bool	on_is_keep_alive_flag()		const	=	0;
		virtual void	on_init_keep_alive_flag()			=	0;
		virtual void	on_set_keep_alive_flag()			=	0;

	public:
		bool	send(
			uint16_t packet_id_,
			const void* body_,
			size_t size_ );

		bool	fast_send(
			uint16_t packet_id_,
			const void* body_,
			size_t size_ );

	protected:
		virtual boost::optional< packet::outbound_ptr_type >
			on_alloc_outbound()	=	0;

		virtual bool	on_send( const packet::outbound_ptr_type& out_ )		=	0;
		virtual bool	on_fast_send( const packet::outbound_ptr_type& out_ )	=	0;

		virtual void	on_post( const void_func_type& f_ )	=	0;
		virtual void	on_dispatch( const void_func_type& f_ )	=	0;
	};
}