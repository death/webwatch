// WebWatch UI - by DEATH, 2004
//
// $Workfile: HttpContentChild.h $ - Site Check property page - HTTP content child
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

class CHttpContentChild : public CDialog
{
public:
    explicit CHttpContentChild(WebWatch::SiteItem & site);

    enum { IDD = IDD_CHILD_HTTP_CONTENT };

    void SetParamsToModify(XML::CXML & params);

protected:
    virtual BOOL OnInitDialog();
    virtual void OnOK();
    virtual void OnCancel();

    afx_msg void OnStripNumbersChanged();
    afx_msg void OnStripHTMLChanged();
    afx_msg void OnSkipWhitespaceChanged();
    afx_msg void OnSkipScriptChanged();
    afx_msg void OnRegexpsItemChanged(NMHDR *pNMHDR, LRESULT *pResult);
    afx_msg void OnRegexpsEndLabelEdit(NMHDR *pNMHDR, LRESULT *pResult);
    afx_msg void OnRegexpsBeginLabelEdit(NMHDR *pNMHDR, LRESULT *pResult);
    afx_msg void OnAddRegexp();
    afx_msg void OnRemoveRegexp();
    afx_msg void OnEditRegexp();

    void UpdateParams();
    void UpdateFilterButtons();
    void UpdateRegexps();
    void UpdateRegexpButtons();

private:
    XML::CXML *FindFilter(const char *method);
    void ChangeRegex(const std::string & oldRegex, const std::string & newRegex);

    virtual void DoDataExchange(CDataExchange *pDX);

    DECLARE_MESSAGE_MAP()

private:
    XML::CXML *m_params;
    CListCtrl m_regexps;
    std::string m_sourceRegexp;
    WebWatch::SiteItem & m_site;
};