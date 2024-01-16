#pragma once

namespace	impl::util
{
	class	uuid_generator : private boost::noncopyable
	{
	public:
		enum
		{
			_size_	=	36
		};

		bool		gen( OUT std::string& r_ );

		bool		gen( OUT boost::uuids::uuid& u_ );

		//	hash uuid
		uint64_t	gen();

		static	void		gen_uuid( OUT std::string& r_ );

		static	uint64_t	gen_hash_uuid();

	private:
		class lock_gen : public boost::uuids::basic_random_generator< boost::mt19937_64 >, public impl::util::spin_lock	{};
		lock_gen	_gen;

		boost::hash< boost::uuids::uuid >	_uuid_hasher;
	};
}

/*
impl::util::uuid_generator	gen;

boost::uuids::uuid	u, uu;
gen.gen( u );

std::string stru = boost::uuids::to_string( u ).c_str();

boost::uuids::string_generator sgen;
uu = sgen( stru );

std::string stru2 = boost::uuids::to_string( uu ).c_str();

std::cout << stru << std::endl;
std::cout << stru2 << std::endl;

bool	is_tie = boost::equal( u.data, uu.data );
*/