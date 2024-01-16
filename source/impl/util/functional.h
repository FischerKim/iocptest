#pragma once

namespace	impl::util
{
	extern	boost::optional< std::string >	timestamp_micro();
	extern	boost::optional< std::string >	timestamp();

	template < typename type >
	static	bool	is_equal( const boost::shared_ptr< type >& _1_, const boost::shared_ptr< type >& _2_ )
	{
		if ( nullptr == _1_ || nullptr == _2_ )
		{
			_error_log_( 
				boost::format( "%1% %2% ( %3% )" ) 
					%	( nullptr == _1_ ? "nullptr" : "not nullptr" )
					%	( nullptr == _2_ ? "nullptr" : "not nullptr" )
					%	__FILE_LINE__ );

			return	false;
		}

		return	*_1_.get() == *_2_.get();
	}
}