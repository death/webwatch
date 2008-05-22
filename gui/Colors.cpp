// WebWatch UI - by DEATH, 2004
//
// $Workfile: Colors.cpp $ - WebWatch colors
//
// $Author: Death $
// $Revision: 2 $
// $Date: 4/02/05 3:58 $
// $NoKeywords: $
//
#include "stdafx.h"

#include "Colors.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif // _DEBUG

namespace WebWatch
{
    // Color class

    Color::Color(const XML::CXML & xml)
    {
        Read(xml);
    }

    const XML::CXML & Color::Read(const XML::CXML & xml)
    {
        ASSERT(strcmp(xml.GetTagName(), GetXMLName()) == 0);

        unsigned char r = static_cast<unsigned char>(xml.GetParamInt("Red"));
        unsigned char g = static_cast<unsigned char>(xml.GetParamInt("Green"));
        unsigned char b = static_cast<unsigned char>(xml.GetParamInt("Blue"));
        color = (static_cast<COLORREF>(b) << 16) + 
                (static_cast<COLORREF>(g) << 8 ) + 
                 static_cast<COLORREF>(r);

        return xml;
    }

    XML::CXML & Color::Write(XML::CXML & xmlParent, const char *name) const
    {
        if (XML::CXML *xml = xmlParent.CreateChild(GetXMLName())) {
            COLORREF c = color;
            xml->SetParam("Name", name);
            xml->SetParam("Red", c & 0xFF);
            xml->SetParam("Green", (c >> 8) & 0xFF);
            xml->SetParam("Blue", (c >> 16) & 0xFF);
        }    
        return xmlParent;
    }

    const char *Color::GetXMLName()
    {
        return "Color";
    }

    // ColorPair class

    ColorPair::ColorPair(const XML::CXML & xml, COLORREF defaultText, COLORREF defaultBackground)
    : text(defaultText)
    , background(defaultBackground)
    {
        ASSERT(strcmp(xml.GetTagName(), GetXMLName()) == 0);

        const XML::CXML *xmlColor = xml.FindFirstChild(Color::GetXMLName());
        while (xmlColor) {
            const char *name = xmlColor->GetParamText("Name");
            if (strcmp(name, "Text") == 0)
                text.Read(*xmlColor);
            else if (strcmp(name, "Background") == 0)
                background.Read(*xmlColor);
            xmlColor = xml.FindNextChild(Color::GetXMLName());
        }
    }

    XML::CXML & ColorPair::Write(XML::CXML & xmlParent, const char *name) const
    {
        if (XML::CXML *xml = xmlParent.CreateChild(GetXMLName())) {
            xml->SetParam("Name", name);
            text.Write(*xml, "Text");
            background.Write(*xml, "Background");
        }
        return xmlParent;
    }

    const char *ColorPair::GetXMLName()
    {
        return "ColorPair";
    }
} 
