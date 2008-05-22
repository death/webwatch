// WebWatch UI - by DEATH, 2004
//
// $Workfile: SiteGroupsTreeColors.cpp $ - Site groups tree control colors
//
// $Author: Death $
// $Revision: 1 $
// $Date: 8/08/05 0:27 $
// $NoKeywords: $
//
#include "stdafx.h"

#include "SiteGroupsTree.h"

using WebWatch::Color;

#define LENGTHOF(x) (sizeof(x) / sizeof(x[0]))

namespace {
    struct DefaultColors
    {
        enum Colors {
            regular = 0x000000, // Black
            updated = 0xFF0000, // Blue
            mu = 0x007F40, // Green
            droppable = 0x226A24, // Dark green
            nondroppable = 0x0000FF, // Red
            dragMask = 0xFF00FF // Purple
        };
    };

    const struct NamedColor {
        const char *name;
        Color CSiteGroupsTree::Colors::*obj;
    } colors[] = {
        {"Regular", &CSiteGroupsTree::Colors::regular},
        {"Updated", &CSiteGroupsTree::Colors::updated},
        {"MU", &CSiteGroupsTree::Colors::mu},
        {"Droppable", &CSiteGroupsTree::Colors::droppable},
        {"Nondroppable", &CSiteGroupsTree::Colors::nondroppable},
        {"DragMask", &CSiteGroupsTree::Colors::dragMask}
    };

    struct NameIs
    {
        const char *m_toFind;
        explicit NameIs(const char *toFind) : m_toFind(toFind) {}
        bool operator() (const NamedColor & color) const 
        { return strcmp(color.name, m_toFind) == 0; }
    };

    struct WriteColor
    {
        const CSiteGroupsTree::Colors & m_owner;
        XML::CXML & m_xml;
        
        explicit WriteColor(const CSiteGroupsTree::Colors & owner,
                            XML::CXML & xml)
        : m_owner(owner)
        , m_xml(xml) 
        {
        }

        void operator() (const NamedColor & color)
        {
            (m_owner.*color.obj).Write(m_xml, color.name);
        }
    };
}

CSiteGroupsTree::Colors::Colors()
{
    Init();
}

CSiteGroupsTree::Colors::Colors(const XML::CXML & xml)
{
    Init();

    ASSERT(strcmp(xml.GetTagName(), GetXMLName()) == 0);

    const NamedColor *first = colors;
    const NamedColor *last = colors + LENGTHOF(colors);
    
    for (const XML::CXML *xmlColor = xml.FindFirstChild(Color::GetXMLName());
         xmlColor; xmlColor = xml.FindNextChild(Color::GetXMLName())) {

        const char *name = xmlColor->GetParamText("Name");
        const NamedColor *color = std::find_if(first, last, NameIs(name));
        if (color != last)
            (this->*color->obj).Read(*xmlColor);
    }
}

XML::CXML *CSiteGroupsTree::Colors::Write(XML::CXML & xmlParent) const
{
    XML::CXML *xml = xmlParent.CreateChild(GetXMLName());
    if (xml == 0)
        return 0;

    std::for_each(colors, colors + LENGTHOF(colors), WriteColor(*this, *xml));

    return xml;
}

void CSiteGroupsTree::Colors::Init()
{
    regular = DefaultColors::regular;
    updated = DefaultColors::updated;
    mu = DefaultColors::mu;
    droppable = DefaultColors::droppable;
    nondroppable = DefaultColors::nondroppable;
    dragMask = DefaultColors::dragMask;
}

const char *CSiteGroupsTree::Colors::GetXMLName()
{
    return "SiteGroupsTreeColors";
}
