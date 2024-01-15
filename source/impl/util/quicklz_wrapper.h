/*!
* \class quicklz_wrapper.h
*
* \ingroup
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
* \date 2020/2/11
*
* Contact:	muse76@hotmail.com
*			muse4116@gmail.com
*
*/

#pragma once

namespace	impl::util
{
	extern bool		quicklz_compress( const std::string_view& source_, OUT std::string& dest_ );
	extern bool		quicklz_decompress( const std::string_view& source_, OUT std::string& dest_ );
}