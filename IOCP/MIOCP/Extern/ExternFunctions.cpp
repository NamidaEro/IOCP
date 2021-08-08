#include "ExternFunctions.h"

SOCKET_API CBaseRefObject* CreateRefObject()
{
	return CBaseRefObject::Create();
}

SOCKET_API CTiocp* CreateIOCPObject()
{
	return (CTiocp*)CBaseRefObject::Create();
}