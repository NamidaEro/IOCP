#pragma once

#include "../Common/Macros.h"

#include "BaseRefObject.h"

class CTiocp : public CBaseRefObject
{
public:
	HResult Initialize() override;
	HResult Release() override;

public:
	HResult StartUP();
	HResult CreateIO();
};

