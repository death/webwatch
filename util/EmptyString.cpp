#include "stdafx.h"

#include "EmptyString.h"

namespace Util_DE050401
{
    namespace
    {
        std::string Empty; // Not defined as const for extra usability ;)
    }

    const std::string & EmptyString()
    {
        return Empty;
    }
}
