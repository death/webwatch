// WebWatch Util - by DEATH, 2004
//
// $Workfile: SocketUtil.cpp $ - WebWatch socket utilities implementation
//
// $Author: Death $
// $Revision: 3 $
// $Date: 8/05/05 20:31 $
// $NoKeywords: $
//
#include "stdafx.h"

#include "SocketUtil.h"

namespace Util_DE050401
{
    std::string BinToBase64(const void *in_, size_t len)
    {
        const unsigned char *in = static_cast<const unsigned char *>(in_);
        const char * const B64Chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
        std::string out;

        while (len >= 3) {
            out.push_back(B64Chars[in[0] >> 2]);
            out.push_back(B64Chars[((in[0] << 4) & 0x30) | (in[1] >> 4)]);
            out.push_back(B64Chars[((in[1] << 2) & 0x3c) | (in[2] >> 6)]);
            out.push_back(B64Chars[in[2] & 0x3f]);
            len -= 3;
            in += 3;
        }

        /* clean up remainder */
        if (len > 0) {
            unsigned char fragment;
            out.push_back(B64Chars[in[0] >> 2]);
            fragment = (in[0] << 4) & 0x30;
            if (len > 1)
                fragment |= in[1] >> 4;
            out.push_back(B64Chars[fragment]);
            out.push_back((len < 2) ? '=' : B64Chars[(in[1] << 2) & 0x3c]);
            out.push_back('=');
        }

        return out;
    }
}
