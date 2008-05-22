// WebWatch UI - by DEATH, 2004
//
// $Workfile: MU.h $ - Minor Update settings
//
// $Author: Death $
// $Revision: 2 $
// $Date: 4/02/05 3:58 $
// $NoKeywords: $
//
#ifndef WEBWATCH_INC_MU_H
#define WEBWATCH_INC_MU_H

namespace WebWatch
{

class MUSettings
{
    static const int DefaultSpan = 13;
    static const int MinSpan = 1;
    static const int MaxSpan = 365;

public:
    MUSettings()
    : m_span(DefaultSpan)
    {
    }

    MUSettings(const XML::CXML & xml)
    {
        SetSpan(xml.GetParamInt("Span", DefaultSpan));
    }

    static const char *GetXMLName()
    {
        return "MUSettings";
    }

    void SetSpan(int span)
    {
        if (span < MinSpan)
            m_span = MinSpan;
        else if (span > MaxSpan)
            m_span = MaxSpan;
        else
            m_span = span;
    }

    int GetSpan() const
    {
        return m_span;
    }

    static int GetDefaultSpan()
    {
        return DefaultSpan;
    }

    static int GetMinSpan()
    {
        return MinSpan;
    }

    static int GetMaxSpan()
    {
        return MaxSpan;
    }

private:
    int m_span;                         // MU span in days
};

inline XML::CXML & operator<< (XML::CXML & xmlParent, const MUSettings & settings)
{
    int span = settings.GetSpan();
    if (span == MUSettings::GetDefaultSpan())
        return xmlParent;

    XML::CXML *xml = xmlParent.CreateChild(MUSettings::GetXMLName());
    ASSERT(xml);
    xml->SetParam("Span", span);

    return xmlParent;
}

} 

#endif // WEBWATCH_INC_MU_H