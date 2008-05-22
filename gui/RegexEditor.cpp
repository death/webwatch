// WebWatch UI - by DEATH, 2004
//
// $Workfile: RegexEditor.cpp $ - Regex editor
//
// $Author: Death $
// $Revision: 3 $
// $Date: 4/23/05 11:50 $
// $NoKeywords: $
//
#include "stdafx.h"

#include "xml/XmlUtil.h"

#include "core/Core.h"
#include "core/CoreException.h"

#include "RegexEditor.h"
#include "SiteItem.h"
#include "SiteOperationManager.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif // _DEBUG

namespace {
    const int MaxResults = 20;
    const int LongTextLength = 4096;
}

// CheckHook class definition

class CRegexEditor::CheckHook : public CSiteCheckDlg::Hook
{
public:
    explicit CheckHook(CRegexEditor & owner)
    : m_owner(owner)
    {
    }

    virtual void OnSiteChecked(WebWatch::SiteItem &, const XML::CXML &);
    virtual void OnException(WebWatch::SiteItem &, const Core::Exception &);

private:
    CRegexEditor & m_owner;
};
    
// Message map

BEGIN_MESSAGE_MAP(CRegexEditor, CDialog)
    ON_COMMAND(IDC_REFRESH, OnRefresh)
    ON_EN_CHANGE(IDC_REGEX, OnRegexChanged)
    ON_BN_CLICKED(IDC_WRAP_CONTENT, OnWrapContent)
END_MESSAGE_MAP()

// External interface

CRegexEditor::CRegexEditor(WebWatch::SiteItem & site, const XML::CXML & params_)
: CDialog(CRegexEditor::IDD)
, m_site(site)
, m_wrapContent(TRUE)
{
    XML::CXML params(params_);
    XML::CXML *mode = params.CreateChild("Mode");
    if (mode)
        mode->SetParam("Type", "GetContent");
    m_site.GetCheckParameters(m_oldParams);
    m_site.SetCheckParameters(params);
}

CRegexEditor::~CRegexEditor()
{
    m_site.SetCheckParameters(m_oldParams);
}

void CRegexEditor::SetRegex(const std::string & regex)
{
    m_regexString = regex;
}

std::string CRegexEditor::GetRegex() const
{
    return m_regexString;
}

// Overrides

void CRegexEditor::DoDataExchange(CDataExchange *dx)
{
    CDialog::DoDataExchange(dx);
    DDX_Control(dx, IDC_MATCH_RESULTS, m_matchResults);
    DDX_Check(dx, IDC_WRAP_CONTENT, m_wrapContent);
    DDX_Control(dx, IDC_WEBPAGE_CONTENT, m_webpageContent);
}

BOOL CRegexEditor::OnInitDialog()
{
    CDialog::OnInitDialog();

    const std::string & name = m_site.GetName();
    if (name.empty() == false) {
        CString caption;
        GetWindowText(caption);
        caption += " - ";
        caption += name.c_str();
        SetWindowText(caption);
    }

    CRect rect;
    m_matchResults.GetClientRect(&rect);
    m_matchResults.InsertColumn(0, "Result", LVCFMT_LEFT, rect.Width());
    m_matchResults.SetExtendedStyle(m_matchResults.GetExtendedStyle() | LVS_EX_FULLROWSELECT);

    SetDlgItemText(IDC_REGEX, m_regexString.c_str()); // calls OnRegexChanged()
    OnWrapContent();

    return TRUE;
}

void CRegexEditor::OnOK()
{
    if (m_regexString.empty() == true) {
        MessageBox("The expression must be non-empty", "Error", MB_OK | MB_ICONEXCLAMATION);
        return;
    }

    CDialog::OnOK();
}

// Messages

void CRegexEditor::OnRefresh()
{
    UpdateSiteContent();
}

void CRegexEditor::OnRegexChanged()
{
    CString regexString;
    GetDlgItemText(IDC_REGEX, regexString);
    m_regexString = static_cast<const char *>(regexString);
    UpdateMatchResults();
    GetDlgItem(IDOK)->EnableWindow(m_regexString.empty() ? FALSE : TRUE);
}

void CRegexEditor::OnWrapContent()
{
    // Windows is so braindead -- we need to recreate the edit box
    // in order to switch between word-wrap and non-word-wrap.

    UpdateData();

    DWORD style = m_webpageContent.GetStyle();
    DWORD prevStyle = style;
    const DWORD flags = ES_AUTOHSCROLL | WS_HSCROLL;
    if (m_wrapContent)
        style &= ~flags;
    else
        style |= flags;

    if (prevStyle == style)
        return;

    style &= ~WS_VISIBLE;

    int len = m_webpageContent.GetWindowTextLength();
    HCURSOR prevCursor = len > LongTextLength ? SetCursor(LoadCursor(0, IDC_WAIT)) : 0;

    if (m_wrapContent == FALSE)
        m_webpageContent.FmtLines(FALSE);
    
    CString text;
    m_webpageContent.GetWindowText(text);

    DWORD exStyle = m_webpageContent.GetExStyle();

    CRect rect;
    m_webpageContent.GetWindowRect(&rect);
    ScreenToClient(&rect);

    BOOL modified = m_webpageContent.GetModify();

    CEdit edit;
    if (edit.CreateEx(exStyle, WC_EDIT, text, style, rect, this, IDC_WEBPAGE_CONTENT)) {
        CFont *font = m_webpageContent.GetFont();
        edit.SetFont(font);
        edit.SetModify(modified);
        m_webpageContent.DestroyWindow();
        m_webpageContent.Attach(edit.Detach());
        m_webpageContent.ShowWindow(SW_SHOW);
    } else {
        if (m_wrapContent == FALSE)
            m_webpageContent.FmtLines(TRUE);
    }

    UpdateData(FALSE);

    if (prevCursor)
        SetCursor(prevCursor);
}

// Internal stuff

void CRegexEditor::UpdateSiteContent()
{
    std::vector<WebWatch::SiteItem *> sites;
    sites.push_back(&m_site);
    CheckHook hook(*this);
    WebWatch::SiteOperationManager::Check check(hook, this);
    if (check(sites.begin(), sites.end()) == false)
        m_content.clear();
    m_webpageContent.SetWindowText(m_content.c_str());
    UpdateData();
    UpdateMatchResults();
}

void CRegexEditor::UpdateMatchResults()
{
    m_matchResults.DeleteAllItems();

    if (m_regexString.empty()) {
        UpdateMatchResultsStats(0);
        return;
    }

    m_matchResults.SetRedraw(FALSE);
    int count = 0;

    try {
        boost::regex regex(m_regexString);
        std::string::const_iterator start = m_content.begin();
        std::string::const_iterator end = m_content.end();
        boost::match_results<std::string::const_iterator> what;
        boost::match_flag_type flags = boost::match_default;
        
        while (boost::regex_search(start, end, what, regex, flags)) {
            if (count >= MaxResults)
                break;

            std::string result(what[0].first, what[0].second);
            result = XML::MultilineToXML(result);
            m_matchResults.InsertItem(count, result.c_str());
            // Update search position
            start = what[0].second;
            // Update flags
            flags |= boost::match_prev_avail;
            flags |= boost::match_not_bob;
            
            count++;
        }
    } catch(const std::exception &) {
    }

    m_matchResults.SetRedraw();
    UpdateMatchResultsStats(count);
}

void CRegexEditor::UpdateMatchResultsStats(int count)
{
    std::ostringstream os;
    os << "Match results (";
    
    if (count == 0) {
        os << "No results";
    } else if (count == 1) {
        os << "One result";
    } else if (count >= MaxResults) {
        os << "Too many results, first " << m_matchResults.GetItemCount() << " shown";
    } else {
        os << count << " results";
    }

    os << "):";

    const std::string & stats = os.str();
    SetDlgItemText(IDC_MATCH_RESULTS_STATS, stats.c_str());
}

// CheckHook methods

void CRegexEditor::CheckHook::OnSiteChecked(WebWatch::SiteItem & site, const XML::CXML & results)
{
    const char *content = results.GetChildText("Content");
    m_owner.m_content = XML::MultilineFromXML(content);
}

void CRegexEditor::CheckHook::OnException(WebWatch::SiteItem & site, const Core::Exception & ex)
{
    m_owner.MessageBox(ex.what(), "Error", MB_OK | MB_ICONERROR);
}
