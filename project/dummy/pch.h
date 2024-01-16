#pragma once

#define	USE_IMPL_LIB
#include <impl.h>

//#define	CRYPTOPP_DISABLE_UNCAUGHT_EXCEPTION
//#pragma comment( lib, "cryptlib.lib" )
//#include <cryptopp/aes.h>
//#include <cryptopp/modes.h>
//#include <cryptopp/filters.h>

#include <header.h>
#include <inbound.h>
#include <outbound.h>
#include <base_session.h>
#include <tcp_session.h>

#include <base_handler.h>
#include <tcp_handler.hpp>
#include <tcp_handler.ipp>
#include <tcp_client.hpp>
#include <tcp_client.ipp>

#include <packet.h>
#include <trader.h>

#include <iostream>
#include <fstream>
#include <vector>
#include <mutex>