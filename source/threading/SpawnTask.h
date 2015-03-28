//--------------------------------------------------------------------------------
// (C) Copyright 2014-2015 Stephane Molina, All rights reserved.
// See https://github.com/Dllieu for updates, documentation, and revision history.
//--------------------------------------------------------------------------------
#ifndef __THREADING_SPAWMTASK_H__
#define __THREADING_SPAWMTASK_H__

#include <thread>
#include <future>

namespace threading
{
    // Not much use, kind of similar to std::async with std::launch::async policy, will lead to massive oversubscription
    template < typename FUNCTOR, typename... ARGS >
    std::future< typename std::result_of< FUNCTOR ( ARGS&&... ) >::type >   spawnTask( FUNCTOR&& functor, ARGS&&... args )
    {
        using resultFutureType = std::result_of_t< FUNCTOR( ARGS&&... ) >;
        std::packaged_task< resultFutureType ( ARGS&&... ) > task( std::forward< FUNCTOR >( functor ) );

        std::future< resultFutureType > future( task.get_future() );

        std::thread t( std::move( task ), std::forward< ARGS >( args )... );
        t.detach();

        return future;
    }
}

#endif /* ! __THREADING_SPAWMTASK_H__ */
