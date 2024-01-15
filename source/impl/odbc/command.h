/*!
* \class command.h
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

namespace	impl::odbc
{
	using	param_vector_type		=	std::vector< boost::any >;
	using	param_vector_rows_type	=	std::vector< param_vector_type >;

	class	command : private boost::noncopyable
	{
	public:
		command()			{}
		virtual ~command()	{}

		void	init();

		template< typename type >
		void	bind_parameter( const type& param_ );

		bool	make_query( const std::string_view& sp_ );
		void	ping_query();

		void	get_params_string( OUT std::wstring& params_ )	const;

		const std::wstring&	
			get_query()		const	{	return	_query;		}

		const param_vector_type&	
			get_param_vec()	const	{	return	_param_v;	}

	protected:
		std::string					_sp;
		std::vector< boost::any >	_param_v;
		std::wstring				_query;
	};

	template< typename type >
	void	command::bind_parameter( const type& param_ )
	{
		_param_v.emplace_back( param_ );
		boost::any& a	=	_param_v.back();

		if ( a.type() == typeid( uint8_t ) )		return;
		if ( a.type() == typeid( short ) )			return;
		if ( a.type() == typeid( int ) )			return;
		if ( a.type() == typeid( long ) )			return;
		if ( a.type() == typeid( float ) )			return;
		if ( a.type() == typeid( double ) )			return;
		if ( a.type() == typeid( __int64 ) )		return;
		if ( a.type() == typeid( std::string ) )	return;
		if ( a.type() == typeid( std::wstring ) )	return;

		_param_v.pop_back();

		throw	std::exception( boost::str( 
			boost::format( "%1% ( %2% )" ) 
				%	a.type().name() 
				%	__FILE_LINE__ )
			.c_str() );
	}
}