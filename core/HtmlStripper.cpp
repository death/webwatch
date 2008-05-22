// WebWatch Core - by DEATH, 2004
//
// $Workfile: HtmlStripper.cpp $ - HTML stripper filter implementation
//
// $Author: Death $
// $Revision: 3 $
// $Date: 8/20/05 3:12 $
// $NoKeywords: $
//
#include "stdafx.h"
#include "htmlstripper.h"

namespace 
{
    typedef stdext::hash_map<std::string, std::string> Synonyms;

    Synonyms s_htmlSynonyms;

    void DumpUntilTag(std::istream & is, std::ostream & os, bool pre, bool ignoreWhiteSpace);
    void IgnoreUntilTag(std::istream & is);
    std::string GetWord(std::istream & is, const std::string & delimiters);
}

namespace Core_DE050401
{
    bool StripHtml(std::istream & is, std::ostream & os, bool skipScript, bool skipWhiteSpace)
    {
        static bool initialized = false;
        const static std::string delimiters = ">";

        if (initialized == false) {
            s_htmlSynonyms["nbsp"] = " ";
            s_htmlSynonyms["amp"] = "&";
            s_htmlSynonyms["lt"] = "<";
            s_htmlSynonyms["gt"] = ">";
            initialized = true;
        }

        bool pre = false;

        while (is.good()) {
            DumpUntilTag(is, os, pre, skipWhiteSpace);

            if (is.good()) {
                // Get tag
                std::string token = GetWord(is, delimiters);
                std::string tag = boost::to_lower_copy(token);

                if (tag == "/pre")
                    pre = false;

                if (pre == false) {
                    if (tag == "pre") {
                        pre = true;
                    } else if (skipScript) {
                        if (tag.compare(0, 5, "script", 5) == 0) {
                            while (is.good()) {
                                IgnoreUntilTag(is);
                                token = GetWord(is, delimiters);
                                tag = boost::to_lower_copy(token);
                                if (tag == "/script")
                                    break;
                            }
                        }
                    }
                } else {
                    os.put('<');
                    os.write(token.c_str(), static_cast<std::streamsize>(token.length()));
                    os.put('>');
                }
            }
        }

        return(false);
    }
}

namespace 
{
    void IgnoreUntilTag(std::istream & is)
    {
        while (is.good()) {
            int ch = is.get();
            if (is.fail()) return;
            switch (ch) {
                case '<':
                    return;
                default:
                    break;
            }
        }
    }

    void DumpUntilTag(std::istream & is, std::ostream & os, bool pre, bool ignoreWhiteSpace)
    {
        const static std::string delimiters = "; ";

        while (is.good()) {
            int ch = is.get();
            if (is.fail()) return;
            switch (ch) {
                case '<':
                    return;
                case '\r':
                case '\n':
                case ' ':
                case '\t':
                    if (!ignoreWhiteSpace)
                        os.put(static_cast<char>(ch));
                    break;
                case '&':
                    if (pre == false && is.good()) {
                        std::istream::pos_type pos = is.tellg();
                        std::string word = GetWord(is, delimiters);
                        Synonyms::const_iterator i = s_htmlSynonyms.find(word);
                        if (i != s_htmlSynonyms.end()) {
                            const std::string & synonym = i->second;
                            os.write(synonym.c_str(), static_cast<std::streamsize>(synonym.length()));
                            break;
                        } else {
                            is.seekg(pos);
                        }
                    }
                    // Fallthrough
                default:
                    os.put(static_cast<char>(ch));
                    break;
            }
        }
    }

    std::string GetWord(std::istream & is, const std::string & delimiters)
    {
        std::string strWord;

        while (is.good()) {
            char ch = static_cast<char>(is.get());
            if (delimiters.find(ch) == std::string::npos) {
                strWord.append(1, ch);
            } else {
                break;
            }
        }

        return(strWord);
    }
}