//--------------------------------------------------------------------------------
// (C) Copyright 2014-2015 Stephane Molina, All rights reserved.
// See https://github.com/Dllieu for updates, documentation, and revision history.
//--------------------------------------------------------------------------------
#ifndef __GENERIC_DESIGNPATTERN_OBSERVER_H__
#define __GENERIC_DESIGNPATTERN_OBSERVER_H__

#include <memory>
#include <set>
#include <algorithm>

namespace designpattern
{

template <class Observer>
class Observable
{
public:
    Observable()
        : isNotifying_( false )
    {
        // NOTHING
    }
 
    inline void subscribe(const std::shared_ptr< Observer >& observer)
    {
        if (!observer)
            return;

        if ( ! isNotifying_ )
        {
            observers_.insert(observer);
            return;
        }

        pendingInsert_.insert(observer);
        pendingErase_.erase(observer);
    }
 
    inline void unsubscribe(const std::shared_ptr< Observer >& observer)
    {
        if (!observer)
            return;

        if ( ! isNotifying_ )
        {
            observers_.erase(observer);
            return;
        }

        pendingInsert_.erase(observer);
        pendingErase_.insert(observer);
    }

    template <class Function, class... Arguments>
    inline void notify(Function&& f, Arguments&&... args)
    {
        isNotifying_ = true;
        std::for_each(std::begin(observers_), std::end(observers_), [&f, &args...](const std::shared_ptr< Observer >& observer){ (observer.get()->*f)(args...); });
        isNotifying_ = false;

        std::for_each(std::begin(pendingErase_), std::end(pendingErase_), [this](const std::shared_ptr< Observer >& observer){ observers_.erase(observer); });
        pendingErase_.clear();
        
        std::for_each(std::begin(pendingInsert_), std::end(pendingInsert_), [this](const std::shared_ptr< Observer >& observer){ observers_.insert(observer); });
        pendingInsert_.clear();
    }
 
private:
    bool isNotifying_; //<! Used in order to not modify observers_ during a notification phase (if the observer notified do a unsubscribe)

    std::set< std::shared_ptr< Observer > > observers_;
    std::set< std::shared_ptr< Observer > > pendingInsert_;
    std::set< std::shared_ptr< Observer > > pendingErase_;
};

}

#endif /* ! __GENERIC_DESIGNPATTERN_OBSERVER_H__ */
