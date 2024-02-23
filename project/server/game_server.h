#pragma once

class server final : public common::base_server_instance
{
private:
	server();

public:
	static	server* get_instance();

private:
	virtual bool	on_listen()		final;
	virtual bool	on_start() 		final;
	virtual	void	on_stop()		final;
	virtual void	on_destroy()	final;
	virtual void	on_timer()		final;

public:
	handler& get_handler() { return	_handler; }

private:
	handler			_handler;

	using	tcp_acceptor =
		impl::network::server::acceptor
		<
		handler,
		impl::network::server::tcp_session
		<
		session,
		handler
		>
		>;

	tcp_acceptor	_acceptor;

};

static	server* afx_server()
{
	return	server::get_instance();
}