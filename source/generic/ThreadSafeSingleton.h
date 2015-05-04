//--------------------------------------------------------------------------------
// (C) Copyright 2014-2015 Stephane Molina, All rights reserved.
// See https://github.com/Dllieu for updates, documentation, and revision history.
//--------------------------------------------------------------------------------
#ifndef __GENERIC_DESIGNPATTERN_THREADSAFESINGLETON_H__
#define __GENERIC_DESIGNPATTERN_THREADSAFESINGLETON_H__

#include <memory>
#include <mutex>

namespace designpattern
{

// Inherit using CRTP idiom
template < typename T >
class ThreadSafeSingleton
{
public:
    static T& instance()
    {
        // Both of these member are declared static to avoid user to redefine them per type T
        static std::unique_ptr< T >     singleton;
        static std::once_flag           onceFlag;

        // No capture as singleton is not on the stack, can be accessed directly through static storage
        // If call_once throw an exception, next call to instance() will retry the call_once
        std::call_once( onceFlag, []() { singleton.reset( new T ); } );
        return *singleton;
    }
};

}

#endif /* ! __GENERIC_DESIGNPATTERN_THREADSAFESINGLETON_H__ */
