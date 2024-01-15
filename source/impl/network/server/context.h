/*!
* \class context.h
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

namespace	impl::network::server
{
	class	context : private boost::noncopyable
	{
	public:
		context( 
			const std::string& crt_, 
			const std::string& key_, 
			const std::string& pem_, 
			const std::string& pw_, 
			const std::string& verify_ = "" );

		context( 
			const std::string& crt_, 
			const std::string& key_, 
			const std::string& pem_ );

		~context()	{}

		boost::asio::ssl::context&	
			get_context()			{	return	_context;	}

	private:
		const	std::string&
			get_password()	const	{	return	_pw;		}

	private:
		boost::asio::ssl::context	_context;
		std::string					_pw;
	};
}