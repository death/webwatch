/* $Workfile: Xml.h $
 *
 * XML class header.
 *
 * $Author: Death $
 * $Revision: 13 $
 * $Date: 3/08/04 23:48 $
 *
 * $NoKeywords: $
 */
#pragma once

#include <list>
#include <string>
#include <hash_map>
#include <sstream>

namespace XML
{

class CXML
{
public:
    explicit CXML(const char *tag = 0);
    CXML(const CXML & other);
    ~CXML();

    CXML & operator= (const CXML & rhs);
    friend bool operator== (const CXML & xml1, const CXML & xml2);

    bool IsEmpty() const;
    void ResetContents();

    const char *GetTagName() const;
    void SetTagName(const char *tag);

    const char *GetText() const;
    void SetText(const char *text);

    const char *GetChildText(const char *tag) const;
    std::size_t GetChildCount() const;
    void AddChild(CXML *child);
    void RemoveChild(CXML *child);
    CXML *GetCurrentChild();
    const CXML *GetCurrentChild() const;
    CXML *CreateChild(const char *tag, const char *text = 0);
    CXML *CreateChild(const CXML & xml);

    CXML *GetFirstChild();
    const CXML *GetFirstChild() const;
    CXML *GetNextChild();
    const CXML *GetNextChild() const;
    CXML *GetPrevChild();
    const CXML *GetPrevChild() const;
    CXML *GetLastChild();
    const CXML *GetLastChild() const;

    const CXML *FindFirstChild(const char *tag) const;
    CXML *FindFirstChild(const char *tag, bool alwaysFind = false);
    const CXML *FindNextChild(const char *tag) const;
    CXML *FindNextChild(const char *tag);

    bool Load(const char *filename, const char *tag = 0);
    bool Save(const char *filename = 0);
    bool SaveAs(const char *filename);

    void XMLWrite(FILE *file, int indent = 0);
    bool XMLRead(FILE *file, const char *tag = 0);
    void XMLToStr(std::string & str);
    bool XMLFromStr(const std::string & str, const char *tag = 0, int pos = 0);
    
    std::size_t GetParamCount() const;
    const char *GetParamText(const char *name, const char *def = "") const;

#define DECLARE_XML_PARAM_GET(Type, Name) \
    Type Name(const char *name, Type def = 0) const;

    DECLARE_XML_PARAM_GET(int, GetParamInt);
    DECLARE_XML_PARAM_GET(long, GetParamLong);
    DECLARE_XML_PARAM_GET(unsigned long, GetParamDWORD);
    DECLARE_XML_PARAM_GET(float, GetParamFloat);
    DECLARE_XML_PARAM_GET(double, GetParamDouble);
    DECLARE_XML_PARAM_GET(bool, GetParamBool)

#undef DECLARE_XML_PARAM_GET

    void SetParam(const char *name, const char *value);
    void SetParam(const char *name, bool value);

    template<class T> void SetParam(const char *name, T value)
    {
		std::ostringstream os;
		os << value;
		SetParam(name, os.str().c_str());
    }

    void RemoveParam(const char *name);

private:
    struct Impl;
    std::auto_ptr<Impl> m_pimpl;
};

bool operator== (const CXML & xml1, const CXML & xml2);
bool operator!= (const CXML & xml1, const CXML & xml2);

inline CXML & operator<< (CXML & output, const CXML & input)
{
    output.CreateChild(input);
    return output;
}

}