#pragma once

#include "../Common/Macros.h"
#include "../Common/Enums.h"
#include "../Common/Functions.h"

#include "../Classes/BaseRefObject.h"
#include "../Classes/Tiocp.h"

SOCKET_API CBaseRefObject* CreateRefObject()
{
	return CBaseRefObject::Create();
}

SOCKET_API CTiocp* CreateIOCPObject()
{
	return (CTiocp*)CBaseRefObject::Create();
}