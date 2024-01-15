#pragma once

class	handler : public impl::network::server::handler< session >
{
private:
	enum
	{
#ifdef _DEBUG
		_1_sec_ = 1000 * 10,
#else
		_1_sec_ = 1000,
#endif
		_60_sec_ = 60000,

		_game_guard_check_time = 1000 * 60,	// 1min

		_default_connect_size_ = 5000,
	};

protected:
	using	session_ptr_type = impl::network::server::handler< session >::session_ptr_type;
	using	inbound_ptr_type = impl::network::packet::inbound_ptr_type;

public:
	explicit	handler(
		impl::util::io_context& io_);
	virtual ~handler();

	virtual void	on_start()	override;

	virtual void	on_stop()	override;

	virtual	bool	on_user_enter(const session_ptr_type& session_)	override;
	virtual	bool	on_user_leave(const session_ptr_type& session_)	override;

	virtual bool	on_route(
		const session_ptr_type& session_,
		const inbound_ptr_type& in_)	override;

private:
	bool	dispatch_CS_HEART_BEAT(
		const session_ptr_type& session_,
		const CS_HEART_BEAT& pk_);

	bool	dispatch_CS_TOTAL_USERCOUNT(
		const session_ptr_type& session_,
		const CS_TOTAL_USERCOUNT& pk_);

	bool	dispatch_CS_SERVER_LOGIN(
		const session_ptr_type& session_,
		const CS_SERVER_LOGIN& pk_);

	bool	dispatch_CS_SERVER_ACCESS(
		const session_ptr_type& session_,
		const CS_SERVER_ACCESS& pk_);

	bool	dispatch_CS_USER_LOOKUP(
		const session_ptr_type& session_,
		const CS_USER_LOOKUP& pk_);

	bool	dispatch_CS_MARKET_DATA(
		const session_ptr_type& session_,
		const CS_MARKET_DATA& pk_);


protected:
	virtual bool	on_complete_connect(
		const session_ptr_type& session_);

	void			post(const void_func_type& func_) { _task_io.post(func_); }
	void			dispatch(const void_func_type& func_) { _task_io.dispatch(func_); }
	void			catch_post(const void_func_type& func_) { _task_io.catch_post(func_); }
	void			catch_dispatch(const void_func_type& func_) { _task_io.catch_dispatch(func_); }

public:
	void			max_connect_size(uint16_t max_connect_size_) { _max_connect_size = max_connect_size_; }
	uint16_t		max_connect_size()	const { return	_max_connect_size; }

	void			shutdown() { _is_shutdown = true; }
	bool			is_shutdown()	const { return	_is_shutdown; }

private:
	impl::util::io_context& _task_io;
	impl::util::io_context& _db_io;

	using repeat_task_ptr_type = impl::util::timer::repeat_task_ptr_type;
	repeat_task_ptr_type		_60sec_task;
	repeat_task_ptr_type		_1sec_task;

	uint16_t					_max_connect_size = _default_connect_size_;
	boost::atomic_bool			_is_shutdown = false;
};