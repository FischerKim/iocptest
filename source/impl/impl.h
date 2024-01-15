/*!
* \class impl.h
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

#include <impl_lib.h>

#include <exception_handler.h>

#define	CRYPTOPP_DISABLE_UNCAUGHT_EXCEPTION
#pragma comment( lib, "cryptlib.lib" )
#include <cryptopp/aes.h>
#include <cryptopp/modes.h>
#include <cryptopp/filters.h>

#include <type.h>
#include <log_tool.h>
#include <util.h>
#include <network.h>
#include <odbc.h>