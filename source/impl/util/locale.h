#pragma once

namespace	impl::util::locale
{
	extern	boost::optional< std::string >	to_utf( const std::wstring_view& multi_ );
	extern	boost::optional< std::string >	to_utf( const std::string_view& multi_ );
	extern	boost::optional< std::wstring >	from_utf_to_wide( const std::string& utf_ );
	extern	boost::optional< std::string >	from_utf( const std::string& utf_ );
}