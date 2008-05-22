// WebWatch Core - by DEATH, 2004
//
// $Workfile: CheckMethodFactory.h $ - Check method factory
//
// $Author: Death $
// $Revision: 2 $
// $Date: 4/02/05 3:58 $
// $NoKeywords: $
//
#ifndef WEBWATCH_INC_CHECKMETHODFACTORY_H
#define WEBWATCH_INC_CHECKMETHODFACTORY_H

#include <string>
#include "loki/Factory.h"
#include "loki/Singleton.h"
#include "CheckMethod.h"

namespace Core_DE050401
{
    typedef Loki::SingletonHolder< Loki::Factory<CheckMethod, std::string > > CheckMethodFactory;
}

#endif // WEBWATCH_INC_CHECKMETHODFACTORY_H