#pragma once

namespace	framework::net
{
	template < typename inheritance_type >
	class	tcp_handler : public	base_handler,
		public	boost::enable_shared_from_this< inheritance_type >
	{
	public:
		tcp_handler(boost::asio::io_context& io_);
		virtual		~tcp_handler();

		virtual bool	on_connect(
			const std::string_view& ip_,
			uint16_t port_)	override;

		virtual	void	on_reconnect()	override;
		virtual void	on_close()	override;

	protected:
		virtual	bool	on_send(const outbound_ptr_type& out_)	override;

	private:
		using	session_ptr_type = boost::shared_ptr< tcp_session >;
		using	atomic_session_ptr_type = boost::atomic_shared_ptr< tcp_session >;
		atomic_session_ptr_type		_session;
	};
}