#pragma once

#include "../Common/Macros.h"

#include <WinSock2.h>
#include <thread>
#include <vector>

#include "BaseRefObject.h"
#include "CObjectPool.h"
#include "ClientInfo.h"

class CTiocp : public CBaseRefObject
{
public:
	CTiocp(void) {}

	virtual ~CTiocp(void);
	

	//소켓을 초기화하는 함수
	bool Init(const UINT32 maxIOWorkerThreadCount_);

	//서버의 주소정보를 소켓과 연결시키고 접속 요청을 받기 위해 소켓을 등록하는 함수
	bool BindandListen(int bindPort_);

	//접속 요청을 수락하고 메세지를 받아서 처리하는 함수
	bool StartServer(const UINT32 maxClientCount_);

	//생성되어있는 쓰레드를 파괴한다.
	void DestroyThread();

	bool SendMsg(const UINT32 clientIndex_, const UINT32 dataSize_, char* pData);

	virtual void OnConnect(const UINT32 clientIndex_) {}

	virtual void OnClose(const UINT32 clientIndex_) {}

	virtual void OnReceive(const UINT32 clientIndex_, const UINT32 size_, char* pData_) {}

private:
	void CreateClient(const UINT32 maxClientCount_);

	//WaitingThread Queue에서 대기할 쓰레드들을 생성
	bool CreateWokerThread();

	//사용하지 않는 클라이언트 정보 구조체를 반환한다.
	stClientInfo* GetEmptyClientInfo();

	stClientInfo* GetClientInfo(const UINT32 clientIndex_);

	//accept요청을 처리하는 쓰레드 생성
	bool CreateAccepterThread();

	//Overlapped I/O작업에 대한 완료 통보를 받아 그에 해당하는 처리를 하는 함수
	void WokerThread();

	//사용자의 접속을 받는 쓰레드
	void AccepterThread();

	//소켓의 연결을 종료 시킨다.
	void CloseSocket(stClientInfo* clientInfo_, bool isForce_ = false);



	UINT32 MaxIOWorkerThreadCount = 0;

	//클라이언트 정보 저장 구조체
	std::vector<stClientInfo*> mClientInfos;

	//클라이언트의 접속을 받기위한 리슨 소켓
	SOCKET		mListenSocket = INVALID_SOCKET;

	//접속 되어있는 클라이언트 수
	int			mClientCnt = 0;

	//IO Worker 스레드
	std::vector<std::thread> mIOWorkerThreads;

	//Accept 스레드
	std::thread	mAccepterThread;

	//CompletionPort객체 핸들
	HANDLE		mIOCPHandle = INVALID_HANDLE_VALUE;

	//작업 쓰레드 동작 플래그
	bool		mIsWorkerRun = true;

	//접속 쓰레드 동작 플래그
	bool		mIsAccepterRun = true;
};