// WebWatch Core - by DEATH, 2004
//
// $Workfile: NumberStripper.cpp $ - Number stripper filter implementation
//
// $Author: Death $
// $Revision: 2 $
// $Date: 4/02/05 3:58 $
// $NoKeywords: $
//
#include "StdAfx.h"

#include "util/Util.h"
#include "util/Common.h"

namespace Core_DE050401
{
    namespace
    {
        class IsDigit : public std::unary_function<char, bool>
        {
        public:
            bool operator() (unsigned char c) const { return std::isdigit(c); }
        };
    }

    void StripNumbers(std::istream & is, std::ostream & os)
    {
        std::istreambuf_iterator<char> in_begin(is);
        std::istreambuf_iterator<char> in_end;
        std::ostreambuf_iterator<char> out(os);

        Util::CopyIf(in_begin, in_end, out, std::not1(IsDigit()));
    }
}
