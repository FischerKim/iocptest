#pragma once

class session_manager : private boost::noncopyable
{
private:
	session_manager() = default;

public:
	static	session_manager* get_instance();

	size_t	count()	const;

	boost::optional< session_ptr_type >
		lookup(uint64_t compid_)	const;

	bool	is_exist(uint64_t compid_) const;

	bool	session_enter(const session_ptr_type& p_);

	void	session_leave(uint64_t compid_);

	boost::optional< std::vector< session_ptr_type > >
		in_range_sessions(const session_ptr_type& source_)	const;

	boost::optional< std::vector< session_ptr_type > >
		users()	const;

private:
	class lock_map : public std::unordered_map< uint64_t, session_ptr_type >, public boost::shared_mutex {};
	mutable lock_map	_map;
};

static session_manager* afx_player_manager()
{
	return	session_manager::get_instance();
}