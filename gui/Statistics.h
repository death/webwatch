// WebWatch UI - by DEATH, 2004
//
// $Workfile: Statistics.h $ - Statistics provider
//
// $Author: Death $
// $Revision: 2 $
// $Date: 4/02/05 3:58 $
// $NoKeywords: $
//
#ifndef WEBWATCH_INC_STATISTICS_H
#define WEBWATCH_INC_STATISTICS_H

#include "SiteItem.h"

namespace WebWatch
{

class SiteItemGroup;

namespace Statistics
{

enum CountType
{
    directCount, deepCount, recursiveCount
};

int GetSiteCountByState(const SiteItemGroup & group, SiteItem::State state, CountType type);
int GetMUSiteCount(const SiteItemGroup & group, CountType type);

}

} 

#endif // WEBWATCH_INC_STATISTICS_H
