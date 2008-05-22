// WebWatch Util - by DEATH, 2004
//
// $Workfile: Observer.h $ - Observer design pattern implementation
//
// $Author: Death $
// $Revision: 2 $
// $Date: 4/02/05 3:58 $
// $NoKeywords: $
//
#ifndef WEBWATCH_UTIL_INC_OBSERVER_H
#define WEBWATCH_UTIL_INC_OBSERVER_H

namespace Util_DE050401
{
    template<class SUBJECT>
    class Observer
    {
    public:
        virtual ~Observer() {}
        virtual void UpdateObserver(SUBJECT & subject) = 0;
    };

    template<class SUBJECT>
    class Subject
    {
    public:
        typedef Observer<SUBJECT> OBSERVER;

    private:
        // NOTE: If necessary, we could implement reference counting
        typedef std::list<OBSERVER *> Observers;

        class NotifyObserver : public std::unary_function<OBSERVER *, void>
        {
        public:
            NotifyObserver(SUBJECT & subject)
            : m_subject(subject)
            {
            }

            void operator() (OBSERVER *observer)
            {
                observer->UpdateObserver(m_subject);
            }

        private:
            SUBJECT & m_subject;
        };

    public:
        virtual ~Subject() {}

        void AttachObserver(OBSERVER *observer)
        {
            Observers::const_iterator it = std::find(m_observers.begin(), m_observers.end(), observer);
            if (it == m_observers.end())
                m_observers.push_back(observer);
        }

        void DetachObserver(OBSERVER *observer)
        {
            m_observers.remove(observer);
        }

        void NotifyObservers()
        {
            // I don't use std::for_each() since it caches end() iterator
            // std::for_each(m_observers.begin(), m_observers.end(), NotifyObserver(static_cast<SUBJECT &>(*this)));
            Observers::iterator it = m_observers.begin();
            NotifyObserver notifier(static_cast<SUBJECT &>(*this));
            while (it != m_observers.end())
                notifier(*it++);
        }

    protected:
        Subject()
        {
        }

    private:
        Observers m_observers;
    };
}

#endif // WEBWATCH_UTIL_INC_OBSERVER_H
