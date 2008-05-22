/* $Workfile: XmlUtil.cpp $
 *
 * XML Utility functions.
 *
 * $Author: Death $
 * $Revision: 4 $
 * $Date: 12/06/04 18:08 $
 *
 * $NoKeywords: $
 */
#include "XmlUtil.h"

using namespace std;

namespace XML
{

void StringReplace(std::string & str, const char *from, size_t lenFrom, const char *to, size_t lenTo)
{
    string::size_type found = str.find(from);
	while (found != string::npos) {
		str.replace(str.begin() + found, str.begin() + found + lenFrom, to);
		found = str.find(from, found + lenTo);
	}
}

std::string StringTrim(const std::string & str, const std::string & mask)
{
    size_t len = str.length();
    const char *p = str.c_str();

    if (len == 0)
        return str;

    size_t start = 0;
    while (start < len && mask.find(p[start]) != string::npos)
        start++;

    size_t end = len - 1;
    while (end != 0 && mask.find(p[end]) != string::npos)
        end--;

    return str.substr(start, end + 1 - start);
}

std::string PlainToXML(const std::string & source)
{
    std::string result = source;
	StringReplace(result, "&", 1, "&amp;", 5);
	StringReplace(result, "\"", 1, "&quot;",  6);
	StringReplace(result, "<", 1, "&lt;", 4);
	StringReplace(result, ">", 1, "&gt;", 4);
    return result;
}

std::string PlainFromXML(const std::string & source)
{
    std::string result = source;
    StringReplace(result, "&quot;", 6, "\"", 1);
	StringReplace(result, "&lt;", 4, "<", 1);
	StringReplace(result, "&gt;", 4, ">", 1);
	StringReplace(result, "&amp;", 5, "&", 1);
    return result;
}

std::string MultilineToXML(const std::string & source)
{
    std::string result = source;
    StringReplace(result, "\\", 1, "\\\\", 2);
    StringReplace(result, "\r", 1, "\\r", 2);
    StringReplace(result, "\n", 1, "\\n", 2);
    StringReplace(result, "\t", 1, "\\t", 2);
    return result;
}

std::string MultilineFromXML(const std::string & source)
{
    std::string result = source;
    StringReplace(result, "\\\\", 2, "\\", 1);
    StringReplace(result, "\\r", 2, "\r", 1);
    StringReplace(result, "\\n", 2, "\n", 1);
    StringReplace(result, "\\t", 2, "\t", 1);
    return result;
}

//---------------------------------------------------------------------------
// ValidateXML - validate an XML type string
//---------------------------------------------------------------------------
// DESCRIPTION: validates the XML string inserted
// PARAMETERS:	
// Argument         : const char *str	- the xml string to validate
// RETURN:    	int  -	0=not a valid XML
//						n=length of valid XML in chars
// AUTHOR: 	Michael Badichi [25/6/2000]
int Validate(const char * str)
{
    const char *c;
    int intxt=false;
    int lastchar=false;
    int shouldret=false;
    int toclose=false;
    int open=0;
    c = str;
    while(*c) {
        switch(*c) {
            case '"':
                intxt=!intxt;
                break;
            case '<':
                if(!intxt) {
                    open++;
                    shouldret=true;
                }
                break;
            case '>':
                if(!intxt) {
                    if(lastchar=='/' || toclose) {
                        open--;
                    }
                    toclose=false;
                    if(open==0 && shouldret) return(int)(c-str +1);
                }
                break;
            case '/':
                if(!intxt) {
                    if(lastchar=='<')  {
                        toclose=true;
                        open--; //for the last open (which is actually a close)
                    }
                }
                break;
            default:
                break;
        }
        lastchar = *c;
        c++;
    }
    return(0);
}

}