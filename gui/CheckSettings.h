// WebWatch UI - by DEATH, 2004
//
// $Workfile: CheckSettings.h $ - Site check settings
//
// $Author: Death $
// $Revision: 2 $
// $Date: 4/02/05 3:58 $
// $NoKeywords: $
//
#ifndef WEBWATCH_INC_CHECKSETTINGS_H
#define WEBWATCH_INC_CHECKSETTINGS_H

namespace WebWatch
{

class CheckSettings
{
public:
    CheckSettings();
    CheckSettings(const XML::CXML & xml);

    static const char *GetXMLName();

    void SetThreadCount(int threadCount);
    int GetThreadCount() const;
    static int GetDefaultThreadCount();
    static int GetMinThreadCount();
    static int GetMaxThreadCount();

private:
    int m_threadCount;
};

XML::CXML & operator<< (XML::CXML & xmlParent, const CheckSettings & settings);

} 

#endif // WEBWATCH_INC_CHECKSETTINGS_H