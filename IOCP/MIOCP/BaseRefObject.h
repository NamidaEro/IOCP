#pragma once

#include "Common/Enums.h";

class CBaseRefObject
{
public:
	~CBaseRefObject() { Release(); }

private:
	static unsigned int refcnt;

protected:
	CBaseRefObject* instance;

private:
	CBaseRefObject() :instance(nullptr) {}

public:
	static CBaseRefObject* Create();

public:
	virtual HResult Initialize();
	virtual HResult Release();

};

