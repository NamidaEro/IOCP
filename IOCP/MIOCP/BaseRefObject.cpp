#include "BaseRefObject.h"
#include <string>
#include <iostream>

CBaseRefObject* CBaseRefObject::Create()
{
    auto result = new CBaseRefObject;
    refcnt++;

    return result;
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
