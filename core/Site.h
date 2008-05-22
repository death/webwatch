// WebWatch Core - by DEATH, 2004
//
// $Workfile: Site.h $ - WebWatch Site
//
// $Author: Death $
// $Revision: 2 $
// $Date: 4/02/05 3:58 $
// $NoKeywords: $
//
#ifndef WEBWATCH_INC_SITE_H
#define WEBWATCH_INC_SITE_H

#include "CoreException.h"
#include "CheckMethod.h" // Included so client won't have to include

namespace Core_DE050401
{
    class Site
    {
        friend inline XML::CXML & operator<< (XML::CXML & xmlParent, const Site & site);
        friend bool operator== (const Site & site1, const Site & site2);
        friend bool operator!= (const Site & site1, const Site & site2);

    public:
        explicit Site(const char *address);
        Site(const Site & other);
        Site(const XML::CXML & xml);
        virtual ~Site();

        Site & operator= (const Site & rhs);

        void SetAddress(const char *address);
        std::string GetAddress() const;
        std::string GetType() const;

        void SetCheckParameters(const XML::CXML & parameters);
        void GetCheckParameters(XML::CXML & parameters) const;

        void SetCheckResults(const XML::CXML & results);
        void GetCheckResults(XML::CXML & results) const;
        bool AreCheckResultsAvailable() const;
        bool IsUpdated(const XML::CXML & results) const;

        void GetCheckTime(XML::CXML & checkTime) const;

        void Check(XML::CXML & results, StatusNotifyClient *statusNotifyClient = 0) const;
        void Abort() const;

        static const char *GetXMLName();

    protected:
        virtual XML::CXML *Write(XML::CXML & xmlParent) const;

    private:
        struct Impl;
        std::auto_ptr<Impl> m_pimpl;
    };

    inline XML::CXML & operator<< (XML::CXML & xmlParent, const Site & site)
    {
        site.Write(xmlParent);
        return xmlParent;
    }

    class SiteError : public Exception
    {
    public:
        SiteError(const std::string & error)
        : Exception(error)
        {
        }
    };
}

#endif // WEBWATCH_INC_SITE_H