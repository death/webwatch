// WebWatch Util - by DEATH, 2004
//
// $Workfile: ThreadUtil.h $ - WebWatch thread utilities
//
// $Author: Death $
// $Revision: 2 $
// $Date: 4/02/05 3:58 $
// $NoKeywords: $
//
#ifndef WEBWATCH_INC_THREADUTIL_H
#define WEBWATCH_INC_THREADUTIL_H

#pragma warning(push)
#pragma warning(disable:4275 4251)
#include <boost/thread.hpp>
#pragma warning(pop)

namespace Util_DE050401
{
    typedef boost::recursive_mutex CriticalSection;
    typedef CriticalSection::scoped_lock ScopedLock;
}

#endif // WEBWATCH_INC_THREADUTIL_H