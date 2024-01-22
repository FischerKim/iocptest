﻿#pragma once

namespace	impl::network::packet
{
	class	outbound : public impl::util::memory_pool::singleton_pool_allocator< outbound >
	{
	public:
		outbound()
			:	_buf_use_size( 0 )
		{
			_buffers.fill( 0 );
		}

		void	init()
		{
			_buf_use_size	=	0;
		}

		void	encode(uint16_t id_packet_, const void* body_, size_t size_)
		{
			if (0 == size_ || nullptr == body_)
			{
				_error_log_(
					boost::format("%1% %2% %3% ( %4% )")
					% id_packet_
					% (nullptr == body_)
					% size_
					% __FILE_LINE__);
				return;
			}

			auto res(encrypt(body_, size_));
			if (!res)
			{
				_error_log_(
					boost::format("%1% %2% ( %3% )")
					% id_packet_
					% size_
					% __FILE_LINE__);
				return;
			}

			size_ = res->size();

			_header.id = id_packet_;
			_header.size = _header_size_ + static_cast<uint32_t>(size_);

			std::copy(reinterpret_cast<const char*>(&_header),
				reinterpret_cast<const char*>(&_header) + _header_size_,
				_buffers.begin());

			std::copy(res->begin(), res->end(), _buffers.begin() + _header_size_);
			_buf_use_size = _header.size;

			/*_debug_log_(
				boost::format("sending: compid %1% body: %2% (%3%)")
				% id_packet_
				% body_
				% __FILE_LINE__);*/

		}

		using buffer_type	=	boost::array< char, _packet_size_ >;
		buffer_type&	get_buffer() { return	_buffers; }
		size_t			get_buf_use_size() { return	_buf_use_size; }

	protected:
		size_t			_buf_use_size = 0;
		buffer_type		_buffers	=	{ 0, };
		header			_header;
	};

	using	outbound_ptr_type	=	boost::shared_ptr< outbound >;
}