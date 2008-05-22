// WebWatch Util - by DEATH, 2005
//
// $Workfile: Picture.cpp $ - Picture class
//
// $Author: Death $
// $Revision: 1 $
// $Date: 4/02/05 6:07 $
// $NoKeywords: $
//
#include "stdafx.h"
#include "Picture.h"

namespace
{
    inline int MapLogHimToPix(int x, int ppli)
    {
        const int HimetricPerInch = 2540;
        return MulDiv(ppli, x, HimetricPerInch);
    }

    IStream *StreamFromResource(HINSTANCE instance, 
                                const char *type, 
                                const char *name);
    IPicture *PictureFromStream(IStream *pStream);
    int MakeWidth(HDC hdc, IPicture *pic);
    int MakeHeight(HDC hdc, IPicture *pic);
}

namespace Util_DE050401
{
    PicturePtr Picture::FromResource(HWND hwnd, 
                                    HINSTANCE instance, 
                                    const char *type, 
                                    const char *name)
    {
        PicturePtr result;

        if (IStream *stream = StreamFromResource(instance, type, name)) {
            IPicture *pic = PictureFromStream(stream);
            stream->Release();
            if (pic) {
                if (HDC hdc = GetDC(hwnd)) {
                    int width = MakeWidth(hdc, pic);
                    int height = MakeHeight(hdc, pic);
                    
                    ReleaseDC(hwnd, hdc);

                    if (width && height)
                        result.reset(new Picture(pic, width, height));
                }
                
                if (!result)
                    pic->Release();
            }
        }

        return result;
    }

    Picture::Picture(IPicture *pic, int width, int height)
    : m_pic(pic)
    , m_width(width)
    , m_height(height)
    {
    }

    Picture::~Picture()
    {
        if (m_pic)
            m_pic->Release();
    }

    int Picture::GetWidth() const
    {
        return m_width;
    }

    int Picture::GetHeight() const
    {
        return m_height;
    }

    bool Picture::Draw(HDC hdc, const RECT & rect) const
    {
        bool result = false;

        OLE_XSIZE_HIMETRIC cx;
        if (SUCCEEDED(m_pic->get_Width(&cx))) {
            OLE_YSIZE_HIMETRIC cy;
            if (SUCCEEDED(m_pic->get_Height(&cy))) {
                if (SUCCEEDED(m_pic->Render(hdc, 
                                            rect.left, 
                                            rect.bottom, 
                                            rect.right - rect.left, 
                                            rect.top - rect.bottom, 
                                            0, 0, cx, cy, 
                                            0))) {
                    result = true;
                }
            }
        }

        return result;
    }
}

namespace
{
    IStream *StreamFromResource(HINSTANCE instance, 
                                const char *type, 
                                const char *name)
    {
        IStream *result = 0;

        if (HRSRC hrsrc = FindResource(instance, name, type)) {
            if (HGLOBAL resData = LoadResource(instance, hrsrc)) {
                if (void *src = LockResource(resData)) {
                    SIZE_T size = SizeofResource(instance, hrsrc);
                    if (HANDLE data = GlobalAlloc(GMEM_MOVEABLE, size)) {
                        bool shouldFree = true;
                        if (void *dst = GlobalLock(data)) {
                            std::memcpy(dst, src, size);
                            GlobalUnlock(data);
                            if (CreateStreamOnHGlobal(data, TRUE, &result) == S_OK)
                                shouldFree = false;
                        }
                        if (shouldFree)
                            GlobalFree(data);
                    }
                }
            }
        }

        return result;
    }

    IPicture *PictureFromStream(IStream *stream)
    {
        IPicture *result = 0;
        void **ugly = reinterpret_cast<void **>(&result);
        if (FAILED(OleLoadPicture(stream, 0, FALSE, IID_IPicture, ugly)))
            result = 0;

        return result;
    }

    int MakeWidth(HDC hdc, IPicture *pic)
    {
        OLE_XSIZE_HIMETRIC cx;
        HRESULT hr = pic->get_Width(&cx);
        return SUCCEEDED(hr) ? MapLogHimToPix(cx, GetDeviceCaps(hdc, LOGPIXELSX))
                             : 0;
    }

    int MakeHeight(HDC hdc, IPicture *pic)
    {
        OLE_YSIZE_HIMETRIC cy;
        HRESULT hr = pic->get_Height(&cy);
        return SUCCEEDED(hr) ? MapLogHimToPix(cy, GetDeviceCaps(hdc, LOGPIXELSY))
                             : 0;
    }
}