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
	

	//������ �ʱ�ȭ�ϴ� �Լ�
	bool Init(const UINT32 maxIOWorkerThreadCount_);

	//������ �ּ������� ���ϰ� �����Ű�� ���� ��û�� �ޱ� ���� ������ ����ϴ� �Լ�
	bool BindandListen(int bindPort_);

	//���� ��û�� �����ϰ� �޼����� �޾Ƽ� ó���ϴ� �Լ�
	bool StartServer(const UINT32 maxClientCount_);

	//�����Ǿ��ִ� �����带 �ı��Ѵ�.
	void DestroyThread();

	bool SendMsg(const UINT32 clientIndex_, const UINT32 dataSize_, char* pData);

	virtual void OnConnect(const UINT32 clientIndex_) {}

	virtual void OnClose(const UINT32 clientIndex_) {}

	virtual void OnReceive(const UINT32 clientIndex_, const UINT32 size_, char* pData_) {}

private:
	void CreateClient(const UINT32 maxClientCount_);

	//WaitingThread Queue���� ����� ��������� ����
	bool CreateWokerThread();

	//������� �ʴ� Ŭ���̾�Ʈ ���� ����ü�� ��ȯ�Ѵ�.
	stClientInfo* GetEmptyClientInfo();

	stClientInfo* GetClientInfo(const UINT32 clientIndex_);

	//accept��û�� ó���ϴ� ������ ����
	bool CreateAccepterThread();

	//Overlapped I/O�۾��� ���� �Ϸ� �뺸�� �޾� �׿� �ش��ϴ� ó���� �ϴ� �Լ�
	void WokerThread();

	//������� ������ �޴� ������
	void AccepterThread();

	//������ ������ ���� ��Ų��.
	void CloseSocket(stClientInfo* clientInfo_, bool isForce_ = false);



	UINT32 MaxIOWorkerThreadCount = 0;

	//Ŭ���̾�Ʈ ���� ���� ����ü
	std::vector<stClientInfo*> mClientInfos;

	//Ŭ���̾�Ʈ�� ������ �ޱ����� ���� ����
	SOCKET		mListenSocket = INVALID_SOCKET;

	//���� �Ǿ��ִ� Ŭ���̾�Ʈ ��
	int			mClientCnt = 0;

	//IO Worker ������
	std::vector<std::thread> mIOWorkerThreads;

	//Accept ������
	std::thread	mAccepterThread;

	//CompletionPort��ü �ڵ�
	HANDLE		mIOCPHandle = INVALID_HANDLE_VALUE;

	//�۾� ������ ���� �÷���
	bool		mIsWorkerRun = true;

	//���� ������ ���� �÷���
	bool		mIsAccepterRun = true;
};