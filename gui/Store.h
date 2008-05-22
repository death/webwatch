// WebWatch UI - by DEATH, 2004
//
// $Workfile: Store.h $ - WebWatch data store
//
// $Author: Death $
// $Revision: 2 $
// $Date: 4/02/05 3:58 $
// $NoKeywords: $
//
#ifndef WEBWATCH_INC_STORE_H
#define WEBWATCH_INC_STORE_H

#include "loki/Singleton.h"

namespace WebWatch
{

class SiteItemGroup;
class MUSettings;
class CheckSettings;
struct GeneralSettings;

class StoreImpl
{
public:
    StoreImpl();
    ~StoreImpl();

    void Load(const char *filename);
    void Save(const char *filename) const;

    SiteItemGroup & GetRootGroup();
    const SiteItemGroup & GetRootGroup() const;
    bool IsRootGroup(const SiteItemGroup & group) const;

    void SetSiteListSettings(const XML::CXML & xml);
    void GetSiteListSettings(XML::CXML & xml) const;
    void SetDefaultSiteListSettings();

    void SetWindowPlacement(const WINDOWPLACEMENT & wp);
    void GetWindowPlacement(WINDOWPLACEMENT & wp) const;
    void SetDefaultWindowPlacement();

    void SetMUSettings(const MUSettings & settings);
    void GetMUSettings(MUSettings & settings) const;
    void SetDefaultMUSettings();

    void SetCheckSettings(const CheckSettings & settings);
    void GetCheckSettings(CheckSettings & settings) const;
    void SetDefaultCheckSettings();

    void SetSiteGroupsTreeSettings(const XML::CXML & xml);
    void GetSiteGroupsTreeSettings(XML::CXML & xml) const;
    void SetDefaultSiteGroupsTreeSettings();

    void ExportGroup(const SiteItemGroup & group, const char *filename) const;
    SiteItemGroup & ImportGroup(SiteItemGroup & group, const char *filename);

    void SetGeneralSettings(const GeneralSettings & settings);
    void GetGeneralSettings(GeneralSettings & settings) const;
    void SetDefaultGeneralSettings();

private:
    struct Impl;
    std::auto_ptr<Impl> m_pimpl;
};

typedef Loki::SingletonHolder<StoreImpl> Store;

class StoreError : public std::runtime_error
{
public:
    StoreError(const std::string & error)
    : std::runtime_error(error)
    {
    }
};

} 

#endif // WEBWATCH_INC_STORE_H