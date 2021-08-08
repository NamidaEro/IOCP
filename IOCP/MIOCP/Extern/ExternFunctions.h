#pragma once

#include "../Common/Macros.h"
#include "../Common/Enums.h"
#include "../Common/Functions.h"

#include "../Classes/BaseRefObject.h"
#include "../Classes/Tiocp.h"

SOCKET_API CBaseRefObject* CreateRefObject();

SOCKET_API CTiocp* CreateIOCPObject();