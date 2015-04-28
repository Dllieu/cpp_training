//--------------------------------------------------------------------------------
// (C) Copyright 2014-2015 Stephane Molina, All rights reserved.
// See https://github.com/Dllieu for updates, documentation, and revision history.
//--------------------------------------------------------------------------------
#ifndef __THREADING_SEMAPHORESINGLEPROCESS_H__
#define __THREADING_SEMAPHORESINGLEPROCESS_H__

#include <mutex>

namespace threading
{
    class SemaphoreSingleProcess
    {
    public:
        SemaphoreSingleProcess( unsigned long initialCount = 1 )
            : count_( initialCount )
        {
            // NOTHING
        }

        SemaphoreSingleProcess( const SemaphoreSingleProcess& ) = delete;
        SemaphoreSingleProcess& operator=( const SemaphoreSingleProcess& ) = delete;

        void    signal( unsigned long count = 1 )
        {
            std::lock_guard< std::mutex > lock( mutex_ );

            count_ += count;
            do
            {
                conditionVariable_.notify_one();
            } while ( --count );
        }

        void    wait()
        {
            std::unique_lock< std::mutex > lock( mutex_ );
            while ( ! count_ )
                conditionVariable_.wait( lock );

            --count_;
        }

    private:
        unsigned long               count_;
        std::mutex                  mutex_;
        std::condition_variable     conditionVariable_;

        friend class std::lock_guard< SemaphoreSingleProcess >;
        void    unlock() { signal( 1 ); }
        void    lock() { wait(); }
    };
}

#endif /* ! __THREADING_SEMAPHORESINGLEPROCESS_H__ */
