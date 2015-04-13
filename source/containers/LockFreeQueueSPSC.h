//--------------------------------------------------------------------------------
// (C) Copyright 2014-2015 Stephane Molina, All rights reserved.
// See https://github.com/Dllieu for updates, documentation, and revision history.
//--------------------------------------------------------------------------------
#ifndef __CONTAINERS_LOCKFREEQUEUESPSC_H__
#define __CONTAINERS_LOCKFREEQUEUESPSC_H__

namespace containers
{
    template < typename T >
    class LockFreeQueueSPSC // Single Producer Single Consumer
    {
    private:
        struct Node
        {
            Node()
                : next( nullptr )
            {
                // NOTHING
            }

            std::shared_ptr< T >    data;
            Node*                   next;
        };

        std::atomic< Node* >    head_;
        std::atomic< Node* >    tail_;

        Node*   popHead()
        {
            auto previousHead = head_.load();
            if ( previousHead == tail_.load() )
                return nullptr;

            head_.store( previousHead->next );
            return previousHead;
        }

    public:
        LockFreeQueueSPSC()
            : head_( new Node )
            , tail_( head_.load() )
        {
            // NOTHING
        }

        ~LockFreeQueueSPSC()
        {
            while ( auto previousHead = head_.load() )
            {
                head_.store( previousHead->next );
                delete previousHead;
            }
        }

        std::shared_ptr< T > pop()
        {
            auto previousHead = popHead();
            if ( previousHead == nullptr )
                return std::shared_ptr< T >();

            auto result = previousHead->data;
            delete previousHead;
            return result;
        }

        void    push( T data )
        {
            auto newTail = new Node;

            auto previousTail = tail_.load();
            previousTail->data = std::make_shared< T >( std::move( data ) );
            previousTail->next = newTail;

            tail_.store( newTail );
        }
    };
}

#endif /* ! __CONTAINERS_LOCKFREEQUEUESPSC_H__ */
