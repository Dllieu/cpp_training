//--------------------------------------------------------------------------------
// (C) Copyright 2014-2015 Stephane Molina, All rights reserved.
// See https://github.com/Dllieu for updates, documentation, and revision history.
//--------------------------------------------------------------------------------
#ifndef __CONTAINERS_LOCKFREEQUEUE__
#define __CONTAINERS_LOCKFREEQUEUE__

#include <memory>
#include <atomic>

namespace containers
{
    // ReferenceCountingPolicy
    template < typename T >
    class LockFreeStack
    {
    private:
        struct Node;
        struct NodeCounter
        {
            NodeCounter( int c, Node* n )
                : externalCount( c )
                , ptr( n )
            {}

            NodeCounter()
                : NodeCounter( 0, nullptr )
            {}

            int     externalCount; // how many time we have read the node
            Node*   ptr;
        };

        struct Node
        {
            std::shared_ptr< T >    data;
            std::atomic< int >      internalCount;
            NodeCounter             next;

            Node( const T& refData )
                : data( std::make_shared< T >( refData ) )
                , internalCount( 0 )
            {
                // NOTHING
            }
        };

        std::atomic< NodeCounter >  head_;

        void    increaseHeadNodeExternalCount( NodeCounter& previousCounter )
        {
            NodeCounter newCounter;
            do
            {
                newCounter = previousCounter;
                ++newCounter.externalCount; // also avoid the ABA problem
            } while ( ! head_.compare_exchange_strong( previousCounter, newCounter, std::memory_order_acquire /* success */, std::memory_order_relaxed /* failure */ ) );

            previousCounter.externalCount = newCounter.externalCount;
        }

    public:
        ~LockFreeStack()
        {
            while ( pop() );
        }

        void    push( const T& data )
        {
            NodeCounter node( 1, new Node( data ) ); // only one reference to this node (head_)
            node.ptr->next = head_.load( std::memory_order_relaxed );
            while ( ! head_.compare_exchange_weak( node.ptr->next, node, std::memory_order_release, std::memory_order_relaxed ) );
        }

        std::shared_ptr< T >    pop()
        {
            auto previousHead = head_.load( std::memory_order_relaxed );

            for (;;)
            {
                // increment the external reference count, to ensure that the pointer remains valid for the duration of the access
                increaseHeadNodeExternalCount( previousHead );
                const auto ptr = previousHead.ptr;

                if ( ptr == nullptr )
                    return std::shared_ptr< T >();

                if ( head_.compare_exchange_strong( previousHead, ptr->next, std::memory_order_relaxed ) )
                {
                    // previousHead is not on the stack anymore
                    std::shared_ptr< T > result;
                    result.swap( ptr->data );

                    auto countIncrease = previousHead.externalCount - 2; // -2 : removed node from the list / this thread no longer need this node
                    // Need to do this check in case where the ptr could still be referenced (previousHead is not on the stack but the pointer could have been used prior to that)
                    // will delete the ptr either here or lower
                    if ( ptr->internalCount.fetch_add( countIncrease, std::memory_order_release ) == -countIncrease ) // i.e. countIncrease + ptr->internalCount == 0
                        delete ptr;

                    return result;
                }

                if ( ptr->internalCount.fetch_sub( 1, std::memory_order_relaxed ) == 1 )
                {
                    // The node referencing this ptr have already been removed from the stack (see upper), but the ptr was shared with this thread only
                    // this thread have the responsability to delete the ptr
                    ptr->internalCount.load( std::memory_order_acquire );
                    delete ptr;
                }
            }
        }
    };
}

#endif /* __CONTAINERS_LOCKFREEQUEUE__ */
