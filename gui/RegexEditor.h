// WebWatch UI - by DEATH, 2004
//
// $Workfile: RegexEditor.h $ - Regex editor
//
// $Author: Death $
// $Revision: 3 $
// $Date: 4/23/05 11:50 $
// $NoKeywords: $
//
#pragma once

#include "Resource.h"

namespace WebWatch
{
    class SiteItem;
}

class CRegexEditor : public CDialog
{
    DECLARE_MESSAGE_MAP()

    class CheckHook;

public:
    CRegexEditor(WebWatch::SiteItem & site, const XML::CXML & params_);
    virtual ~CRegexEditor();

    enum { IDD = IDD_REGEX_EDITOR };

    void SetRegex(const std::string & regex);
    std::string GetRegex() const;

public:
    virtual BOOL OnInitDialog();
    virtual void DoDataExchange(CDataExchange *dx);
    virtual void OnOK();
    
    afx_msg void OnRefresh();
    afx_msg void OnRegexChanged();
    afx_msg void OnWrapContent();

private:
    void UpdateSiteContent();
    void UpdateMatchResults();
    void UpdateMatchResultsStats(int count);

private:
    WebWatch::SiteItem & m_site;
    XML::CXML m_oldParams;
    std::string m_regexString;
    std::string m_content;
    CListCtrl m_matchResults;
    BOOL m_wrapContent;
    CEdit m_webpageContent;
};
