// WebWatch UI - by DEATH, 2004
//
// $Workfile: Store.cpp $ - WebWatch data store
//
// $Author: Death $
// $Revision: 5 $
// $Date: 8/07/05 21:58 $
// $NoKeywords: $
//
#include "stdafx.h"

#include "util/Util.h"
#include "util/ThreadUtil.h"

#include "Store.h"
#include "Colors.h"
#include "Common.h"
#include "SiteItemGroup.h"
#include "MU.h"
#include "CheckSettings.h"
#include "SiteList.h"
#include "SiteGroupsTree.h"
#include "GeneralSettings.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif // _DEBUG

namespace 
{
    const char WebWatchTag[] = "WebWatch";
    const char GroupExportTag[] = "GroupExport";
    const char WindowPlacementTag[] = "WindowPlacement";
    const char DefaultRootGroupName[] = "Root";
    const char DefaultStartGroupName[] = "Default";
    const char FlagsTag[] = "Flags";
    const char ShowCmdTag[] = "ShowCmd";
    const char MaxPositionTag[] = "MaxPosition";
    const char MinPositionTag[] = "MinPosition";
    const char NormalPositionTag[] = "NormalPosition";
    const char XTag[] = "X";
    const char YTag[] = "Y";
    const char TopTag[] = "Top";
    const char LeftTag[] = "Left";
    const char RightTag[] = "Right";
    const char BottomTag[] = "Bottom";

    void ConvertXMLToWindowPlacement(const XML::CXML & xml,
                                     WINDOWPLACEMENT & wp);
    void ConvertWindowPlacementToXML(const WINDOWPLACEMENT & wp,
                                     XML::CXML & xml);

    void AddPointXML(XML::CXML & parent, const char *name, const POINT & pt);
    void AddRectXML(XML::CXML & parent, const char *name, const RECT & rect);
    POINT GetPointXML(const XML::CXML & parent, const char *name);
    RECT GetRectXML(const XML::CXML & parent, const char *name);

    std::string FormatCantLoadError(const char *filename);
    std::string FormatRootGroupHasSitesWarning();
    std::string FormatCantSaveError(const char *filename);
    std::string FormatCantExportGroupError(const char *filename);
    std::string FormatCantImportGroupError(const char *filename);
    std::string FormatNoGroupExportInfoError(const char *filename);
    std::string FormatNoSiteGroupsError(const char *filename);
}

namespace WebWatch
{

struct StoreImpl::Impl
{
    std::auto_ptr<SiteItemGroup> rootGroup;
    XML::CXML siteListSettings;
    WINDOWPLACEMENT wp;
    std::auto_ptr<MUSettings> muSettings;
    std::auto_ptr<CheckSettings> checkSettings;
    XML::CXML siteGroupsTreeSettings;
    std::auto_ptr<GeneralSettings> generalSettings;
    Util::CriticalSection operationsCS;
};

StoreImpl::StoreImpl()
: m_pimpl(new Impl)
{
    m_pimpl->rootGroup.reset(new SiteItemGroup(DefaultRootGroupName));
    m_pimpl->rootGroup->AddGroup(*new SiteItemGroup(DefaultStartGroupName));
    SetDefaultSiteListSettings();
    SetDefaultWindowPlacement();
    SetDefaultMUSettings();
    SetDefaultCheckSettings();
    SetDefaultSiteGroupsTreeSettings();
    SetDefaultGeneralSettings();
}

StoreImpl::~StoreImpl()
{
}

SiteItemGroup & StoreImpl::GetRootGroup()
{
    ASSERT(m_pimpl->rootGroup.get());
    return *m_pimpl->rootGroup;
}

const SiteItemGroup & StoreImpl::GetRootGroup() const
{
    ASSERT(m_pimpl->rootGroup.get());
    return *m_pimpl->rootGroup;
}

bool StoreImpl::IsRootGroup(const SiteItemGroup & group) const
{
    ASSERT(m_pimpl->rootGroup.get());
    return &group == m_pimpl->rootGroup.get();
}

// MU settings

void StoreImpl::SetDefaultMUSettings()
{
    Util::ScopedLock lock(m_pimpl->operationsCS);
    m_pimpl->muSettings.reset(new MUSettings);
}

void StoreImpl::SetMUSettings(const MUSettings & settings)
{
    Util::ScopedLock lock(m_pimpl->operationsCS);
    if (m_pimpl->muSettings.get())
        *m_pimpl->muSettings = settings;
    else
        m_pimpl->muSettings.reset(new MUSettings(settings));
}

void StoreImpl::GetMUSettings(MUSettings & settings) const
{
    ASSERT(m_pimpl->muSettings.get());
    settings = *m_pimpl->muSettings;
}

// Check settings

void StoreImpl::SetDefaultCheckSettings()
{
    Util::ScopedLock lock(m_pimpl->operationsCS);
    m_pimpl->checkSettings.reset(new CheckSettings);
}

void StoreImpl::SetCheckSettings(const CheckSettings & settings)
{
    Util::ScopedLock lock(m_pimpl->operationsCS);
    if (m_pimpl->checkSettings.get())
        *m_pimpl->checkSettings = settings;
    else
        m_pimpl->checkSettings.reset(new CheckSettings(settings));
}

void StoreImpl::GetCheckSettings(CheckSettings & settings) const
{
    ASSERT(m_pimpl->checkSettings.get());
    settings = *m_pimpl->checkSettings;
}

// Site list settings

void StoreImpl::SetSiteListSettings(const XML::CXML & xml)
{
    Util::ScopedLock lock(m_pimpl->operationsCS);
    m_pimpl->siteListSettings = xml;
}

void StoreImpl::GetSiteListSettings(XML::CXML & xml) const
{
    xml = m_pimpl->siteListSettings;
}

void StoreImpl::SetDefaultSiteListSettings()
{
    Util::ScopedLock lock(m_pimpl->operationsCS);
    try {
        CSiteList::GetDefaultSettings(m_pimpl->siteListSettings);
    }
    catch (std::exception & ex) {
        throw StoreError(ex.what());
    }
}

// Window placement

void StoreImpl::SetWindowPlacement(const WINDOWPLACEMENT & wp)
{
    Util::ScopedLock lock(m_pimpl->operationsCS);
    m_pimpl->wp = wp;
}

void StoreImpl::GetWindowPlacement(WINDOWPLACEMENT & wp) const
{
    wp = m_pimpl->wp;
}

void StoreImpl::SetDefaultWindowPlacement()
{
    Util::ScopedLock lock(m_pimpl->operationsCS);
    memset(&m_pimpl->wp, 0, sizeof(WINDOWPLACEMENT));
    m_pimpl->wp.length = sizeof(WINDOWPLACEMENT);
    m_pimpl->wp.showCmd = SW_SHOWMAXIMIZED;
    m_pimpl->wp.ptMaxPosition.x = -1;
    m_pimpl->wp.ptMaxPosition.y = -1;
    m_pimpl->wp.ptMinPosition.x = -1;
    m_pimpl->wp.ptMinPosition.y = -1;
    m_pimpl->wp.rcNormalPosition.top = 50;
    m_pimpl->wp.rcNormalPosition.bottom = 400;
    m_pimpl->wp.rcNormalPosition.left = 50;
    m_pimpl->wp.rcNormalPosition.right = 400;
}

// Saving and loading

void StoreImpl::Load(const char *filename)
{
    Util::ScopedLock lock(m_pimpl->operationsCS);

    XML::CXML root;
    if (root.Load(filename, WebWatchTag) == false)
        throw StoreError(FormatCantLoadError(filename));
    
    if (XML::CXML *xml = root.FindFirstChild(SiteItemGroup::GetXMLName())) {
        m_pimpl->rootGroup.reset(new SiteItemGroup(*xml));
        if (m_pimpl->rootGroup->GetSiteCount() != 0) {
            MessageBox(0, FormatRootGroupHasSitesWarning().c_str(), 
                       "Warning", MB_OK | MB_ICONEXCLAMATION);
            
            m_pimpl->rootGroup->DeleteAllSites();
        }
    } else {
        m_pimpl->rootGroup.reset(new SiteItemGroup(DefaultRootGroupName));
    }

    if (XML::CXML *xml = root.FindFirstChild(CSiteList::GetXMLName()))
        m_pimpl->siteListSettings = *xml;
    else
        SetDefaultSiteListSettings();

    if (XML::CXML *xml = root.FindFirstChild(WindowPlacementTag))
        ConvertXMLToWindowPlacement(*xml, m_pimpl->wp);
    else
        SetDefaultWindowPlacement();

    if (XML::CXML *xml = root.FindFirstChild(MUSettings::GetXMLName()))
        m_pimpl->muSettings.reset(new MUSettings(*xml));
    else
        SetDefaultMUSettings();

    if (XML::CXML *xml = root.FindFirstChild(CheckSettings::GetXMLName()))
        m_pimpl->checkSettings.reset(new CheckSettings(*xml));
    else
        SetDefaultCheckSettings();

    if (XML::CXML *xml = root.FindFirstChild(CSiteGroupsTree::GetXMLName()))
        m_pimpl->siteGroupsTreeSettings = *xml;
    else
        SetDefaultSiteGroupsTreeSettings();

    if (XML::CXML *xml = root.FindFirstChild(GeneralSettings::GetXMLName()))
        m_pimpl->generalSettings.reset(new GeneralSettings(*xml));
    else
        SetDefaultGeneralSettings();
}

void StoreImpl::Save(const char *filename) const
{
    Util::ScopedLock lock(m_pimpl->operationsCS);

    XML::CXML xml(WebWatchTag);
    
    xml << *m_pimpl->rootGroup;
    
    xml << m_pimpl->siteListSettings;
    
    XML::CXML *xmlWindowPlacement = xml.CreateChild(WindowPlacementTag);
    ConvertWindowPlacementToXML(m_pimpl->wp, *xmlWindowPlacement);
    
    ASSERT(m_pimpl->muSettings.get());
    xml << *m_pimpl->muSettings;
    
    ASSERT(m_pimpl->checkSettings.get());
    xml << *m_pimpl->checkSettings;

    xml << m_pimpl->siteGroupsTreeSettings;

    ASSERT(m_pimpl->generalSettings.get());
    xml << *m_pimpl->generalSettings;

    // Make a per-save backup of configuration file
    if (IsFileExistent(filename)) {
        MakeBackup(filename, perSave, 
                   AfxGetApp()->GetMainWnd()->GetSafeHwnd());
    }

    if (xml.SaveAs(filename) == false)
        throw StoreError(FormatCantSaveError(filename));
}

void StoreImpl::ExportGroup(const SiteItemGroup & group, 
                            const char *filename) const
{
    Util::ScopedLock lock(m_pimpl->operationsCS);

    XML::CXML xml(WebWatchTag);
    xml.SetParam("Type", GroupExportTag);

    xml << group;

    if (xml.SaveAs(filename) == false)
        throw StoreError(FormatCantExportGroupError(filename));
}

SiteItemGroup & StoreImpl::ImportGroup(SiteItemGroup & group, 
                                       const char *filename)
{
    Util::ScopedLock lock(m_pimpl->operationsCS);

    XML::CXML xml;
    if (xml.Load(filename, WebWatchTag) == false)
        throw StoreError(FormatCantImportGroupError(filename));

    if (strcmp(xml.GetParamText("Type"), GroupExportTag) != 0)
        throw StoreError(FormatNoGroupExportInfoError(filename));

    XML::CXML *xmlGroup = xml.FindFirstChild(SiteItemGroup::GetXMLName());
    if (xmlGroup == 0)
        throw StoreError(FormatNoSiteGroupsError(filename));

    SiteItemGroup *newGroup = new SiteItemGroup(*xmlGroup);
    group.AddGroup(*newGroup);

    return *newGroup;
}

// Site groups tree settings

void StoreImpl::SetSiteGroupsTreeSettings(const XML::CXML & xml)
{
    Util::ScopedLock lock(m_pimpl->operationsCS);
    m_pimpl->siteGroupsTreeSettings = xml;
}

void StoreImpl::GetSiteGroupsTreeSettings(XML::CXML & xml) const
{
    xml = m_pimpl->siteGroupsTreeSettings;
}

void StoreImpl::SetDefaultSiteGroupsTreeSettings()
{
    Util::ScopedLock lock(m_pimpl->operationsCS);
    try {
        CSiteGroupsTree::GetDefaultSettings(m_pimpl->siteGroupsTreeSettings);
    }
    catch (std::exception & ex) {
        throw StoreError(ex.what());
    }
}

// General settings

void StoreImpl::SetDefaultGeneralSettings()
{
    Util::ScopedLock lock(m_pimpl->operationsCS);
    m_pimpl->generalSettings.reset(new GeneralSettings);
}

void StoreImpl::SetGeneralSettings(const GeneralSettings & settings)
{
    Util::ScopedLock lock(m_pimpl->operationsCS);
    if (m_pimpl->generalSettings.get())
        *m_pimpl->generalSettings = settings;
    else
        m_pimpl->generalSettings.reset(new GeneralSettings(settings));
}

void StoreImpl::GetGeneralSettings(GeneralSettings & settings) const
{
    ASSERT(m_pimpl->generalSettings.get());
    settings = *m_pimpl->generalSettings;
}

} 

namespace {
    void ConvertXMLToWindowPlacement(const XML::CXML & xml,
                                     WINDOWPLACEMENT & wp)
    {
        memset(&wp, 0, sizeof(WINDOWPLACEMENT));
        wp.length = sizeof(WINDOWPLACEMENT);
        wp.flags = static_cast<UINT>(xml.GetParamDWORD(FlagsTag));
        wp.showCmd = static_cast<UINT>(xml.GetParamDWORD(ShowCmdTag, 
                                                         SW_SHOWMAXIMIZED));
        wp.ptMaxPosition = GetPointXML(xml, MaxPositionTag);
        wp.ptMinPosition = GetPointXML(xml, MinPositionTag);
        wp.rcNormalPosition = GetRectXML(xml, NormalPositionTag);
    }

    void ConvertWindowPlacementToXML(const WINDOWPLACEMENT & wp, 
                                     XML::CXML & xml)
    {
        xml.SetParam(FlagsTag, wp.flags);
        xml.SetParam(ShowCmdTag, wp.showCmd);
        AddPointXML(xml, MaxPositionTag, wp.ptMaxPosition);
        AddPointXML(xml, MinPositionTag, wp.ptMinPosition);
        AddRectXML(xml, NormalPositionTag, wp.rcNormalPosition);
    }

    void AddPointXML(XML::CXML & parent, const char *name, const POINT & pt)
    {
        if (XML::CXML *xml = parent.CreateChild(name)) {
            xml->SetParam(XTag, pt.x);
            xml->SetParam(YTag, pt.y);
        }
    }

    void AddRectXML(XML::CXML & parent, const char *name, const RECT & rect)
    {
        if (XML::CXML *xml = parent.CreateChild(name)) {
            xml->SetParam(TopTag, rect.top);
            xml->SetParam(LeftTag, rect.left);
            xml->SetParam(BottomTag, rect.bottom);
            xml->SetParam(RightTag, rect.right);
        }
    }

    POINT GetPointXML(const XML::CXML & parent, const char *name)
    {
        POINT result = {-1, -1};

        if (const XML::CXML *xml = parent.FindFirstChild(name)) {
            result.x = xml->GetParamLong(XTag, result.x);
            result.y = xml->GetParamLong(YTag, result.y);
        }

        return result;
    }

    RECT GetRectXML(const XML::CXML & parent, const char *name)
    {
        RECT result = {0, 0, 0, 0};

        if (const XML::CXML *xml = parent.FindFirstChild(name)) {
            result.top = xml->GetParamLong(TopTag, result.top);
            result.left = xml->GetParamLong(LeftTag, result.left);
            result.bottom = xml->GetParamLong(BottomTag, result.bottom);
            result.right = xml->GetParamLong(RightTag, result.right);
        }

        return result;
    }

    std::string FormatCantLoadError(const char *filename)
    {
        return std::string("Can't load ") + filename;
    }

    std::string FormatRootGroupHasSitesWarning()
    {
        return "The root group contains one or more sites.\n"
            "WebWatch expects the root group to contain only\n"
            "sub-groups. Therefore, WebWatch will now delete\n"
            "the sites.";
    }

    std::string FormatCantSaveError(const char *filename)
    {
        return std::string("Can't save ") + filename;
    }

    std::string FormatCantExportGroupError(const char *filename)
    {
        return std::string("Can't export group to ") + filename;
    }

    std::string FormatCantImportGroupError(const char *filename)
    {
        return std::string("Can't import group from ") + filename;
    }

    std::string FormatNoGroupExportInfoError(const char *filename)
    {
        return filename 
            + std::string(" doesn't contain group export information.");
    }

    std::string FormatNoSiteGroupsError(const char *filename)
    {
        return std::string("No site groups in ") + filename;
    }
}
