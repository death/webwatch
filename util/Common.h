// WebWatch UI - by DEATH, 2004
//
// $Workfile: Common.h $ - Common utilities
//
// $Author: Death $
// $Revision: 2 $
// $Date: 4/02/05 3:58 $
// $NoKeywords: $
//
#ifndef WEBWATCH_UTIL_INC_COMMON_H
#define WEBWATCH_UTIL_INC_COMMON_H

namespace Util_DE050401
{
    // Deletes the object passed and sets its pointer to zero
    struct SafeDelete
    {
        template<typename T>
        void operator() (T *& p)
        {
            delete p;
            p = 0;
        }
    };

    // Dereferences the pointer to the object passed and inserts it to an output
    // stream
    template<class OutputStream, typename T>
    class DereferencingStreamInserter : public std::unary_function<const T *, void>
    {
    public:
        DereferencingStreamInserter(OutputStream & os)
        : m_os(os)
        {
        }

        void operator() (const T *obj)
        {
            m_os << *obj;
        }

    private:
        OutputStream & m_os;
    };

    // Inserts an object to an output stream
    template<class OutputStream, typename T>
    class StreamInserter : public std::unary_function<T, void>
    {
    public:
        StreamInserter(OutputStream & os)
        : m_os(os)
        {
        }

        void operator() (const T & obj)
        {
            m_os << obj;
        }

    private:
        OutputStream & m_os;
    };

    template<class InIter, class OutIter, class Pred>
    inline OutIter CopyIf(InIter first, InIter last, OutIter dest, Pred pred)
    {
        for (; first != last; ++first) {
            if (pred(*first)) {
                *dest = *first;
                ++dest;
            }
        }

        return dest;
    }

    template<typename Pair>
    struct Select1st : public std::unary_function<Pair, typename Pair::first_type>
    {
        const typename Pair::first_type & operator() (const Pair & p) const
        {
            return p.first;
        }
    };

    template<typename Pair>
    struct Select2nd : public std::unary_function<Pair, typename Pair::second_type>
    {
        const typename Pair::second_type & operator() (const Pair & p) const
        {
            return p.second;
        }
    };
}

#endif // WEBWATCH_UTIL_INC_COMMON_H