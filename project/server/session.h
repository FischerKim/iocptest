#pragma once
class	session : public impl::network::server::interface_session,
	public boost::shared_mutex
{
private:
	using inbound_ptr_type = impl::network::packet::inbound_ptr_type;

public:
	session();
	virtual ~session();

	virtual	void	on_complete_connect();

	virtual bool	on_route(const inbound_ptr_type& in_);

	bool			set(
		uint64_t compid_);

	boost::optional< boost::tuple< uint64_t, std::string, std::string > >
		get()	const;

	uint64_t		compid()		const { return	_compid; }

private:
	session& self()	const { return	*const_cast<session*>(this); }

private:
	boost::atomic_uint64_t	_compid = 0;

public:
	boost::atomic_bool		is_joined = false;
	//sSymbol Sym[10000] = {};

//private:
	//CCSAuth3								_csa;

};

using session_ptr_type = boost::shared_ptr< session >;