#pragma once

class	trader final : public framework::net::tcp_client< trader >
{
private:
	enum
	{
		_30sec_ = 30000,
		_200ms_ = 200,
		_1sec_ = 1000,
	};

public:
	trader(
		boost::asio::io_context& io_, uint64_t compid);

	virtual	~trader();

	void login();
	void access_server();

	bool simulate();

protected:
	virtual	void	on_complete_connect()	override;


	using	inbound_ptr_type = framework::net::inbound_ptr_type;

	virtual bool	on_route(const inbound_ptr_type& in_)	override;

private:
	using repeat_task_ptr_type = impl::util::timer::repeat_task_ptr_type;
	repeat_task_ptr_type	_30sec_task;
	repeat_task_ptr_type	_200ms_task;
	repeat_task_ptr_type	_1sec_task;
	int64_t					_compid = 0;
	sSymbol Sym[10000] = {};
	std::unordered_map<UINT64, sSymbol> m_other_vectors;

	boost::atomic_bool		initialized = false;
	boost::atomic_bool		loggedin = false;

	class	lock_random_generator : public	impl::util::random_generator, public boost::mutex {};
	lock_random_generator	_gen;
};

using	trader_ptr = boost::shared_ptr< trader >;