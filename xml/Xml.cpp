/* $Workfile: Xml.cpp $
 *
 * XML class implementation.
 *
 * $Author: Death $
 * $Revision: 23 $
 * $Date: 1/07/05 21:28 $
 *
 * $NoKeywords: $
 */
#include "Xml.h"
#include "XmlUtil.h"

#include <cassert>
#include <algorithm>
#include <functional>
#include <hash_map>

using namespace std;

#pragma warning(disable:4355) // 'this' : used in base member initializer list

namespace {
    const char * const DefaultTag = "XML";

    bool IsStartBlock(const std::string & line, const std::string & TagName);

    const size_t BufferSize = 16384;

    // CXMLBuffer structure
    struct CXMLBuffer
    {
	    CXMLBuffer(std::FILE *_pFile, size_t BufferSize)
        : pBuffer(new char [BufferSize])
	    , nBytesLeft(0)
        , nBytesUsed(0)
	    , nBytesRead(BufferSize) // For initialization
	    , file(_pFile)
	    {
	    }

	    ~CXMLBuffer()
	    {
		    delete [] pBuffer;
	    }

	    char *pBuffer;
	    size_t nBytesLeft;
	    size_t nBytesRead;
	    size_t nBytesUsed;
	    std::FILE *file;
    };
}

#define GET_BYTE_FROM_BUFFER(buffer) \
	/* Check that there are still bytes left */ \
	if (buffer.nBytesLeft-- == 0) { \
		/* No more bytes left, if number of bytes read was less than the \
		   maximum capacity, we're on the end of file */ \
		if (buffer.nBytesRead != BufferSize) { \
            buffer.nBytesLeft = 0; \
			eof = true; \
			return; \
		} \
		/* Read new chunk */ \
		buffer.nBytesRead = buffer.nBytesLeft = fread(buffer.pBuffer, 1, BufferSize, buffer.file); \
		/* Check that there are still bytes left */ \
		if (buffer.nBytesLeft-- == 0) { \
            buffer.nBytesLeft = 0; \
			eof = true; \
			return; \
		} \
		buffer.nBytesUsed = 0; \
	} \
	ch = buffer.pBuffer[buffer.nBytesUsed++];

namespace XML {

struct CXML::Impl
{
    typedef list<CXML *> ChildNodes;
    typedef stdext::hash_map<string, string> ParameterMap;

    Impl(CXML *self_, const char *tag_)
    : self(self_)
    , tag(tag_)
    , eof(false)
    {
        currentChild = children.end();
        currentChildConst = children.end();
    }

    Impl(CXML *self_, const Impl & other)
    : self(self_)
    , tag(other.tag)
    , filename(other.filename)
    , text(other.text)
    , eof(other.eof)
    , params(other.params)
    {
        if (other.children.empty() == false) {
            ChildNodes::const_iterator i = other.children.begin();
            ChildNodes::const_iterator end = other.children.end();
            while (i != end) {
                const CXML *child = *i++;
                CXML *myChild = new CXML(*child);
                children.push_back(myChild);
            }
        }

        currentChild = children.end();
        currentChildConst = children.end();
    }

    void GetNextLine(CXMLBuffer & buffer, string & line);
    void GetNextLine(const string & str, int *pos, string & line);

    void ParseBlock(string line, CXMLBuffer & buffer);
    void ParseBlock(string line, const string & str, int *pos);

    CXML *self;

    string tag;
    string filename;
    string text;
    bool eof;
    ChildNodes children;
    ParameterMap params;

    mutable ChildNodes::iterator currentChild;
    mutable ChildNodes::const_iterator currentChildConst;
};

CXML::CXML(const char *tag)
: m_pimpl(new Impl(this, tag ? tag : DefaultTag))
{
}

CXML::CXML(const CXML & other)
: m_pimpl(new Impl(this, *other.m_pimpl))
{
}

CXML::~CXML()
{
    ResetContents();
}

CXML & CXML::operator= (const CXML & rhs)
{
    if (this != &rhs) {
        CXML temp(rhs);
        swap(m_pimpl->self, temp.m_pimpl->self);
        swap(m_pimpl, temp.m_pimpl);
    }
    return *this;
}

bool CXML::IsEmpty() const
{
    return m_pimpl->text.empty() && m_pimpl->params.empty() && m_pimpl->children.empty();
}

void CXML::ResetContents()
{
    Impl::ChildNodes::iterator i = m_pimpl->children.begin();
    Impl::ChildNodes::iterator end = m_pimpl->children.end();
    while (i != end) {
        CXML *child = *i++;
        delete child;
    }

    m_pimpl->children.clear();

    m_pimpl->text.clear();
    m_pimpl->params.clear();
}

const char *CXML::GetTagName() const
{
    return m_pimpl->tag.c_str();
}

void CXML::SetTagName(const char *tag)
{
    m_pimpl->tag = tag;
}

const char *CXML::GetText() const
{
    return m_pimpl->text.c_str();
}

void CXML::SetText(const char *text)
{
    m_pimpl->text = text;
}

const char *CXML::GetChildText(const char *tag) const
{
	const CXML *child = FindFirstChild(tag);
	if (child)
		return child->GetText();
	return "";
}

size_t CXML::GetChildCount() const
{
    return m_pimpl->children.size();
}

void CXML::AddChild(CXML *child)
{
    m_pimpl->children.push_back(child);
}

void CXML::RemoveChild(CXML *child)
{
    // Assumes only zero or one pointer of child exists in children list
    if (GetCurrentChild() == child)
        GetNextChild();
    m_pimpl->children.remove(child);
}

CXML *CXML::GetCurrentChild()
{
    if (m_pimpl->currentChild == m_pimpl->children.end())
        return 0;
    return *m_pimpl->currentChild;
}

const CXML *CXML::GetCurrentChild() const
{
    if (m_pimpl->currentChildConst == m_pimpl->children.end())
        return 0;
    return *m_pimpl->currentChildConst;
}

CXML *CXML::CreateChild(const char *tag, const char *text)
{
    CXML *child = new CXML(tag);
    if (text)
        child->SetText(text);
    AddChild(child);
    return child;
}

CXML *CXML::CreateChild(const CXML & xml)
{
    CXML *child = new CXML(xml);
    AddChild(child);
    return child;
}

CXML *CXML::GetFirstChild()
{
    if (m_pimpl->children.empty()) {
        m_pimpl->currentChild = m_pimpl->children.end();
        m_pimpl->currentChildConst = m_pimpl->children.end();
        return 0;
    }

    m_pimpl->currentChild = m_pimpl->children.begin();
    m_pimpl->currentChildConst = m_pimpl->children.begin();
    return *m_pimpl->currentChild;
}

const CXML *CXML::GetFirstChild() const
{
    if (m_pimpl->children.empty()) {
        m_pimpl->currentChild = m_pimpl->children.end();
        m_pimpl->currentChildConst = m_pimpl->children.end();
        return 0;
    }

    m_pimpl->currentChild = m_pimpl->children.begin();
    m_pimpl->currentChildConst = m_pimpl->children.begin();
    return *m_pimpl->currentChildConst;
}

CXML *CXML::GetNextChild()
{
	if (m_pimpl->currentChild == m_pimpl->children.end())
        return 0;
    ++m_pimpl->currentChildConst;
    if (++m_pimpl->currentChild == m_pimpl->children.end())
        return 0;
    return *m_pimpl->currentChild;
}

const CXML *CXML::GetNextChild() const
{
	if (m_pimpl->currentChild == m_pimpl->children.end())
        return 0;
    ++m_pimpl->currentChildConst;
    if (++m_pimpl->currentChild == m_pimpl->children.end())
        return 0;
    return *m_pimpl->currentChildConst;
}

CXML *CXML::GetPrevChild()
{
    if (m_pimpl->children.empty())
        return 0;
    if (m_pimpl->currentChild == m_pimpl->children.begin())
        return 0;
    --m_pimpl->currentChildConst;
    return *--m_pimpl->currentChild;
}

const CXML *CXML::GetPrevChild() const
{
    if (m_pimpl->children.empty())
        return 0;
    if (m_pimpl->currentChild == m_pimpl->children.begin())
        return 0;
    --m_pimpl->currentChildConst;
    return *--m_pimpl->currentChildConst;
}

CXML *CXML::GetLastChild()
{
    m_pimpl->currentChild = m_pimpl->children.end();
    m_pimpl->currentChildConst = m_pimpl->children.end();
    return GetPrevChild();
}

const CXML *CXML::GetLastChild() const
{
    m_pimpl->currentChild = m_pimpl->children.end();
    m_pimpl->currentChildConst = m_pimpl->children.end();
    return GetPrevChild();
}

CXML *CXML::FindFirstChild(const char *name, bool alwaysFind)
{
    CXML *child = GetFirstChild();
    while (child && (strcmp(child->GetTagName(), name) != 0))
        child = GetNextChild();

    if (alwaysFind && child == 0)
        return CreateChild(name);

    return child;
}

const CXML *CXML::FindFirstChild(const char *tag) const
{
    const CXML *child = GetFirstChild();
    while (child && (strcmp(child->GetTagName(), tag) != 0))
        child = GetNextChild();
    return child;
}

CXML *CXML::FindNextChild(const char *tag)
{
    CXML *child = GetNextChild();
    while (child && (strcmp(child->GetTagName(), tag) != 0))
        child = GetNextChild();
    return child;
}

const CXML *CXML::FindNextChild(const char *tag) const
{
    const CXML *child = GetNextChild();
    while (child && (strcmp(child->GetTagName(), tag) != 0))
        child = GetNextChild();
    return child;
}

bool CXML::Load(const char *filename, const char *tag)
{
    FILE *input = fopen(filename, "rt");
    if (input == 0)
        return false;
    bool result = XMLRead(input, tag);
    fclose(input);
    m_pimpl->filename = filename;
    return result;
}

bool CXML::Save(const char *filename)
{
    if (filename == 0) {
        if (m_pimpl->filename.empty())
            return false;
        filename = m_pimpl->filename.c_str();
    }

    FILE *output = fopen(filename, "wt");
    if (output == 0)
        return false;
    XMLWrite(output);
    fclose(output);
    return true;
}

bool CXML::SaveAs(const char *filename)
{
    m_pimpl->filename = filename;
    return Save();
}

size_t CXML::GetParamCount() const
{
    return m_pimpl->params.size();
}

const char *CXML::GetParamText(const char *name,const char *def) const
{
    Impl::ParameterMap::const_iterator pos = m_pimpl->params.find(name);
	if (pos == m_pimpl->params.end())
		return def;
	return pos->second.c_str();
}

#define DEFINE_XML_PARAM_GET(Type, Name)                            \
Type CXML::Name(const char * name, Type def) const                  \
{                                                                   \
    Impl::ParameterMap::const_iterator pos = m_pimpl->params.find(name); \
	if (pos == m_pimpl->params.end())                               \
		return def;                                                 \
	const std::string & text = pos->second;                         \
	if (text.empty() == true)                                       \
		return def;                                                 \
	std::istringstream ss(text);                                    \
	Type t;                                                         \
	ss >> t;                                                        \
    if (ss.fail())                                                  \
        return def;                                                 \
	return t;                                                       \
}

DEFINE_XML_PARAM_GET(int, GetParamInt);
DEFINE_XML_PARAM_GET(long, GetParamLong);
DEFINE_XML_PARAM_GET(unsigned long, GetParamDWORD);
DEFINE_XML_PARAM_GET(float, GetParamFloat);
DEFINE_XML_PARAM_GET(double, GetParamDouble);

#undef DEFINE_XML_PARAM_GET

void CXML::SetParam(const char *name, const char *text)
{
    m_pimpl->params[name] = text;
}

void CXML::SetParam(const char *name, bool value)
{
    SetParam(name, value ? "True" : "False");
}

void CXML::RemoveParam(const char *name)
{
    Impl::ParameterMap::iterator pos = m_pimpl->params.find(name);
	if (pos == m_pimpl->params.end())
		return;
	m_pimpl->params.erase(pos);
}

bool CXML::GetParamBool(const char *name, bool def) const
{
    Impl::ParameterMap::const_iterator pos = m_pimpl->params.find(name);
    if (pos == m_pimpl->params.end())
        return def;
    const std::string & text = pos->second;
    return text == "True";
}

bool CXML::XMLFromStr(const std::string & str, const char *tag, int pos)
{
    m_pimpl->tag = tag;
    m_pimpl->eof = false;
    string nextLine;
    m_pimpl->GetNextLine(str, &pos, nextLine);
    while (!m_pimpl->eof && !IsStartBlock(nextLine, m_pimpl->tag))
        m_pimpl->GetNextLine(str, &pos, nextLine);
    m_pimpl->text.clear();
    if (m_pimpl->eof)
        return false;
	m_pimpl->ParseBlock(nextLine, str, &pos);
	return true;
}

// GetNextLine supporting a memory buffer and reading into it when needed
void CXML::Impl::GetNextLine(CXMLBuffer & buffer, std::string & line)
{
    char ch;
    bool isCommentTag;
    bool isPrivateTag;
    std::string data;

    do {
        isCommentTag = false;
        isPrivateTag = false;

        // Get to a tag

        GET_BYTE_FROM_BUFFER(buffer);
        
        data.clear();
        bool inData = false;
        while (ch != '<') {
            if (inData == false && ch >= ' ')
                inData = true;
            
            if (inData)
                data += ch;

            GET_BYTE_FROM_BUFFER(buffer);
        }

        text = PlainFromXML(data);

        // Read tag

        bool inQuote = false;
        bool finish = false;
        std::size_t len = 1;
        line = '<';

        while (finish == false) {
            GET_BYTE_FROM_BUFFER(buffer);
            
            switch (ch) {
                case '"':
                    inQuote = !inQuote;
                    break;
                case '-':
                    if (len == 3) {
                        if (line == "<!-")
                            isCommentTag = true;
                    }
                    break;
                case '?':
                    if (len == 1) {
                        isPrivateTag = true;
                    }
                    break;
                case '>':
                    if (inQuote == false) {
                        if (isCommentTag) {
                            // <!----> is shortest comment possible
                            if (len > 5) {
                                if (line[len - 2] == '-' && line[len - 1] == '-') {
                                    finish = true;
                                }
                            }
                        } else {
                            finish = true;
                        }
                    }
                    break;
                default:
                    break;
            }
            
            line += ch;
            len++;
        }
    } while (isCommentTag || isPrivateTag);
}

void CXML::Impl::GetNextLine(const string & str, int *pos, string & line)
{
    // THIS FUNCTION IS NO LONGER SUPPORTED

    struct Hack
    {
        // NOTE: Expects ms to contain at least one character, this character
        // is expected to be '<'.
        static bool IsPrivateTagOrComment(const std::string & ms)
        {
	        const char *str = ms.c_str();
	        if (str[1] == '?')
		        return true;
	        if (ms.length() > 4)
		        if (strncmp(&str[1], "!--", 3) == 0)
			        return true;
	        return false;
        }
    };

	string ms;
	char ch;
	do {
		line.clear();
		if (static_cast<size_t>(*pos) >= str.length()) {
			eof = true;
			break;
		}

		ch = str[*pos];
		(*pos)++;

		bool inData = false;
		ms.clear();
		while (ch != '<') {
			if (inData || ch >= ' ') {
				ms += static_cast<char>(ch);
				inData = true;
			}
			if (static_cast<size_t>(*pos) >= str.length()) {
				eof = true;
				break;
			}
			ch = str[*pos];
			(*pos)++;
		}
		if (eof)
			break;
        text = PlainFromXML(ms);
		bool inQuote = false;
		while (ch != '>' || inQuote) {
			if (ch >= ' ') {
				if (ch == '"')
					inQuote = !inQuote;
				line += static_cast<char>(ch);
			}
			if (static_cast<size_t>(*pos) >= str.length()) {
				eof = true;
				break;
			}
			ch = str[*pos];
			(*pos)++;
		}
		if (eof)
			break;
		line += '>';
    } while (Hack::IsPrivateTagOrComment(line));
}

void CXML::Impl::ParseBlock(string line, CXMLBuffer & buffer)
{
    self->ResetContents();

	if (line[0] != '<') return;

	size_t linePos = line.find(' ');
	if (linePos == string::npos) {
		linePos = line.length() - 1;
	}

	tag = line.substr(1, linePos - 1);

	size_t EndPos = line.length();
	if (line[EndPos - 1] != '>') {
//		MessageBox(0, "Warning: Tag block is incomplete", "Error", MB_OK | MB_ICONERROR);
	}

	line[EndPos - 1] = ' ';

	size_t TagNameLen = tag.length();
	bool Children = true;
	if (line[EndPos - 2] == '/') {
		if (tag[TagNameLen - 1] == '/') {
			tag.resize(TagNameLen - 1);
		}
		Children = false;
		line[EndPos - 2] = ' ';
	}

	line = line.substr(TagNameLen + 1);
	line = StringTrim(line);

	while (line.length() > 1) {
		linePos = line.find('=');
		if (linePos == string::npos) break;
		string pname = StringTrim(line.substr(0, linePos));
		linePos = line.find('"');
		line = line.substr(linePos + 1);
		linePos = line.find('"');
		string pval = PlainFromXML(line.substr(0, linePos));
		self->SetParam(pname.c_str(), pval.c_str());
		line = StringTrim(line.substr(linePos + 1));
	}

	if (!Children) return;

	line.clear();
	GetNextLine(buffer, line);
	string LastLine = "</";
	LastLine += tag;
	LastLine += '>';
	CXML *child;
	while ((line != LastLine) && !eof) {
		child = new CXML;
		child->m_pimpl->ParseBlock(line, buffer);
		if (child->m_pimpl->tag.empty() == false) self->AddChild(child);
		GetNextLine(buffer, line);
	}
}

void CXML::Impl::ParseBlock(string line, const string & str, int *pos)
{
    self->ResetContents();

	if (line[0] != '<') return;

	size_t linePos = line.find(' ');
	if (linePos == string::npos) {
		linePos = line.length() - 1;
	}

	tag = line.substr(1, linePos - 1);

	size_t EndPos = line.length();
	if (line[EndPos - 1] != '>') {
//		MessageBox(0, "Warning: Tag block is incomplete", "Error", MB_OK | MB_ICONERROR);
	}

	line[EndPos - 1] = ' ';

	bool Children = true;
	if (line[EndPos - 2] == '/') {
		if (tag[tag.length() - 1] == '/') {
			tag.resize(tag.length() - 1);
		}
		Children = false;
		line[EndPos - 2] = ' ';
	}

	line = line.substr(tag.length() + 1);
	line = StringTrim(line);

	while (line.length() > 1) {
		linePos = line.find('=');
		if (linePos == string::npos) break;
		string pname = StringTrim(line.substr(0, linePos));
		linePos = line.find('"');
		line = line.substr(linePos + 1);
		linePos = line.find('"');
		string pval = PlainFromXML(line.substr(0, linePos));
		self->SetParam(pname.c_str(), pval.c_str());
		line = StringTrim(line.substr(linePos + 1));
	}

	if (!Children) return;

	line.clear();
	GetNextLine(str, pos, line);
	string LastLine = "</";
	LastLine += tag;
	LastLine += '>';
	CXML *child;
	while (line != LastLine && !eof) {
		child = new CXML;
		child->m_pimpl->ParseBlock(line, str, pos);
		if (child->m_pimpl->tag.empty() == false) self->AddChild(child);
		GetNextLine(str, pos, line);
	}
}

// Write XML to file
void CXML::XMLWrite(FILE *file,int indent)
{
	std::string line;
	if(indent)
		line = '\n';
	line.append(indent, '\t');
	line += '<';
	line += m_pimpl->tag;
    Impl::ParameterMap::const_iterator pos = m_pimpl->params.begin();
	while(pos != m_pimpl->params.end())
	{
		line += ' ';
		line += pos->first;
		line += "=\"";
		line += PlainToXML(pos->second);
		line += '\"';
		pos++;
	}
	if(m_pimpl->text.empty() && m_pimpl->children.empty())
	{
		line += "/>";
		fwrite(line.c_str(),line.length(),1,file);
		return;
	}
	line+='>';
	fwrite(line.c_str(),line.length(),1,file);
	// Now the text block
	if(m_pimpl->text.empty() == false) {
        std::string tmp = PlainToXML(m_pimpl->text);
		fwrite(tmp.c_str(), tmp.length(), 1, file);
	}
	// Now the children
	CXML *child = GetFirstChild();
	bool newLine = child != 0;
	while(child!=0)
	{
		child->XMLWrite(file,indent+1);
		child = GetNextChild();
	}
	// and the terminator
	line.clear();
	if (newLine) {
		line += '\n';
		line.append(indent, '\t');
	}
	line += "</";
	line += m_pimpl->tag;
	line += '>';
	fwrite(line.c_str(),line.length(),1,file);
	return;
}

// Read XML from file
bool CXML::XMLRead(FILE *file, const char *tag)
{
	CXMLBuffer buffer(file, BufferSize);
    m_pimpl->eof = false;
	m_pimpl->tag = tag;
	std::string nextLine;
	m_pimpl->GetNextLine(buffer, nextLine);
	while (!m_pimpl->eof && !IsStartBlock(nextLine,m_pimpl->tag))
	{
		m_pimpl->GetNextLine(buffer, nextLine);
	}
	m_pimpl->text.clear();

	if(m_pimpl->eof)
		return false;

	m_pimpl->ParseBlock(nextLine, buffer);

	return true;
}

// Convert XML to string
void CXML::XMLToStr(std::string & str)
{
	str += "<";
	str += m_pimpl->tag;
    Impl::ParameterMap::const_iterator pos = m_pimpl->params.begin();
	//browse through all parameters in this tag
	while(pos != m_pimpl->params.end()) {
		str += " ";
		str += pos->first;
		str += "=\"";
		str += PlainToXML(pos->second);
		str += "\"";
		pos++;
	}
	//are there any child XMLs here?
	if(GetChildCount()<1) {
		//no child xmls
		//is there any text here?
		if(GetText()[0] == '\0') {
			//no text
			str += "/>";
		} else {
			//attach text
			str += ">";
			str += PlainToXML(m_pimpl->text);
			str += "</";
			str += m_pimpl->tag;
			str += ">";
		}
		return;
	}
	str += ">";
	if(m_pimpl->text.empty() == false) {
		//attach text
		str += PlainToXML(m_pimpl->text);
	}
	// Now the children
	CXML *child;
	child = GetFirstChild();
	while(child!=0) {
		child->XMLToStr(str);
		child = GetNextChild();
	}
	// and the terminator
	str += "</";
	str += m_pimpl->tag;
	str += ">";
}

bool operator== (const CXML & xml1, const CXML & xml2)
{
    // Simple tag/text checks
    if (xml1.m_pimpl->tag != xml2.m_pimpl->tag)
        return false;
    if (xml1.m_pimpl->text != xml2.m_pimpl->text)
        return false;

    // Parameter count and Children count checks
    const CXML::Impl::ParameterMap & p1 = xml1.m_pimpl->params;
    const CXML::Impl::ParameterMap & p2 = xml2.m_pimpl->params;

    if (p1.size() != p2.size())
        return false;

    const CXML::Impl::ChildNodes & c1 = xml1.m_pimpl->children;
    const CXML::Impl::ChildNodes & c2 = xml2.m_pimpl->children;

    if (c1.size() != c2.size())
        return false;

    // Thorough parameter equality
    if (p1.empty() == false) {
        CXML::Impl::ParameterMap::const_iterator i = p1.begin();
        do {
            const string & key = i->first;
            const string & value = i->second;
            CXML::Impl::ParameterMap::const_iterator j = p2.find(key);
            if (j == p2.end())
                return false;
            if (value != j->second)
                return false;
        } while (++i != p1.end());
    }

    // Thorough child equality
    if (c1.empty() == false) {
        CXML::Impl::ChildNodes::const_iterator i = c1.begin();
        CXML::Impl::ChildNodes::const_iterator j = c2.begin();
        do {
            const CXML *child1 = *i;
            const CXML *child2 = *j;
            // Recursive
            if (*child1 != *child2)
                return false;
            ++j;
        } while (++i != c1.end());
    }

    return true;
}

bool operator!= (const CXML & xml1, const CXML & xml2)
{
    return !operator==(xml1, xml2);
}

}

namespace {

bool IsPrivateTag(const std::string & s)
{
    if (s.length() >= 2) {
        if (s[0] == '<' && s[1] == '?')
            return true;
    }
    
    return false;
}

bool IsCommentTag(const std::string & s)
{
    if (s.length() >= 4) {
        if (s[0] == '<' && s[1] == '!' && s[2] == '-' && s[3] == '-')
            return true;
    }

    return false;
}

bool IsStartBlock(const std::string & line, const std::string & TagName)
{
	if (line.empty() || line[0] != '<') return false;
	if (TagName.empty()) return false;

	size_t endPos = line.find(' ');
	string LineName;
	if (endPos == string::npos) {
		endPos = line.length();
		if (line[endPos - 1] != '>') return false;
		if (line[endPos - 2] == '/') endPos--;
		LineName = line.substr(1, endPos - 2);
	} else {
		LineName = line.substr(1, endPos - 1);
	}

	if (LineName != TagName) return false;

	return true;
}

}
