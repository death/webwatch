// WebWatch Core - by DEATH, 2004
//
// $Workfile: HtmlStripper.h $ - HTML stripper filter
//
// $Author: Death $
// $Revision: 2 $
// $Date: 4/02/05 3:58 $
// $NoKeywords: $
//
#ifndef WEBWATCH_INC_HTMLSTRIPPER_H
#define WEBWATCH_INC_HTMLSTRIPPER_H

namespace Core_DE050401
{
    bool StripHtml(std::istream & is, std::ostream & os, bool skipScript, bool skipWhiteSpace);
}

#endif // WEBWATCH_INC_HTMLSTRIPPER_H