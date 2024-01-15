/*!
* \class locale.h
*
* \ingroup play vegas team
*
* \brief
*
* TODO: 버그 리포트는 아래 메일로 좀 부탁해요!!
*
* \note 
*
* \author jay kim
*
* \version 1.0
*
* \date 2022/1/1
*
* Contact:   muse76@hotmail.com
*            muse4116@gmail.com
*
*/

#pragma once

namespace	impl::util::locale
{
	extern	boost::optional< std::string >	to_utf( const std::wstring_view& multi_ );
	extern	boost::optional< std::string >	to_utf( const std::string_view& multi_ );
	extern	boost::optional< std::wstring >	from_utf_to_wide( const std::string& utf_ );
	extern	boost::optional< std::string >	from_utf( const std::string& utf_ );
}