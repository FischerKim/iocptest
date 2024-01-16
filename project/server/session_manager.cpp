#include <pch.h>

session_manager* session_manager::get_instance()
{
	static	boost::movelib::unique_ptr< session_manager >	inst;
	if (nullptr == inst)
		inst.reset(new session_manager());

	return	inst.get();
}

size_t	session_manager::count()	const
{
	_non_exclusive_lock_(_map);
	return	_map.size();
}

boost::optional< session_ptr_type >
session_manager::lookup(uint64_t compid_)	const
{
	if (0 == compid_)
	{
		_error_log_(
			boost::format("( %1% )")
			% __FILE_LINE__);
		return	{};
	}

	_non_exclusive_lock_(_map);
	auto pos(_map.find(compid_));
	if (pos == _map.end())
		return	{};

	return { pos->second };
}

bool	session_manager::is_exist(uint64_t compid_) const
{
	if (0 == compid_)
	{
		_error_log_(
			boost::format("( %1% )")
			% __FILE_LINE__);
		return	false;
	}

	_non_exclusive_lock_(_map);
	return	(_map.find(compid_) != _map.end());
}

bool	session_manager::session_enter(const session_ptr_type& p_)
{
	if (nullptr == p_)
	{
		_error_log_(
			boost::format("( %1% )")
			% __FILE_LINE__);
		return	false;
	}

	_exclusive_lock_(_map);
	auto ret = _map.try_emplace(p_->compid(), p_);
	if (false == ret.second)
	{
		_error_log_(
			boost::format("%1% ( %2% )")
			% p_->compid()
			% __FILE_LINE__);
		return	false;
	}

	return	true;
}

void	session_manager::session_leave(uint64_t compid_)
{
	if (0 == compid_)
	{
		_error_log_(
			boost::format("( %1% )")
			% __FILE_LINE__);
		return;
	}

	_exclusive_lock_(_map);
	const auto pos(_map.find(compid_));
	if (pos == _map.end())
	{
		_error_log_(
			boost::format("%1% ( %2% )")
			% compid_
			% __FILE_LINE__);
		return;
	}

	_map.erase(pos);
}

boost::optional< std::vector< session_ptr_type > >
session_manager::in_range_sessions(const session_ptr_type& source_)	const
{
	std::vector< session_ptr_type >	v;

	_non_exclusive_lock_(_map);
	boost::range::for_each(
		_map,
		[&](const auto& v_) -> void
		{
			if (nullptr == v_.second)								return;
			if (v_.second->compid() == source_->compid())	return;
			if (false == v_.second->is_joined)					return;

			v.emplace_back(v_.second);
		});

	if (true == v.empty())	return	{};

	return
	{
		boost::move(v)
	};
}

boost::optional< std::vector< session_ptr_type > >
session_manager::users()	const
{
	_non_exclusive_lock_(_map);
	if (true == _map.empty())	return	{};

	std::vector< session_ptr_type >	v;
	boost::range::for_each(
		_map,
		[&](const auto& v_) -> void
		{
			if (nullptr == v_.second)	return;

			v.emplace_back(v_.second);
		});

	return
	{
		boost::move(v)
	};
}
