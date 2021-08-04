#pragma once

#include "InnInclude.h"
#include "Define.h"
#include "Functions.h"

class IComplexSocket
{
public:
	virtual BOOL Stop() PURE;
	virtual BOOL Send(ULONG_PTR dwConnID, const BYTE* pBuffer, int iLength, int iOffset = 0) PURE;
	virtual BOOL SendPackets(ULONG_PTR dwConnID, const WSABUF pBuffers[], int iCount) PURE;
	virtual BOOL PauseReceive(ULONG_PTR dwConnID, BOOL bPause = TRUE) PURE;
	virtual BOOL Disconnect(ULONG_PTR dwConnID, BOOL bForce = TRUE) PURE;
	virtual BOOL DisconnectLongConnections(DWORD dwPeriod, BOOL bForce = TRUE) PURE;
	virtual BOOL DisconnectSilenceConnections(DWORD dwPeriod, BOOL bForce = TRUE) PURE;
	virtual BOOL Wait(DWORD dwMilliseconds = INFINITE) PURE;

public:
	virtual BOOL SetConnectionExtra(ULONG_PTR dwConnID, PVOID pExtra) PURE;
	virtual BOOL GetConnectionExtra(ULONG_PTR dwConnID, PVOID* ppExtra) PURE;

	virtual BOOL IsSecure() PURE;
	virtual BOOL HasStarted() PURE;
	virtual EnServiceState GetState() PURE;
	virtual DWORD GetConnectionCount() PURE;
	virtual BOOL GetAllConnectionIDs(ULONG_PTR pIDs[], DWORD& dwCount) PURE;
	virtual BOOL GetConnectPeriod(ULONG_PTR dwConnID, DWORD& dwPeriod) PURE;
	virtual BOOL GetSilencePeriod(ULONG_PTR dwConnID, DWORD& dwPeriod) PURE;
	virtual BOOL GetLocalAddress(ULONG_PTR dwConnID, TCHAR lpszAddress[], int& iAddressLen, USHORT& usPort) PURE;
	virtual BOOL GetRemoteAddress(ULONG_PTR dwConnID, TCHAR lpszAddress[], int& iAddressLen, USHORT& usPort) PURE;
	virtual EnSocketError GetLastError() PURE;
	virtual LPCTSTR GetLastErrorDesc() PURE;
	virtual BOOL GetPendingDataLength(ULONG_PTR dwConnID, int& iPending) PURE;
	virtual BOOL IsPauseReceive(ULONG_PTR dwConnID, BOOL& bPaused) PURE;
	virtual BOOL IsConnected(ULONG_PTR dwConnID) PURE;

	virtual void SetReuseAddressPolicy(EnReuseAddressPolicy enReusePolicy) PURE;
	virtual void SetSendPolicy(EnSendPolicy enSendPolicy) PURE;
	virtual void SetOnSendSyncPolicy(EnOnSendSyncPolicy enSyncPolicy) PURE;
	virtual void SetMaxConnectionCount(DWORD dwMaxConnectionCount) PURE;
	virtual void SetFreeSocketObjLockTime(DWORD dwFreeSocketObjLockTime) PURE;
	virtual void SetFreeSocketObjPool(DWORD dwFreeSocketObjPool) PURE;
	virtual void SetFreeBufferObjPool(DWORD dwFreeBufferObjPool) PURE;
	virtual void SetFreeSocketObjHold(DWORD dwFreeSocketObjHold) PURE;
	virtual void SetFreeBufferObjHold(DWORD dwFreeBufferObjHold) PURE;
	virtual void SetWorkerThreadCount(DWORD dwWorkerThreadCount) PURE;
	virtual void SetMarkSilence(BOOL bMarkSilence) PURE;

	virtual EnReuseAddressPolicy GetReuseAddressPolicy() PURE;
	virtual EnSendPolicy GetSendPolicy() PURE;
	virtual EnOnSendSyncPolicy GetOnSendSyncPolicy() PURE;
	virtual DWORD GetMaxConnectionCount() PURE;
	virtual DWORD GetFreeSocketObjLockTime() PURE;
	virtual DWORD GetFreeSocketObjPool() PURE;
	virtual DWORD GetFreeBufferObjPool() PURE;
	virtual DWORD GetFreeSocketObjHold() PURE;
	virtual DWORD GetFreeBufferObjHold() PURE;
	virtual DWORD GetWorkerThreadCount() PURE;
	virtual BOOL IsMarkSilence() PURE;

public:
	virtual ~IComplexSocket() {}
};

class IServer : public IComplexSocket
{
public:
	virtual BOOL Start(LPCTSTR lpszBindAddress, USHORT usPort) PURE;

public:
	virtual BOOL GetListenAddress(TCHAR lpszAddress[], int& iAddressLen, USHORT& usPort) PURE;
};

class ITcpServer : public IServer
{
public:

	virtual BOOL SendSmallFile(ULONG_PTR dwConnID, LPCTSTR lpszFileName, const LPWSABUF pHead = nullptr, const LPWSABUF pTail = nullptr) PURE;
public:
	virtual void SetAcceptSocketCount(DWORD dwAcceptSocketCount) PURE;
	virtual void SetSocketBufferSize(DWORD dwSocketBufferSize) PURE;
	virtual void SetSocketListenQueue(DWORD dwSocketListenQueue) PURE;
	virtual void SetKeepAliveTime(DWORD dwKeepAliveTime) PURE;
	virtual void SetKeepAliveInterval(DWORD dwKeepAliveInterval) PURE;

	virtual DWORD GetAcceptSocketCount() PURE;
	virtual DWORD GetSocketBufferSize() PURE;
	virtual DWORD GetSocketListenQueue() PURE;
	virtual DWORD GetKeepAliveTime() PURE;
	virtual DWORD GetKeepAliveInterval() PURE;
};


#define ENSURE_STOP()							{if(GetState() != SS_STOPPED) Stop();}
#define ENSURE_HAS_STOPPED()					{if(GetState() != SS_STOPPED) return;}