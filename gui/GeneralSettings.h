// WebWatch UI - by DEATH, 2004
//
// $Workfile: GeneralSettings.h $ - General settings
//
// $Author: Death $
// $Revision: 2 $
// $Date: 4/02/05 3:58 $
// $NoKeywords: $
//
#ifndef WEBWATCH_INC_GENERALSETTINGS_H
#define WEBWATCH_INC_GENERALSETTINGS_H

namespace WebWatch
{

struct GeneralSettings
{
    GeneralSettings()
    : autoSave(DefaultAutoSave)
    {
    }

    GeneralSettings(const XML::CXML & xml)
    : autoSave(DefaultAutoSave)
    {
        const XML::CXML *xmlAutoSave = xml.FindFirstChild("AutoSave");
        if (xmlAutoSave)
            autoSave = xmlAutoSave->GetParamBool("Enabled", DefaultAutoSave);
    }

    static const char *GetXMLName()
    {
        return "GeneralSettings";
    }
    
    bool autoSave;

    static const bool DefaultAutoSave = true;
};

inline XML::CXML & operator<< (XML::CXML & xmlParent, const GeneralSettings & settings)
{
    if (settings.autoSave == GeneralSettings::DefaultAutoSave)
        return xmlParent;

    XML::CXML *xml = xmlParent.CreateChild(GeneralSettings::GetXMLName());
    ASSERT(xml);
    
    XML::CXML *xmlAutoSave = xml->CreateChild("AutoSave");
    xmlAutoSave->SetParam("Enabled", settings.autoSave);

    return xmlParent;
}

} 

#endif // WEBWATCH_INC_CHECKSETTINGS_H