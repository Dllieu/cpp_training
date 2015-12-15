//--------------------------------------------------------------------------------
// (C) Copyright 2014-2015 Stephane Molina, All rights reserved.
// See https://github.com/Dllieu for updates, documentation, and revision history.
//--------------------------------------------------------------------------------
#ifndef __THREADING_ALGORITHM_H__
#define __THREADING_ALGORITHM_H__

#include <future>
#include <algorithm>

#define ALGORITHM_SPLITLENGTH 25

// Recursion based for code clarity, could use an iterative way with promise / future
// Beware of false sharing
namespace threading
{
    template < typename It, typename F >
    void    parallel_for_each( It begin, It end, F f, int splitLength = ALGORITHM_SPLITLENGTH )
    {
        auto length = std::distance( begin, end );
        if ( ! length )
            return;

        if ( length < 2 * splitLength )
        {
            std::for_each( begin, end, f );
            return;
        }

        auto halfIt = std::next( begin, length / 2 );
        auto future = std::async( [&] { parallel_for_each< It, F >( begin, halfIt, f, splitLength ); } );
        parallel_for_each( halfIt, end, f, splitLength );

        future.get();
    }

    template < typename It, typename T >
    It      parallel_find_impl( It begin, It end, T toMatch, int splitLength, std::atomic< bool >& isDone )
    {
        auto length = std::distance( begin, end );
        if ( isDone.load() || length < splitLength * 2 )
        {
            bool casSwapValue = false;
            for ( ; begin != end && ! isDone.load(); ++begin )
                if ( *begin == toMatch && isDone.compare_exchange_strong( casSwapValue /* false */, true ) )
                    return begin;

            return end;
        }

        auto halfIt = std::next( begin, length / 2 );
        auto future = std::async( [ =, &isDone ] { return parallel_find_impl( halfIt, end, toMatch, splitLength, isDone ); } );
        auto currentResult = parallel_find_impl( begin, halfIt, toMatch, splitLength, isDone );

        // weird memory leak on vc120 if not explicitly getting the future, altough the destructor of async_result should join
        auto futureResult = future.get();
        return currentResult == halfIt ? futureResult /*future.get() : leak sometimes*/ : currentResult;
    }

    template < typename It, typename T >
    It  parallel_find( It begin, It end, T toMatch, int splitLength = ALGORITHM_SPLITLENGTH )
    {
        std::atomic< bool > isDone( false );
        return parallel_find_impl( begin, end, toMatch, splitLength, isDone );
    }
}

#endif /* ! __THREADING_ALGORITHM_H__ */
