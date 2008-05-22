// WebWatch UI - by DEATH, 2004
//
// $Workfile: SiteItem.cpp $ - WebWatch site item
//
// $Author: Death $
// $Revision: 2 $
// $Date: 4/02/05 3:58 $
// $NoKeywords: $
//
#include "stdafx.h"

#include "xml/XmlUtil.h"

#include "util/Util.h"
#include "util/TimeUtil.h"

#include "SiteItem.h"
#include "SiteItemGroup.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif // _DEBUG

namespace WebWatch
{

struct SiteItem::Impl
{
    explicit Impl(const char *name)
    : checkedByDefault(true)
    , state(none)
    , muExpiry(0)
    , status("OK")
    , parent(0)
    , name(name)
    {
    }

    Impl(const Impl & other)
    : checkedByDefault(other.checkedByDefault)
    , status(other.status)
    , state(other.state)
    , muExpiry(other.muExpiry)
    , parent(0)
    , name(other.name)
    , notes(other.notes)
    {
    }

    Impl(const XML::CXML & xml);

    bool checkedByDefault;
    std::string status;
    SiteItem::State state;
    time_t muExpiry;
    SiteItemGroup *parent;
    std::string name;
    std::string notes;
};

SiteItem::SiteItem(const char *name, const char *address)
: Core::Site(address)
, m_pimpl(new Impl(name))
{
}

SiteItem::SiteItem(const XML::CXML & xml)
: Core::Site(xml)
, m_pimpl(new Impl(xml))
{
}

SiteItem::SiteItem(const SiteItem & other)
: Core::Site(other)
, m_pimpl(new Impl(*other.m_pimpl))
{
}

SiteItem & SiteItem::operator= (const SiteItem & rhs)
{
    if (this != &rhs) {
        Core::Site::operator=(rhs);
        std::auto_ptr<Impl> newImpl(new Impl(*rhs.m_pimpl));
        std::swap(m_pimpl, newImpl);
        std::swap(m_pimpl->parent, newImpl->parent);
    }
    return *this;
}

SiteItem::~SiteItem()
{
}

void SiteItem::SetCheckedByDefault(bool set)
{
    m_pimpl->checkedByDefault = set;
}

bool SiteItem::IsCheckedByDefault() const
{
    return m_pimpl->checkedByDefault;
}

void SiteItem::SetVerboseStatus(const char *status)
{
    m_pimpl->status = status;
}

std::string SiteItem::GetVerboseStatus() const
{
    return m_pimpl->status;
}

void SiteItem::SetState(SiteItem::State state)
{
    m_pimpl->state = state;
    switch (m_pimpl->state) {
        case updated:
            SetVerboseStatus("Updated");
            break;
        case erroneous:
            SetVerboseStatus("Erroneous");
            break;
        case none:
            SetVerboseStatus("OK");
            break;
        default:
            SetVerboseStatus("Unknown");
            ASSERT(!"Unknown site state");
            break;
    }
}

SiteItem::State SiteItem::GetState() const
{
    return m_pimpl->state;
}

void SiteItem::SetMU(time_t expiry)
{
    m_pimpl->muExpiry = expiry;
}

void SiteItem::ResetMU()
{
    ASSERT(IsMUInfoAvailable());
    m_pimpl->muExpiry = 0;
}

bool SiteItem::IsMUInfoAvailable() const
{
    return m_pimpl->muExpiry != 0;
}

int SiteItem::GetMUDayCount(time_t now) const
{
    ASSERT(IsMUInfoAvailable());
    
    double diff = difftime(m_pimpl->muExpiry, now) / 60.0 / 60.0 / 24.0;
    if (diff > -1.0)
        diff++;
    
    return static_cast<int>(diff);
}

const char *SiteItem::GetXMLName()
{
    return Site::GetXMLName();
}

XML::CXML *SiteItem::Write(XML::CXML & xmlParent) const
{
    XML::CXML *xml = Site::Write(xmlParent);

    if (xml) {
        XML::CXML *xmlUISettings = xml->CreateChild("UISettings");
        if (xmlUISettings) {
            xmlUISettings->CreateChild("Name", m_pimpl->name.c_str());

            if (m_pimpl->notes.empty() == false) {
                std::string notes = XML::MultilineToXML(m_pimpl->notes);
                xmlUISettings->CreateChild("Notes", notes.c_str());
            }

            xmlUISettings->SetParam("CheckedByDefault", m_pimpl->checkedByDefault);
            
            if (m_pimpl->state == updated)
                xmlUISettings->SetParam("State", "Updated");

            if (IsMUInfoAvailable()) {
                struct tm *gmt = gmtime(&m_pimpl->muExpiry);
                if (gmt) {
                    if (Util::CreateXMLTimeNode(*xmlUISettings, "MinorUpdate", *gmt) == 0)
                        ASSERT(!"Failed to create XML time node (MinorUpdate)");
                } else {
                    ASSERT(!"Weird MU value, how did I get this?");
                }
            }
        }
    }

    return xml;
}

SiteItemGroup *SiteItem::GetParentGroup() const
{
    return m_pimpl->parent;
}

void SiteItem::SetParentGroup(SiteItemGroup *group)
{
    m_pimpl->parent = group;
}

void SiteItem::SetName(const char *name)
{
    m_pimpl->name = name;
}

std::string SiteItem::GetName() const
{
    return m_pimpl->name;
}

void SiteItem::SetNotes(const char *notes)
{
    m_pimpl->notes = notes;
}

std::string SiteItem::GetNotes() const
{
    return m_pimpl->notes;
}

SiteItem::Impl::Impl(const XML::CXML & xml)
: checkedByDefault(true)
, muExpiry(0)
, state(none)
, status("OK")
, parent(0)
{
    const XML::CXML *xmlUISettings = xml.FindFirstChild("UISettings");
    if (xmlUISettings == 0)
        return;

    name = xmlUISettings->GetChildText("Name");

    const XML::CXML *xmlNotes = xmlUISettings->FindFirstChild("Notes");
    if (xmlNotes)
        notes = XML::MultilineFromXML(xmlNotes->GetText());

    checkedByDefault = xmlUISettings->GetParamBool("CheckedByDefault", true);
    
    const char *state_ = xmlUISettings->GetParamText("State");
    if (state_ && strcmp(state_, "Updated") == 0) {
        state = updated;
        status = "Updated";
    } else {
        state = none;
    }

    if (xmlUISettings->FindFirstChild("MinorUpdate")) {
        struct tm tm;
        Util::ParseXMLTimeNode(*xmlUISettings, "MinorUpdate", tm);
        Util::GmtToLocal(tm);
        muExpiry = mktime(&tm);
    }
}

bool operator== (const SiteItem & lhs, const SiteItem & rhs)
{
    if (&lhs == &rhs)
        return true;

    if (lhs.m_pimpl->name != rhs.m_pimpl->name)
        return false;

    return static_cast<const Core::Site &>(lhs) == static_cast<const Core::Site &>(rhs);
}

bool operator!= (const SiteItem & lhs, const SiteItem & rhs)
{
    return !(lhs == rhs);
}

} 