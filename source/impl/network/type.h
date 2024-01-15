/*!
* \class type.h
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
* \date 2023/9/2
*
* Contact:	muse76@hotmail.com
*           muse4116@gmail.com
*
*/

#pragma once

namespace	impl::network
{
	enum	class	e_protocol_type : uint8_t
	{
		_none_	=	0x00,
		_tcp_	=	0x01,
		_ssl_	=	0x02,
		_ws_	=	0x03,
		_wss_	=	0x04,
	};
}