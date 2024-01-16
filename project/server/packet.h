#pragma once
typedef unsigned char byte;

#define		MAX_USERS		64
#define		MAX_ACC_TOKEN	200
#define		MAX_DOMAIN_NAME	128
enum
{
	_CS_HEART_BEAT = 1000,		//하트비트
	_SC_HEART_BEAT = 1001,		//하트비트 응답.
	_CS_TOTAL_USERCOUNT = 1100,
	_SC_TOTAL_USERCOUNT = 1101,
	_CS_SERVER_LOGIN = 5017,
	_SC_SERVER_LOGIN = 5018,
	_CS_SERVER_ACCESS = 5019,
	_SC_SERVER_ACCESS = 5020,
	_SC_USER_LOGOUT = 53000,
	_CS_USER_LOOKUP = 53001,
	_SC_USER_LOOKUP = 53002,
	_CS_MARKET_DATA = 53003,
	_SC_MARKET_DATA = 53004,
};

#pragma pack( push, 1 )

struct CS_HEART_BEAT
{
	uint64_t	compid = 0;
};

struct SC_HEART_BEAT
{
	uint16_t	result = 0;
	uint64_t	compid = 0;
};

struct CS_TOTAL_USERCOUNT
{
	uint64_t	compid = 0;
};

struct SC_TOTAL_USERCOUNT
{
	uint64_t	compid = 0;
	uint16_t	total_count = 0;
};

struct CS_SERVER_LOGIN
{
	uint64_t	compid = 0;						//0이면 발급
	byte		bPassFlag = 0;						//테스트용 인즌패스 (인증 된것 처럼)
	char		serviceKey[MAX_ACC_TOKEN] = { 0, };	//인증키
	uint32_t	curVersion = 0;
};

struct SC_SERVER_LOGIN
{
	byte		bPass = 0;
	//uint16_t durationMinutes = 0;
};

struct CS_SERVER_ACCESS
{
	uint64_t compid;
};

struct SC_SERVER_ACCESS
{
	uint16_t result;							//401 실패 ( 인증 다시... )
	uint64_t compid;
	char	serverIp[MAX_DOMAIN_NAME];		    //클라가.. 접속 가능 IP or Dns번호										
	uint16_t	port;							//인증성공이면 현재의 서버 Port 알려줌 서버 이동용.
	uint64_t	serverTime;						//서버상의 시간값.
	uint32_t	curVersion;						//Ver RETURN

	SC_SERVER_ACCESS()
		: result(0),
		compid(0),
		port(0),
		serverTime(0),
		curVersion(0)
	{
		std::fill(std::begin(serverIp), std::end(serverIp), 0);
	}
};

struct SC_USER_LOGOUT
{
	uint64_t	compid = 0;
};

struct CS_USER_LOOKUP
{
	uint64_t	compid = 0;
};

struct SC_USER_LOOKUP
{
	uint64_t	compid = 0;
	byte		is_not_exist = 0;
};

struct sSymbol
{
	std::string Symbol;
	float Bid = 0;
	float Ask = 0;
	float DailyChange = 0;

	sSymbol() : Bid(0), Ask(0), DailyChange(0) {}
	sSymbol(const std::string& _Symbol, float _Bid, float _Ask, float _DailyChange)
		: Symbol(_Symbol), Bid(_Bid), Ask(_Ask), DailyChange(_DailyChange) {}

	float getRandomValue(float min, float max) {
		impl::util::random_generator	g;
		return	g.real_rand< float >(min, max);
	}
};

struct CS_MARKET_DATA
{
	uint64_t compid = 0;
	//sSymbol Sym[10000] = {};
};

struct SC_MARKET_DATA
{
	uint64_t compid = 0;
	int16_t Error = 0;
	sSymbol Sym[10000] = {};
};

#pragma pack( pop )