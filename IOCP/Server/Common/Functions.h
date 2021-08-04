#pragma once

#include "InnInclude.h"
#include "Define.h"

inline void YieldThread(UINT i = THREAD_YIELD_CYCLE)
{
	if ((i & THREAD_SWITCH_CYCLE) == THREAD_SWITCH_CYCLE)
		::SwitchToThread();
	else if ((i & THREAD_YIELD_CYCLE) == THREAD_YIELD_CYCLE)
		::YieldProcessor();
}

DWORD TimeGetTime()
{
	return ::timeGetTime();
}

LPCTSTR GetSocketErrorDesc(EnSocketError enCode)
{
	switch (enCode)
	{
	case SE_OK:						return _T("SUCCESS");
	case SE_ILLEGAL_STATE:			return _T("Illegal State");
	case SE_INVALID_PARAM:			return _T("Invalid Parameter");
	case SE_SOCKET_CREATE:			return _T("Create SOCKET Fail");
	case SE_SOCKET_BIND:			return _T("Bind SOCKET Fail");
	case SE_SOCKET_PREPARE:			return _T("Prepare SOCKET Fail");
	case SE_SOCKET_LISTEN:			return _T("Listen SOCKET Fail");
	case SE_CP_CREATE:				return _T("Create IOCP Fail");
	case SE_WORKER_THREAD_CREATE:	return _T("Create Worker Thread Fail");
	case SE_DETECT_THREAD_CREATE:	return _T("Create Detector Thread Fail");
	case SE_SOCKE_ATTACH_TO_CP:		return _T("Attach SOCKET to IOCP Fail");
	case SE_CONNECT_SERVER:			return _T("Connect to Server Fail");
	case SE_NETWORK:				return _T("Network Error");
	case SE_DATA_PROC:				return _T("Process Data Error");
	case SE_DATA_SEND:				return _T("Send Data Fail");

	case SE_SSL_ENV_NOT_READY:		return _T("SSL environment not ready");

	default: ASSERT(FALSE);			return _T("UNKNOWN ERROR");
	}
}

VOID SysGetSystemInfo(LPSYSTEM_INFO pInfo)
{
	ASSERT(pInfo != nullptr);
	::GetNativeSystemInfo(pInfo);
}

DWORD SysGetNumberOfProcessors()
{
	SYSTEM_INFO si;
	SysGetSystemInfo(&si);

	return si.dwNumberOfProcessors;
}

DWORD GetDefaultWorkerThreadCount()
{
	static const DWORD s_dwtc = Min((::SysGetNumberOfProcessors() * 2 + 2), MAX_WORKER_THREAD_COUNT);
	return s_dwtc;
}