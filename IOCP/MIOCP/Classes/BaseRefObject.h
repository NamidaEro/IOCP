#pragma once

#include "../Common/Enums.h"
#include "../Common/Functions.h"

#include <string>
#include <map>

class CBaseRefObject
{
public:
	~CBaseRefObject() { Release(); }

private:
	static unsigned int refcnt;

	std::string UUID;

protected:
	static std::map<const std::string, CBaseRefObject*> objects;

protected:
	CBaseRefObject();

public:
	static CBaseRefObject* Create();

public:
	const std::string GetUUID();

public:
	virtual HResult Initialize();
	virtual HResult Release();

};