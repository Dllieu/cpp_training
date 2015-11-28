//--------------------------------------------------------------------------------
// (C) Copyright 2014-2015 Stephane Molina, All rights reserved.
// See https://github.com/Dllieu for updates, documentation, and revision history.
//--------------------------------------------------------------------------------
#pragma once

// Stolen from https://github.com/progschj/ThreadPool

#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <future>

namespace threading
{
    class ThreadPool
    {
    public:
        ThreadPool( size_t threadNumber );
        ~ThreadPool();

        template < typename F, typename... Args >
        auto enqueue( F&& f, Args&&... args ) -> std::future< std::result_of_t< F( Args... ) > >;

    private:
        // need to keep track of threads so we can join them
        std::vector< std::thread >          workers_;

        // the task queue
        std::queue< std::function<void()> > tasks_;

        // synchronization
        std::mutex                          queueMutex_;
        std::condition_variable             conditionVariable_;
        bool                                stop_;
    };
}

#include "ThreadPool.hxx"
