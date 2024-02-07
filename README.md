# iocptest
목적: IOCP 성능 테스트
임의의 패킷구조체를 정의해서 서버-다수 클라이언트간에 데이터를 IOCP를 사용해서 처리한다.
현재 어플리케이션에서 사용하는 버퍼 사이즈는 2^16-1 바이트이다.

기본 구조
패킷 헤더: 패킷 유형, 길이 등과 같은 필수 정보를 포함하여 패킷 형식을 정의한다.
인바운드/아웃바운드 패킷: 패킷 유형에 맞게 패킷을 캡슐화한다.
TCP Handler: IP:port에 연결하고 handler함수를 정의한다.
Session: 서버에 대한 연결을 나타냄. 서버 end point에 연결하여 IO Context를 이용해서 request 패킷을 전송한다.
서버 패킷 유형별 인터페이스 저리 후 응답 패킷 생성 및 클라이언트 세션으로 다시 전송한다.


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

