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

struct sSymbol {
    char name[10];
    boost::posix_time::ptime time;// = boost::posix_time::not_a_date_time;
    int digits = 5;
    float spread_float = 0.0f;
    int spread = 11;
    int trade_calc_mode = 5;
    int trade_mode = 4;
    int start_time = 0;
    int expiration_time = 0;
    int trade_stops_level = 0;
    int trade_freeze_level = 0;
    int trade_exemode = 2;
    int swap_mode = 1;
    int swap_rollover3days = 3;
    float point = 0.00001000f;
    float trade_tick_value = 1.00000000f;
    float trade_tick_value_profit = 1.00000000f;
    float trade_tick_value_loss = 1.00000000f;
    float trade_tick_size = 0.00001000f;
    float trade_contract_size = 100000.00000000f;
    float volume_min = 0.01000000f;
    float volume_max = 100.00000000f;
    float volume_step = 0.01000000f;
    float volume_limit = 0.00000000f;
    float swap_long = -6.01000000f;
    float swap_short = 2.08000000f;
    float margin_initial = 0.00000000f;
    float margin_maintenance = 0.00000000f;
    char currency_base[5];
    char currency_profit[5];
    char currency_margin[5];
    char description[32];
    char path[32];
    int error_id = 0;
    char error_description[256];
    char demo[16];

    sSymbol() : time(boost::posix_time::not_a_date_time), digits(5), spread_float(0.0f), spread(11), trade_calc_mode(5), trade_mode(4), start_time(0), expiration_time(0), trade_stops_level(0),
        trade_freeze_level(0), trade_exemode(2), swap_mode(1), swap_rollover3days(3), point(0.00001000f),
        trade_tick_value(1.00000000f), trade_tick_value_profit(1.00000000f), trade_tick_value_loss(1.00000000f),
        trade_tick_size(0.00001000f), trade_contract_size(100000.00000000f), volume_min(0.01000000f),
        volume_max(100.00000000f), volume_step(0.01000000f), volume_limit(0.00000000f), swap_long(-6.01000000f),
        swap_short(2.08000000f), margin_initial(0.00000000f), margin_maintenance(0.00000000f), error_id(0) {
        memset(name, 0, sizeof(name));
        memset(currency_base, 0, sizeof(currency_base));
        memset(currency_profit, 0, sizeof(currency_profit));
        memset(currency_margin, 0, sizeof(currency_margin));
        memset(description, 0, sizeof(description));
        memset(path, 0, sizeof(path));
        memset(error_description, 0, sizeof(error_description));
        memset(demo, 0, sizeof(demo));
        strncpy_s(name, "EURUSD", sizeof(name) - 1);
        strncpy_s(currency_base, "EUR", sizeof(currency_base) - 1);
        strncpy_s(currency_profit, "USD", sizeof(currency_profit) - 1);
        strncpy_s(currency_margin, "EUR", sizeof(currency_margin) - 1);
        strncpy_s(description, "Euro vs US Dollar", sizeof(description) - 1);
        strncpy_s(path, "Forex\\EURUSD", sizeof(path) - 1);
        strncpy_s(error_description, "The operation completed successfully", sizeof(error_description) - 1);
        strncpy_s(demo, "in DEMO mode", sizeof(demo) - 1);
    }

    sSymbol(
        const char* _name,
        boost::posix_time::ptime _time,
        int _digits,
        float _spread_float,
        int _spread,
        int _trade_calc_mode,
        int _trade_mode,
        int _start_time,
        int _expiration_time,
        int _trade_stops_level,
        int _trade_freeze_level,
        int _trade_exemode,
        int _swap_mode,
        int _swap_rollover3days,
        float _point,
        float _trade_tick_value,
        float _trade_tick_value_profit,
        float _trade_tick_value_loss,
        float _trade_tick_size,
        float _trade_contract_size,
        float _volume_min,
        float _volume_max,
        float _volume_step,
        float _volume_limit,
        float _swap_long,
        float _swap_short,
        float _margin_initial,
        float _margin_maintenance,
        const char* _currency_base,
        const char* _currency_profit,
        const char* _currency_margin,
        const char* _description,
        const char* _path,
        int _error_id,
        const char* _error_description,
        const char* _demo)
        : time(_time), digits(_digits), spread_float(_spread_float), spread(_spread), trade_calc_mode(_trade_calc_mode),
        trade_mode(_trade_mode), start_time(_start_time), expiration_time(_expiration_time), trade_stops_level(_trade_stops_level),
        trade_freeze_level(_trade_freeze_level), trade_exemode(_trade_exemode), swap_mode(_swap_mode),
        swap_rollover3days(_swap_rollover3days), point(_point), trade_tick_value(_trade_tick_value),
        trade_tick_value_profit(_trade_tick_value_profit), trade_tick_value_loss(_trade_tick_value_loss),
        trade_tick_size(_trade_tick_size), trade_contract_size(_trade_contract_size), volume_min(_volume_min),
        volume_max(_volume_max), volume_step(_volume_step), volume_limit(_volume_limit), swap_long(_swap_long),
        swap_short(_swap_short), margin_initial(_margin_initial), margin_maintenance(_margin_maintenance),
        error_id(_error_id) {
        strncpy_s(name, _name, sizeof(name) - 1);
        name[sizeof(name) - 1] = '\0';

        strncpy_s(currency_base, _currency_base, sizeof(currency_base) - 1);
        currency_base[sizeof(currency_base) - 1] = '\0';

        strncpy_s(currency_profit, _currency_profit, sizeof(currency_profit) - 1);
        currency_profit[sizeof(currency_profit) - 1] = '\0';

        strncpy_s(currency_margin, _currency_margin, sizeof(currency_margin) - 1);
        currency_margin[sizeof(currency_margin) - 1] = '\0';

        strncpy_s(description, _description, sizeof(description) - 1);
        description[sizeof(description) - 1] = '\0';

        strncpy_s(path, _path, sizeof(path) - 1);
        path[sizeof(path) - 1] = '\0';

        strncpy_s(error_description, _error_description, sizeof(error_description) - 1);
        error_description[sizeof(error_description) - 1] = '\0';

        strncpy_s(demo, _demo, sizeof(demo) - 1);
        demo[sizeof(demo) - 1] = '\0';
    }
};

struct CS_MARKET_DATA
{
    uint64_t compid = 0;
    char name[10];
};

struct SC_MARKET_DATA
{
    uint64_t compid = 0;
    int16_t Error = 0;
    sSymbol Sym[100];
};

#pragma pack( pop )