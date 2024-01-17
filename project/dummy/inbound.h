#pragma once

namespace	framework::net
{
	class	inbound
	{
	public:
		inbound()
			: _body_size(0)
		{
			_header.fill(0);
			_raw_body.fill(0);
		}

		void	init()
		{
			_header.fill(0);
			_body_size = 0;
		}

		void	decide()
		{
			_body_size =
				(_header_size_ <= header_ptr()->size) ?
				static_cast<uint16_t>(header_ptr()->size - _header_size_) :
				0;

			if (_raw_size_ > _body_size)
				_raw_body[_body_size] = 0;
		}

		bool	body_decode()
		{
			const char* charPtr = static_cast<const char*>(_raw_body.data());
			std::string res(charPtr, body_size());
			//auto res(decrypt(_raw_body.data(), body_size()));
			if (!res.empty())
			{
				std::copy(res.c_str(), res.c_str() + body_size(), _raw_body.data());
				return	true;
			}

			_error_log_(
				boost::format("%1% %2% %3% ( %4% )")
				% header_ptr()->id
				% header_ptr()->crc
				% header_ptr()->size
				% __FILE_LINE__);

			return	false;
		}

		char* header_buffer() { return  _header.c_array(); }
		char* raw_buffer() { return	_raw_body.c_array(); }
		const void* body_ptr()				const { return	_raw_body.data(); }
		const uint8_t* body_binary_ptr()		const { return	reinterpret_cast<const uint8_t*>(body_ptr()); }
		const header* header_ptr()			const { return	reinterpret_cast<const header*>(_header.data()); }

		uint16_t			body_size()				const { return	_body_size; }

	private:
		boost::array< char, _header_size_ >	_header;
		uint16_t							_body_size = 0;
		boost::array< char, _raw_size_ >	_raw_body;		//	_flexless_ & _flexible_
	};

	using	inbound_ptr_type = boost::shared_ptr< inbound >;
}