#include "BaseRefObject.h"
#include <string>
#include <iostream>

#include "UUID.h"

unsigned int CBaseRefObject::refcnt = 0;
std::map<const std::string, CBaseRefObject*> CBaseRefObject::objects;

CBaseRefObject::CBaseRefObject()
    : UUID(CUUID::generate_uuid())
{
}

CBaseRefObject* CBaseRefObject::Create()
{
    CBaseRefObject* result = new CBaseRefObject;
    refcnt++;

    CBaseRefObject::objects.emplace(result->GetUUID(), result);

    return result;
}

const std::string CBaseRefObject::GetUUID()
{
    return UUID;
}

HResult CBaseRefObject::Initialize()
{
    return HResult::HR_OK;
}

HResult CBaseRefObject::Release()
{
    try
    {
        if (0 < refcnt)
        {
            refcnt--;
        }
        else
        {
            throw "ref count is 0, Release Over Called";
        }

        return HResult::HR_OK;
    }
    catch (const std::string& str_exception)
    {
        std::cout << str_exception << std::endl;

        return HResult::HR_FAIL;
    }
}
