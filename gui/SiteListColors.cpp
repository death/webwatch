// WebWatch UI - by DEATH, 2004
//
// $Workfile: SiteListColors.cpp $ - Site list control's colors
//
// $Author: Death $
// $Revision: 1 $
// $Date: 8/06/05 19:32 $
// $NoKeywords: $
//
#include "stdafx.h"

#include "SiteList.h"

using WebWatch::ColorPair;

namespace {
    struct DefaultColors
    {
        enum Colors {
            regular = 0x000000, // Black
            updated = 0xFF0000, // Blue
            erroneous = 0x0000FF, // Red
            mu = 0x007F40, // Green
            background = 0xE4F7F8 // Weird yellow?
        };
    };
}

CSiteList::Colors::Colors()
{
    AddDefault();
}

CSiteList::Colors::Colors(const ColorPair & regular, 
                          const ColorPair & updated, 
                          const ColorPair & erroneous, 
                          const ColorPair & mu)
{
    Add("Regular", regular);
    Add("Updated", updated);
    Add("Erroneous", erroneous);
    Add("MU", mu);
}

CSiteList::Colors::Colors(const XML::CXML & xml)
{
    ASSERT(strcmp(xml.GetTagName(), GetXMLName()) == 0);

    const XML::CXML *xmlPair = xml.FindFirstChild(ColorPair::GetXMLName());
    while (xmlPair) {
        const char *name = xmlPair->GetParamText("Name");
        // NOTE: Yes, I know it always uses regular text color as default
        m_cpairs[name] = ColorPair(*xmlPair, DefaultColors::regular, DefaultColors::background);
        xmlPair = xml.FindNextChild(ColorPair::GetXMLName());
    }

    AddDefault();
}

namespace
{
    struct WriteColorPair
    {
        explicit WriteColorPair(XML::CXML & xml) : m_xml(xml) {}

        template<typename Pair>
        void operator() (const Pair & p) const
        {
            const std::string & name = p.first;
            const ColorPair & cpair = p.second;
            cpair.Write(m_xml, name.c_str());
        }

        XML::CXML & m_xml;
    };
}

XML::CXML *CSiteList::Colors::Write(XML::CXML & parent) const
{
    XML::CXML *xml = parent.CreateChild(GetXMLName());
    
    if (xml == 0)
        return 0;

    std::for_each(m_cpairs.begin(), m_cpairs.end(), WriteColorPair(*xml));
    return xml;
}

const ColorPair & CSiteList::Colors::GetColorPair(const char *name) const
{
    ColorPairMap::const_iterator it = m_cpairs.find(name);
    if (it == m_cpairs.end())
        throw std::runtime_error("Non-existent color requested");
    return it->second;
}

void CSiteList::Colors::SetColorPair(const char *name, const ColorPair & cpair)
{
    ColorPairMap::iterator it = m_cpairs.find(name);
    if (it == m_cpairs.end()) {
        m_cpairs.insert(std::make_pair(name, ColorPair(cpair)));
        return;
    }
    it->second = cpair;
}

const char *CSiteList::Colors::GetXMLName()
{
    return "SiteListColors";
}

CSiteList::Colors::ColorPairNames CSiteList::Colors::GetColorPairNames()
{
    ColorPairNames names;
    names.reserve(4);
    names.push_back("Regular");
    names.push_back("Updated");
    names.push_back("Erroneous");
    names.push_back("MU");
    return names;
}

void CSiteList::Colors::Add(const std::string & name, const ColorPair & cpair)
{
    m_cpairs.insert(std::make_pair(name, cpair));
}

void CSiteList::Colors::AddDefault()
{
    Add("Regular", ColorPair(DefaultColors::regular, DefaultColors::background));
    Add("Updated", ColorPair(DefaultColors::updated, DefaultColors::background));
    Add("Erroneous", ColorPair(DefaultColors::erroneous, DefaultColors::background));
    Add("MU", ColorPair(DefaultColors::mu, DefaultColors::background));
}
