/* $Workfile: XmlUtil.h $
 *
 * XML Utility functions header.
 *
 * $Author: Death $
 * $Revision: 3 $
 * $Date: 7/04/04 0:06 $
 *
 * $NoKeywords: $
 */
#pragma once

#include <string>

namespace XML
{

void StringReplace(std::string & str, const char *from, size_t lenFrom, const char *to, size_t lenTo);
std::string StringTrim(const std::string & str, const std::string & mask = " \t");
int Validate(const char *str);

// Plain*XML is called internally, since we know the text is plain -
// Multiline*XML, however, is not.

std::string PlainToXML(const std::string & source);
std::string PlainFromXML(const std::string & source);
std::string MultilineToXML(const std::string & source);
std::string MultilineFromXML(const std::string & source);

}