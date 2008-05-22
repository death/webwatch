// WebWatch UI - by DEATH, 2004
//
// $Workfile: HttpContentChild.cpp $ - Site Check property page - HTTP content child
//
// $Author: Death $
// $Revision: 3 $
// $Date: 4/23/05 11:50 $
// $NoKeywords: $
//
#include "stdafx.h"

#include "HttpContentChild.h"
#include "SiteCheckPage.h"
#include "RegexEditor.h"
#include "SiteItem.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif // _DEBUG

BEGIN_MESSAGE_MAP(CHttpContentChild, CDialog)
    ON_BN_CLICKED(IDC_STRIP_NUMBERS, OnStripNumbersChanged)
    ON_BN_CLICKED(IDC_STRIP_HTML, OnStripHTMLChanged)
    ON_BN_CLICKED(IDC_SKIP_WHITESPACE, OnSkipWhitespaceChanged)
    ON_BN_CLICKED(IDC_SKIP_SCRIPT, OnSkipScriptChanged)
    ON_NOTIFY(LVN_ITEMCHANGED, IDC_REGEXPS, OnRegexpsItemChanged)
    ON_NOTIFY(LVN_ENDLABELEDIT, IDC_REGEXPS, OnRegexpsEndLabelEdit)
    ON_NOTIFY(LVN_BEGINLABELEDIT, IDC_REGEXPS, OnRegexpsBeginLabelEdit)
    ON_BN_CLICKED(IDC_ADD_REGEX, OnAddRegexp)
    ON_BN_CLICKED(IDC_REMOVE_REGEX, OnRemoveRegexp)
    ON_BN_CLICKED(IDC_EDIT_REGEX, OnEditRegexp)
END_MESSAGE_MAP()

CHttpContentChild::CHttpContentChild(WebWatch::SiteItem & site)
: m_params(0)
, m_site(site)
{
}

void CHttpContentChild::DoDataExchange(CDataExchange *pDX)
{
    DDX_Control(pDX, IDC_REGEXPS, m_regexps);
    CDialog::DoDataExchange(pDX);
}

BOOL CHttpContentChild::OnInitDialog()
{
    CDialog::OnInitDialog();
    return TRUE;
}

void CHttpContentChild::OnStripNumbersChanged()
{
    UpdateParams();
}

void CHttpContentChild::OnStripHTMLChanged()
{
    BOOL checked = IsDlgButtonChecked(IDC_STRIP_HTML) != BST_UNCHECKED ? TRUE : FALSE;
    
    GetDlgItem(IDC_HTML_STRIP_OPTIONS_GROUP)->EnableWindow(checked);
    GetDlgItem(IDC_SKIP_WHITESPACE)->EnableWindow(checked);
    GetDlgItem(IDC_SKIP_SCRIPT)->EnableWindow(checked);

    UpdateParams();
}

void CHttpContentChild::OnSkipScriptChanged()
{
    UpdateParams();
}

void CHttpContentChild::OnSkipWhitespaceChanged()
{
    UpdateParams();
}

void CHttpContentChild::SetParamsToModify(XML::CXML & params)
{
    m_params = &params;
    UpdateFilterButtons();
    UpdateRegexps();
    UpdateRegexpButtons();
}

void CHttpContentChild::UpdateParams()
{
    if (m_params == 0)
        return;

    bool stripNumbers = IsDlgButtonChecked(IDC_STRIP_NUMBERS) != BST_UNCHECKED;
    bool stripHtml = IsDlgButtonChecked(IDC_STRIP_HTML) != BST_UNCHECKED;
    bool skipScript = IsDlgButtonChecked(IDC_SKIP_SCRIPT) != BST_UNCHECKED;
    bool skipWhitespace = IsDlgButtonChecked(IDC_SKIP_WHITESPACE) != BST_UNCHECKED;

    const char * const filterStr = "Filter";
    const char * const methodStr = "Method";
    const char * const stripNumbersStr = "StripNumbers";

    XML::CXML *filter;
    
    if (stripNumbers) {
        filter = FindFilter(stripNumbersStr);
        if (filter == 0) {
            filter = m_params->CreateChild(filterStr);
            ASSERT(filter);
            filter->SetParam(methodStr, stripNumbersStr);
        }
    } else {
        filter = m_params->FindFirstChild(filterStr);
        while (filter) {
            XML::CXML *next = m_params->FindNextChild(filterStr);
            const char *method = filter->GetParamText(methodStr);
            if (strcmp(method, stripNumbersStr) == 0)
                m_params->RemoveChild(filter);
            filter = next;
        }
    }

    const char * const stripHtmlStr = "StripHtml";

    if (stripHtml) {
        filter = FindFilter(stripHtmlStr);
        if (filter == 0) {
            filter = m_params->CreateChild(filterStr);
            ASSERT(filter);
            filter->SetParam(methodStr, stripHtmlStr);
        }

        ASSERT(filter);
        filter->SetParam("SkipWhiteSpace", skipWhitespace);
        filter->SetParam("SkipScript", skipScript);
    } else {
        filter = m_params->FindFirstChild(filterStr);
        while (filter) {
            XML::CXML *next = m_params->FindNextChild(filterStr);
            const char *method = filter->GetParamText(methodStr);
            if (strcmp(method, stripHtmlStr) == 0)
                m_params->RemoveChild(filter);
            filter = next;
        }
    }
}

void CHttpContentChild::UpdateFilterButtons()
{
    UINT checkStripNumbers = BST_UNCHECKED;
    UINT checkStripHtml = BST_UNCHECKED;
    UINT checkSkipScript = BST_UNCHECKED;
    UINT checkSkipWhitespace = BST_UNCHECKED;

    const char * const filterStr = "Filter";
    const char * const methodStr = "Method";
    const char * const stripNumbersStr = "StripNumbers";
    const char * const stripHtmlStr = "StripHtml";

    if (m_params) {
        XML::CXML *filter = m_params->FindFirstChild(filterStr);
        while (filter) {
            XML::CXML *next = m_params->FindNextChild(filterStr);
            const char *method = filter->GetParamText(methodStr);
            if (strcmp(method, stripNumbersStr) == 0) {
                checkStripNumbers = BST_CHECKED;
            } else if (strcmp(method, stripHtmlStr) == 0) {
                checkStripHtml = BST_CHECKED;
                checkSkipScript = filter->GetParamBool("SkipScript") ? BST_CHECKED : BST_UNCHECKED;
                checkSkipWhitespace = filter->GetParamBool("SkipWhiteSpace") ? BST_CHECKED : BST_UNCHECKED;
            }
            filter = next;
        }
    }

    CheckDlgButton(IDC_STRIP_NUMBERS, checkStripNumbers);
    CheckDlgButton(IDC_STRIP_HTML, checkStripHtml);
    CheckDlgButton(IDC_SKIP_SCRIPT, checkSkipScript);
    CheckDlgButton(IDC_SKIP_WHITESPACE, checkSkipWhitespace);

    OnStripNumbersChanged();
    OnStripHTMLChanged();
    OnSkipScriptChanged();
    OnSkipWhitespaceChanged();
}

XML::CXML *CHttpContentChild::FindFilter(const char *method)
{
    if (m_params == 0)
        return 0;

    XML::CXML *filter = m_params->FindFirstChild("Filter");
    while (filter) {
        const char *m = filter->GetParamText("Method");
        if (strcmp(m, method) == 0)
            return filter;
        filter = m_params->FindNextChild("Filter");
    }

    return 0;
}

void CHttpContentChild::ChangeRegex(const std::string & oldRegex, const std::string & newRegex)
{
    //
    // Known bug:
    //
    // When multiple regex strings are the same, the wrong regex entry
    // may change.
    //

    XML::CXML *regexp = m_params->FindFirstChild("Regex");
    while (regexp) {
        if (oldRegex == regexp->GetText()) {
            regexp->SetText(newRegex.c_str());
            break;
        }
        regexp = m_params->FindNextChild("Regex");
    }
}

void CHttpContentChild::OnOK()
{
}

void CHttpContentChild::OnCancel()
{
}

void CHttpContentChild::UpdateRegexps()
{
    if (m_regexps.GetHeaderCtrl()->GetItemCount() == 0) {
        CRect rect;
        m_regexps.GetClientRect(&rect);
        int totalWidth = rect.Width() - ::GetSystemMetrics(SM_CXVSCROLL);
        m_regexps.InsertColumn(0, "Regex", LVCFMT_LEFT, totalWidth / 2);
        m_regexps.InsertColumn(1, "Result", LVCFMT_LEFT, totalWidth / 2);
        m_regexps.SetExtendedStyle(m_regexps.GetExtendedStyle() | LVS_EX_FULLROWSELECT);
    }

    m_regexps.DeleteAllItems();

    XML::CXML *regexp = m_params->FindFirstChild("Regex");
    int item = 0;
    typedef stdext::hash_map<std::string, int> RegexToItem;
    RegexToItem regexToItem;
    while (regexp) {
        regexToItem.insert(std::make_pair(regexp->GetText(), item));
        m_regexps.InsertItem(item++, regexp->GetText());
        regexp = m_params->FindNextChild("Regex");
    }

    if (m_site.AreCheckResultsAvailable()) {
        XML::CXML results;
        m_site.GetCheckResults(results);
        XML::CXML *regexResult = results.FindFirstChild("RegexSearchResult");
        while (regexResult) {
            XML::CXML *regex = regexResult->FindFirstChild("Regex");
            if (regex) {
                std::string s = regex->GetText();
                RegexToItem::const_iterator i = regexToItem.find(s);
                if (i != regexToItem.end()) {
                    XML::CXML *result = regexResult->FindFirstChild("Result");
                    if (result) {
                        m_regexps.SetItem(i->second, 1, LVIF_TEXT, result->GetText(), 0, 0, 0, 0);
                    }
                }
            }
            regexResult = results.FindNextChild("RegexSearchResult");
        }
    }

    UpdateData(FALSE);
}

void CHttpContentChild::UpdateRegexpButtons()
{
    if (::IsWindow(m_hWnd) == FALSE)
        return;

    UpdateData();

    UINT selected = m_regexps.GetSelectedCount();

    GetDlgItem(IDC_EDIT_REGEX)->EnableWindow(selected == 1);
    GetDlgItem(IDC_REMOVE_REGEX)->EnableWindow(selected != 0);
}

void CHttpContentChild::OnRegexpsItemChanged(NMHDR *, LRESULT *pResult)
{
    UpdateRegexpButtons();
    *pResult = 0;
}

void CHttpContentChild::OnRegexpsEndLabelEdit(NMHDR *pNMHDR, LRESULT *pResult)
{
    NMLVDISPINFO *info = reinterpret_cast<NMLVDISPINFO *>(pNMHDR);
    
    if (info->item.pszText == 0)
        return; // Return value ignored
    
    if (info->item.pszText[0] == '\0') {
        *pResult = FALSE;
        return;
    }

    ChangeRegex(m_sourceRegexp, info->item.pszText);

    *pResult = TRUE;
}

void CHttpContentChild::OnRegexpsBeginLabelEdit(NMHDR *pNMHDR, LRESULT *pResult)
{
    NMLVDISPINFO *info = reinterpret_cast<NMLVDISPINFO *>(pNMHDR);
    
    CString text = m_regexps.GetItemText(info->item.iItem, 0);
    m_sourceRegexp = static_cast<const char *>(text);

    *pResult = FALSE;
}

void CHttpContentChild::OnAddRegexp()
{
    ASSERT(m_params);
    CRegexEditor dlg(m_site, *m_params);
    if (dlg.DoModal() == IDOK) {
        const std::string & regex = dlg.GetRegex();
        ASSERT(regex.empty() == false);
        int itemCount = m_regexps.GetItemCount();
        m_regexps.InsertItem(itemCount, regex.c_str());
        m_params->CreateChild("Regex", regex.c_str());
    }
}

void CHttpContentChild::OnRemoveRegexp()
{
    if (m_regexps.GetSelectedCount() == 0)
        return;

    // Gather selected items
    typedef std::list<CString> Strings;
    Strings regexps;
    std::vector<int> items;

    POSITION p = m_regexps.GetFirstSelectedItemPosition();
    while (p) {
        int item = m_regexps.GetNextSelectedItem(p);
        regexps.push_back(m_regexps.GetItemText(item, 0));
        items.push_back(item);
    }

    // Remove all selected items from list
    std::sort(items.begin(), items.end(), std::greater<int>());
    std::size_t count = items.size();
    for (std::size_t i = 0; i < count; i++)
        m_regexps.DeleteItem(items[i]);

    // Remove all selected items from XML
    XML::CXML *regexp = m_params->FindFirstChild("Regex");
    while (regexp && (regexps.empty() == false)) {
        XML::CXML *next = m_params->FindNextChild("Regex");
        CString text = regexp->GetText();
        Strings::iterator iter = std::find(regexps.begin(), regexps.end(), text);
        if (iter != regexps.end()) {
            m_params->RemoveChild(regexp);
            delete regexp;
            regexps.erase(iter);
        }
        regexp = next;
    }
}

void CHttpContentChild::OnEditRegexp()
{
    if (POSITION p = m_regexps.GetFirstSelectedItemPosition()) {
        ASSERT(m_params);
        CRegexEditor dlg(m_site, *m_params);
        int item = m_regexps.GetNextSelectedItem(p);
        std::string oldRegex(m_regexps.GetItemText(item, 0));
        dlg.SetRegex(oldRegex);
        if (dlg.DoModal() == IDOK) {
            const std::string & newRegex = dlg.GetRegex();
            ASSERT(newRegex.empty() == false);
            m_regexps.SetItemText(item, 0, newRegex.c_str());
            ChangeRegex(oldRegex, newRegex);
        }
    }
}
