#pragma once

namespace	impl::util
{
	extern bool		quicklz_compress( const std::string_view& source_, OUT std::string& dest_ );
	extern bool		quicklz_decompress( const std::string_view& source_, OUT std::string& dest_ );
}