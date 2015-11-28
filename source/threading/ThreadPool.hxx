#include <stdexcept>
#include <memory>
#include <functional>

#include "ThreadPool.h"


namespace threading
{
    inline ThreadPool::ThreadPool( size_t threadNumber )
        : stop_( false )
    {
        for ( auto i = 0; i < threadNumber; ++i )
            workers_.emplace_back(
                [ this ]
                {
                    for ( ;; )
                    {
                        std::function<void()> task;

                        {
                            std::unique_lock<std::mutex> lock( queueMutex_ );
                            conditionVariable_.wait( lock, [ this ] { return stop_ || !tasks_.empty(); } );

                            if ( stop_ && tasks_.empty() )
                                return;

                            task = std::move( tasks_.front() );
                            tasks_.pop();
                        }

                        task();
                    }
                });
    }

    // the destructor joins all threads
    inline ThreadPool::~ThreadPool()
    {
        {
            std::unique_lock<std::mutex> lock( queueMutex_ );
            stop_ = true;
        }
        conditionVariable_.notify_all();

        for ( std::thread& worker : workers_ )
            worker.join();
    }

    // add new work item to the pool
    template < typename F, typename... Args >
    auto ThreadPool::enqueue( F&& f, Args&&... args ) -> std::future< std::result_of_t< F( Args... ) > >
    {
        using return_type = std::result_of_t< F( Args... ) >;

        auto task = std::make_shared< std::packaged_task< return_type() > >( std::bind( std::forward< F >( f ), std::forward< Args >( args )... ) );
        std::future< return_type > res = task->get_future();

        {
            std::unique_lock<std::mutex> lock( queueMutex_ );

            // Don't allow enqueueing after stopping the pool
            if ( stop_ )
                throw std::runtime_error( "enqueue on stopped ThreadPool" );

            tasks_.emplace( [ task ] () { ( *task )(); } );
        }
        conditionVariable_.notify_one();
        return res;
    }
}
