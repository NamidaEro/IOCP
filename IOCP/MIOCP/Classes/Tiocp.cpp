#include "Tiocp.h"

#pragma comment(lib, "ws2_32")
#pragma comment(lib, "mswsock.lib")

CTiocp::~CTiocp(void)
{
	{
		//윈속의 사용을 끝낸다.
		WSACleanup();
	}
}

bool CTiocp::Init(const UINT32 maxIOWorkerThreadCount_)
{
	WSADATA wsaData;

	int nRet = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (0 != nRet)
	{
		printf("[에러] WSAStartup()함수 실패 : %d\n", WSAGetLastError());
		return false;
	}

	//연결지향형 TCP , Overlapped I/O 소켓을 생성
	mListenSocket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, NULL, WSA_FLAG_OVERLAPPED);

	if (INVALID_SOCKET == mListenSocket)
	{
		printf("[에러] socket()함수 실패 : %d\n", WSAGetLastError());
		return false;
	}

	MaxIOWorkerThreadCount = maxIOWorkerThreadCount_;

	printf("소켓 초기화 성공\n");
	return true;
}

bool CTiocp::BindandListen(int bindPort_)
{
	SOCKADDR_IN		stServerAddr;
	stServerAddr.sin_family = AF_INET;
	stServerAddr.sin_port = htons(bindPort_); //서버 포트를 설정한다.		
	//어떤 주소에서 들어오는 접속이라도 받아들이겠다.
	//보통 서버라면 이렇게 설정한다. 만약 한 아이피에서만 접속을 받고 싶다면
	//그 주소를 inet_addr함수를 이용해 넣으면 된다.
	stServerAddr.sin_addr.s_addr = htonl(INADDR_ANY);

	//위에서 지정한 서버 주소 정보와 cIOCompletionPort 소켓을 연결한다.
	int nRet = bind(mListenSocket, (SOCKADDR*)&stServerAddr, sizeof(SOCKADDR_IN));
	if (0 != nRet)
	{
		printf("[에러] bind()함수 실패 : %d\n", WSAGetLastError());
		return false;
	}

	//접속 요청을 받아들이기 위해 cIOCompletionPort소켓을 등록하고 
	//접속대기큐를 5개로 설정 한다.
	nRet = listen(mListenSocket, 5);
	if (0 != nRet)
	{
		printf("[에러] listen()함수 실패 : %d\n", WSAGetLastError());
		return false;
	}

	//CompletionPort객체 생성 요청을 한다.
	mIOCPHandle = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, NULL, MaxIOWorkerThreadCount);
	if (NULL == mIOCPHandle)
	{
		printf("[에러] CreateIoCompletionPort()함수 실패: %d\n", GetLastError());
		return false;
	}

	auto hIOCPHandle = CreateIoCompletionPort((HANDLE)mListenSocket, mIOCPHandle, (UINT32)0, 0);
	if (nullptr == hIOCPHandle)
	{
		printf("[에러] listen socket IOCP bind 실패 : %d\n", WSAGetLastError());
		return false;
	}

	printf("서버 등록 성공..\n");
	return true;
}

bool CTiocp::StartServer(const UINT32 maxClientCount_)
{
	CreateClient(maxClientCount_);

	//접속된 클라이언트 주소 정보를 저장할 구조체
	bool bRet = CreateWokerThread();
	if (false == bRet) {
		return false;
	}

	bRet = CreateAccepterThread();
	if (false == bRet) {
		return false;
	}

	printf("서버 시작\n");
	return true;
}

void CTiocp::DestroyThread()
{
	mIsWorkerRun = false;
	CloseHandle(mIOCPHandle);

	for (auto& th : mIOWorkerThreads)
	{
		if (th.joinable())
		{
			th.join();
		}
	}

	//Accepter 쓰레드를 종요한다.
	mIsAccepterRun = false;
	closesocket(mListenSocket);

	if (mAccepterThread.joinable())
	{
		mAccepterThread.join();
	}
}

bool CTiocp::SendMsg(const UINT32 clientIndex_, const UINT32 dataSize_, char* pData)
{
	auto pClient = GetClientInfo(clientIndex_);
	return pClient->SendMsg(dataSize_, pData);
}

void CTiocp::CreateClient(const UINT32 maxClientCount_)
{
	for (UINT32 i = 0; i < maxClientCount_; ++i)
	{
		auto client = new stClientInfo;
		client->Init(i, mIOCPHandle);

		mClientInfos.push_back(client);
	}
}

bool CTiocp::CreateWokerThread()
{
	//WaingThread Queue에 대기 상태로 넣을 쓰레드들 생성 권장되는 개수 : (cpu개수 * 2) + 1 
	auto threadCount = (MaxIOWorkerThreadCount * 2) + 1;
	for (UINT32 i = 0; i < threadCount; i++)
	{
		mIOWorkerThreads.emplace_back([this]() { WokerThread(); });
	}

	printf("WokerThread 시작..\n");
	return true;
}

stClientInfo* CTiocp::GetEmptyClientInfo()
{
	for (auto& client : mClientInfos)
	{
		if (client->IsConnectd() == false)
		{
			return client;
		}
	}

	return nullptr;
}

stClientInfo* CTiocp::GetClientInfo(const UINT32 clientIndex_)
{
	return mClientInfos[clientIndex_];
}

bool CTiocp::CreateAccepterThread()
{
	mAccepterThread = std::thread([this]() { AccepterThread(); });

	printf("AccepterThread 시작..\n");
	return true;
}

void CTiocp::WokerThread()
{
	//CompletionKey를 받을 포인터 변수
	stClientInfo* pClientInfo = nullptr;
	//함수 호출 성공 여부
	BOOL bSuccess = TRUE;
	//Overlapped I/O작업에서 전송된 데이터 크기
	DWORD dwIoSize = 0;
	//I/O 작업을 위해 요청한 Overlapped 구조체를 받을 포인터
	LPOVERLAPPED lpOverlapped = NULL;

	while (mIsWorkerRun)
	{
		bSuccess = GetQueuedCompletionStatus(mIOCPHandle,
			&dwIoSize,					// 실제로 전송된 바이트
			(PULONG_PTR)&pClientInfo,		// CompletionKey
			&lpOverlapped,				// Overlapped IO 객체
			INFINITE);					// 대기할 시간

		//사용자 쓰레드 종료 메세지 처리..
		if (TRUE == bSuccess && 0 == dwIoSize && NULL == lpOverlapped)
		{
			mIsWorkerRun = false;
			continue;
		}

		if (NULL == lpOverlapped)
		{
			continue;
		}

		auto pOverlappedEx = (stOverlappedEx*)lpOverlapped;

		//client가 접속을 끊었을때..			
		if (FALSE == bSuccess || (0 == dwIoSize && IOOperation::ACCEPT != pOverlappedEx->m_eOperation))
		{
			//printf("socket(%d) 접속 끊김\n", (int)pClientInfo->m_socketClient);
			CloseSocket(pClientInfo); //Caller WokerThread()
			continue;
		}


		if (IOOperation::ACCEPT == pOverlappedEx->m_eOperation)
		{
			pClientInfo = GetClientInfo(pOverlappedEx->SessionIndex);
			if (pClientInfo->AcceptCompletion())
			{
				//클라이언트 갯수 증가
				++mClientCnt;

				OnConnect(pClientInfo->GetIndex());
			}
			else
			{
				CloseSocket(pClientInfo, true);  //Caller WokerThread()
			}
		}
		//Overlapped I/O Recv작업 결과 뒤 처리
		else if (IOOperation::RECV == pOverlappedEx->m_eOperation)
		{
			OnReceive(pClientInfo->GetIndex(), dwIoSize, pClientInfo->RecvBuffer());

			pClientInfo->BindRecv();
		}
		//Overlapped I/O Send작업 결과 뒤 처리
		else if (IOOperation::SEND == pOverlappedEx->m_eOperation)
		{
			pClientInfo->SendCompleted(dwIoSize);
		}
		//예외 상황
		else
		{
			printf("Client Index(%d)에서 예외상황\n", pClientInfo->GetIndex());
		}
	}
}

void CTiocp::AccepterThread()
{
	while (mIsAccepterRun)
	{
		auto curTimeSec = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now().time_since_epoch()).count();

		for (auto client : mClientInfos)
		{
			if (client->IsConnectd())
			{
				continue;
			}

			if ((UINT64)curTimeSec < client->GetLatestClosedTimeSec())
			{
				continue;
			}

			auto diff = curTimeSec - client->GetLatestClosedTimeSec();
			if (diff <= RE_USE_SESSION_WAIT_TIMESEC)
			{
				continue;
			}

			client->PostAccept(mListenSocket, curTimeSec);
		}

		std::this_thread::sleep_for(std::chrono::milliseconds(32));
	}
}

void CTiocp::CloseSocket(stClientInfo* clientInfo_, bool isForce_)
{
	if (clientInfo_->IsConnectd() == false)
	{
		return;
	}

	auto clientIndex = clientInfo_->GetIndex();

	clientInfo_->Close(isForce_);

	OnClose(clientIndex);
}
