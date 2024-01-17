#pragma once

namespace	impl::network::packet
{
#pragma pack(1)
	struct	header
	{
		uint32_t	separator	=	0xffe3e4e5;
		uint16_t	id			=	0;
		uint16_t	seq			=	0;
		uint32_t	size		=	0;
		uint32_t	crc			=	2;
	};
#pragma pack()

	enum : uint16_t
	{
		_packet_size_	=	0xffff,
		_header_size_	=	sizeof( header ),
		//	additional 8 bytes are subtracted with websocket support.
		_body_size_		=	_packet_size_ - _header_size_ - 8,
		_raw_size_		=	_packet_size_ - _header_size_,
	};

#pragma warning(disable : 4996)
	constexpr unsigned char aesKey[]	= {		
			0x65, 0x6C, 0x6C, 0x69,
			0x70, 0x72, 0x6F, 0x6A,
			0x65, 0x6E, 0x63, 0x72,
			0x79, 0x70, 0x74, 0x65 };

	static	
		boost::optional< std::string >
			encrypt( const void* source_, size_t size_ )
	{
		if ( nullptr == source_ || 0 == size_ )
		{
			_error_log_(
				boost::format( "%1% %2% ( %3% )" )
					%	( nullptr == source_ )
					%	size_
					% 	__FILE_LINE__ );
			return	{};
		}

		//std::string s;

		const char* source = static_cast<const char*>(source_);
		std::string s(source, size_);

		/*uint8_t iv[CryptoPP::AES::BLOCKSIZE] = { 0, };
		CryptoPP::AES::Encryption aesEncryption(aesKey, CryptoPP::AES::DEFAULT_KEYLENGTH);
		CryptoPP::ECB_Mode_ExternalCipher::Encryption ecbEncryption(aesEncryption, iv);
		CryptoPP::StreamTransformationFilter stfEncryptor(ecbEncryption, new CryptoPP::StringSink(s));
		stfEncryptor.Put(reinterpret_cast<const unsigned char*>(source_), size_);
		stfEncryptor.MessageEnd();*/

		/*if ( 0 != ( s.size() % 16) )
		{
			_error_log_(
				boost::format( "%1% ( %2% )" )
					%	s.size()
					%	__FILE_LINE__ );
			return	{};
		}*/

		return	{ boost::move( s ) };
	}

	static	
		boost::optional< std::string >
			decrypt( const char* ciphertext_, size_t size_ )
	{
		if ( nullptr == ciphertext_ || 0 == size_ ) //|| 0 != ( size_ % 16 ) )
		{
			_error_log_(
				boost::format( "%1% %2% ( %3% )" )
					%	( nullptr == ciphertext_ )
					%	size_
					% 	__FILE_LINE__ );
			return	{};
		}

		try
		{
			/*std::string s;
			uint8_t iv[CryptoPP::AES::BLOCKSIZE] = { 0, };
			CryptoPP::AES::Decryption aesDecryption(aesKey, CryptoPP::AES::DEFAULT_KEYLENGTH);
			CryptoPP::ECB_Mode_ExternalCipher::Decryption ecbDecryption(aesDecryption, iv);
			CryptoPP::StreamTransformationFilter stfDecryptor(ecbDecryption, new CryptoPP::StringSink(s));
			stfDecryptor.Put(reinterpret_cast<const unsigned char*>(ciphertext_), size_);
			stfDecryptor.MessageEnd();*/

			const char* source = static_cast<const char*>(ciphertext_);
			std::string s(source, size_);

			return { boost::move(s) };
		}
		catch (const std::exception& e_) //(const CryptoPP::Exception& e_)
		{
			_fatal_log_(boost::format("%1% ( %2% )") % e_.what() % __FILE_LINE__);
		}

		return {};
	}

#pragma warning(default : 4996)
}