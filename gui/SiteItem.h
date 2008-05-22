// WebWatch UI - by DEATH, 2004
//
// $Workfile: SiteItem.h $ - WebWatch site item
//
// $Author: Death $
// $Revision: 2 $
// $Date: 4/02/05 3:58 $
// $NoKeywords: $
//
#ifndef WEBWATCH_INC_SITEITEM_H
#define WEBWATCH_INC_SITEITEM_H

#include "core/Core.h"
#include "core/Site.h"

namespace WebWatch
{

class SiteItem : public Core::Site
{
    friend class SiteItemGroup;
    friend bool operator== (const SiteItem & lhs, const SiteItem & rhs);
    friend bool operator!= (const SiteItem & lhs, const SiteItem & rhs);

public:
    // The states are in increasing 'importance' order
    enum State
    {
        none, erroneous, updated
    };

public:
    SiteItem(const char *name, const char *address);
    SiteItem(const SiteItem & other);
    explicit SiteItem(const XML::CXML & xml);
    virtual ~SiteItem();

    SiteItem & operator= (const SiteItem & rhs);
    
    void SetName(const char *name);
    std::string GetName() const;

    void SetNotes(const char *notes);
    std::string GetNotes() const;

    void SetCheckedByDefault(bool set);
    bool IsCheckedByDefault() const;

    void SetVerboseStatus(const char *status);
    std::string GetVerboseStatus() const;
    
    void SetState(State state);
    State GetState() const;

    void SetMU(time_t expiry);
    void ResetMU();
    bool IsMUInfoAvailable() const;
    int GetMUDayCount(time_t now) const;

    SiteItemGroup *GetParentGroup() const;
    
    static const char *GetXMLName();

protected:
    virtual XML::CXML *Write(XML::CXML & xmlParent) const;

    void SetParentGroup(SiteItemGroup *group);

private:
    struct Impl;
    std::auto_ptr<Impl> m_pimpl;
};

class SiteItemError : public Core::SiteError
{
public:
    SiteItemError(const std::string & error)
    : Core::SiteError(error)
    {
    }
};

} 

#endif // WEBWATCH_INC_SITEITEM_H