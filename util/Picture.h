// WebWatch Util - by DEATH, 2005
//
// $Workfile: Picture.h $ - Picture class
//
// $Author: Death $
// $Revision: 1 $
// $Date: 4/02/05 6:07 $
// $NoKeywords: $
//
#ifndef WEBWATCH_UTIL_INC_PICTURE_H
#define WEBWATCH_UTIL_INC_PICTURE_H

namespace Util_DE050401
{
    class Picture;
    typedef boost::shared_ptr<Picture> PicturePtr;

    class Picture
    {
    public:
        static PicturePtr FromResource(HWND hwnd,
                                       HINSTANCE instance, 
                                       const char *type, 
                                       const char *name);

        ~Picture();

        int GetWidth() const;
        int GetHeight() const;

        bool Draw(HDC hdc, const RECT & rect) const;

    private:
        explicit Picture(IPicture *pic, int width, int height);
        
        Picture(const Picture &);
        Picture & operator= (const Picture &);

    private:
        IPicture *m_pic;
        int m_width;
        int m_height;
    };
}

#endif // WEBWATCH_UTIL_INC_PICTURE_H
