# iocptest
목적: IOCP 성능 테스트
워크플로우:
클라이언트가 헤더와 페이로드가 포함된 패킷을 생성.
TCP Handler를 통해 패킷을 전송.
TCP Handler는 서버의 Endpoint와 통신합니다.
서버측 Session Manager가 클라이언트 세션들의 연결을 수락함.
연결이 설정되면 TCP연결을 통해 클라이언트 패킷을 읽음.
패킷 유형에 따라 적절한 서비스 Handler로 라우팅.
Handler가 요청을 처리하고 응답 패킷을 생성하여 클라이언트 세션으로 다시 전송한다.

구조
패킷 헤더: 패킷 유형, 길이 등과 같은 필수 정보를 포함하여 패킷 형식을 정의한다.
인바운드/아웃바운드 패킷: 패킷 유형에 맞게 패킷을 캡슐화한다.
TCP Handler: IP:port에 연결하고 handler함수를 bind한다.
Session: 서버에 대한 연결을 나타냄. 서버 end point에 연결하여 IO Context를 이용해서 request 패킷을 전송한다.
서버 패킷 유형별 인터페이스 저리 후 응답 패킷 생성 및 클라이언트 세션으로 다시 전송.


필요 설치 파일: Boost 1.83
[ 다운로드 링크 : https://sourceforge.net/projects/boost/files/boost/1.83.0/ ]
소스 위치: D:\OpenSource\boost_1_83_0
설치 위치: D:\OpenSource\boost
D:\OpenSource\boost_1_83_0\tools\build 폴더로 이동
bootstrap.bat 실행
b2.exe install --prefix=D:\OpenSource\boost142 toolset=msvc-14.2 실행
D:\OpenSource\boost_1_83_0 폴더로 이동
환경변수 변경
set PATH=%PATH%;D:\OpenSource\boost142 명령을 실행하여 설치된 b2의 바이너리 경로를 PATH에 추가.
Boost 빌드
> b2 --build-type=complete toolset=msvc-14.2 stage
Boost 설치
> b2 --build-type=complete --prefix=D:\OpenSource\boost142 toolset=msvc-14.2 install

비쥬얼 스튜디오에서 참조 시:
C/C++ 에서 추가 포함 디렉터리에 D:\OpenSource\boost\include\boost-1_83 추가
링커 일반에 추가 라이브러리 디렉터리에 D:\OpenSource\boost142\lib 추가


Handshake
패킷 통신 링크: https://docs.google.com/presentation/d/1EL5GpDtanEqWeWUAAooQxCj4mK8P7SNc9ippeu5FbpQ/edit#slide=id.p

패킷 유형:
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
_CS_MARKET_DATA = 53003, //여기에 symbol 10000개를 할당.
_SC_MARKET_DATA = 53004,
sSymbol : 가상 호가 정보를 represent 함. 멤버변수:
	std::string Symbol;
	float Bid = 0;
	float Ask = 0;
	float DailyChange = 0;

세부 구조
클라이언트: trader 객체는 클라이언트를 나타낸다, 이는 고유 ID “compid”를 가지고 있으며 서버와 TCP 통신을 한다.
연결 시 각 session은 tcp_handler함수가 bind되 있으며 io 작업 완료시 해당 함수가 불려진다.
boost::bind(
		&tcp_handler::on_route,
		this->shared_from_this(),
	_1));

각 trader 객체는 repeat task를 수행하는데:
30초 에 한번씩 HEART BEAT를 체크
200ms 주기로 _CS_MARKET_DATA 패킷을 서버로 전송,
서버에서 CS_MARKET_DATA 패킷을 수신하면 Symbol 만개 랜덤 generate해서 다시 클라에게 전송.
이 경우 클라이언트에서 _SC_MARKET_DATA 패킷이 수신되어_SC_MARKET_DATA 패킷의 sSymbol Sym[10000] 를 읽을 수 있다.
