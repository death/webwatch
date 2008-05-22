// WebWatch UI - by DEATH, 2004
//
// $Workfile: Colors.h $ - WebWatch colors
//
// $Author: Death $
// $Revision: 3 $
// $Date: 8/08/05 0:26 $
// $NoKeywords: $
//
#ifndef WEBWATCH_INC_COLORS_H
#define WEBWATCH_INC_COLORS_H

namespace WebWatch
{

struct Color
{
    Color()
    {
    }

    explicit Color(COLORREF color_)
    : color(color_)
    {
    }

    explicit Color(const XML::CXML & xml);

    Color & operator= (const COLORREF color_)
    {
        color = color_;
        return *this;
    }

    XML::CXML & Write(XML::CXML & xmlParent, const char *name) const;
    const XML::CXML & Read(const XML::CXML & xml);
    
    static const char *GetXMLName();

    COLORREF color;
};
    
struct ColorPair
{
    ColorPair()
    {
    }

    ColorPair(COLORREF text_, COLORREF background_)
    : text(text_)
    , background(background_)
    {
    }

    ColorPair(const XML::CXML & xml, COLORREF defaultText, COLORREF defaultBackground);
    XML::CXML & Write(XML::CXML & xmlParent, const char *name) const;

    static const char *GetXMLName();

    Color text;
    Color background;
};

}

#endif // WEBWATCH_INC_COLORS_H