#include "Tiocp.h"
#include <WinSock2.h>

#pragma comment(lib, "ws2_32")

HResult CTiocp::Initialize()
{
	return HResult::HR_OK;
}

HResult CTiocp::Release()
{
	return HResult::HR_OK;
}

HResult CTiocp::StartUP()
{
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
	{
		// failed
		return HResult::HR_FAIL;
	}

	return HResult::HR_OK;
}

HResult CTiocp::CreateIO()
{
	HANDLE hcp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
	if (hcp == nullptr)
	{
		// failed
		return HResult::HR_FAIL;
	}

	return HResult::HR_OK;
}
