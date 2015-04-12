//--------------------------------------------------------------------------------
// (C) Copyright 2014-2015 Stephane Molina, All rights reserved.
// See https://github.com/Dllieu for updates, documentation, and revision history.
//--------------------------------------------------------------------------------
#ifndef __CONTAINERS_LOCKBASEDQUEUE__
#define __CONTAINERS_LOCKBASEDQUEUE__

#include <memory>
#include <condition_variable>
#include <mutex>

namespace containers
{
    template < typename T >
    struct LockBasedQueue
    {
    public:
        // init head with a dummy pointer : head_ == tail_ -> empty queue
        // Use this dummy so that tail_ and head_ are less linked in the case of an empty queue (instead of doing empty queue -> head_ == tail_ == nullptr)
        LockBasedQueue()
            : head_( new Node )
            , tail_( head_.get() )
        {
            // NOTHING
        }

        // No link to head -> more fine grained lock
        void    push( T data )
        {
            auto newData = std::make_shared< T >( std::move( data ) ); 
            auto newTail = std::make_unique< Node >();
            auto newTailAddress = newTail.get();

            {
                std::lock_guard< std::mutex >   lock( tailMutex_ );
                tail_->data = std::move( newData );
                tail_->next = std::move( newTail );
                tail_ = newTailAddress;
            }
            conditionVariable_.notify_one();
        }

        std::shared_ptr< T >    tryPop()
        {
            auto previousHead = popHead();
            return previousHead ? previousHead->data : nullptr;
        }

        std::shared_ptr< T >    waitAndPop()
        {
            auto previousHead = waitAndPopHead();
            return previousHead->data;
        }

        bool        empty() const
        {
            std::lock_guard< std::mutex >   lock( headMutex_ );
            return head_.get() == getTailAddress();
        }

    private:
        struct Node
        {
            std::shared_ptr< T >        data;
            std::unique_ptr< Node >     next;
        };

    private:
        Node*   getTailAddress() const
        {
            std::lock_guard< std::mutex >   lock( tailMutex_ );
            return tail_;
        }

        std::unique_ptr< Node >    popHead()
        {
            std::lock_guard< std::mutex >   lock( headMutex_ );
            return notThreadSafePopHead();
        }

        std::unique_lock< std::mutex >      waitForNewHead()
        {
            std::unique_lock< std::mutex >  lock( headMutex_ );

            conditionVariable_.wait( lock, [&] { return head_.get() != getTailAddress(); } );
            return std::move( lock );
        }

        std::unique_ptr< Node >     waitAndPopHead()
        {
            std::unique_lock< std::mutex >  lock( waitForNewHead() );
            return notThreadSafePopHead();
        }

        // Very ugly, find a way to factor the code in a cleaner way
        std::unique_ptr< Node >     notThreadSafePopHead()
        {
            if ( head_.get() == getTailAddress() )
                return nullptr;

            auto previousHead = std::move( head_ );
            head_ = std::move( previousHead->next );

            return previousHead;
        }

    private:
        std::unique_ptr< Node >     head_;
        mutable std::mutex          headMutex_;

        Node*                       tail_; // owned by head_ ror one of it's sub-node
        mutable std::mutex          tailMutex_;

        std::condition_variable     conditionVariable_;
    };
}

#endif /* ! __CONTAINERS_LOCKBASEDQUEUE__ */
