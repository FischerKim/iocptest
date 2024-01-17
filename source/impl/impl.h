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