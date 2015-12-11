//--------------------------------------------------------------------------------
// (C) Copyright 2014-2015 Stephane Molina, All rights reserved.
// See https://github.com/Dllieu for updates, documentation, and revision history.
//--------------------------------------------------------------------------------
#include <boost/test/unit_test.hpp>
#include <boost/pool/pool.hpp>
#include <boost/bind.hpp>
#include <boost/function.hpp>

#include "tools/MemoryPool.h"
#include "tools/Timer.h"

BOOST_AUTO_TEST_SUITE( MemoryPool )

namespace
{
    class NoAllocator
    {
    public:
        /*static*/ void*   operator new( size_t /*size*/ )
        {
            return 0;
        }

        /*static*/ void    operator delete( void * /*p*/ )
        {
            // NOTHING
        }

    private:
        char        buff[4096];
    };

    class BasicAllocator
    {
        char        buff[4096];
    };

    template <class Allocator>
    double    testMemoryPool( unsigned int numberOfAllocs, const std::string& timerMessage )
    {
        return tools::Timer::named_elapsed( timerMessage, [numberOfAllocs]
            {
                for ( unsigned int i = 0; i < numberOfAllocs; ++i )
                    std::make_unique< Allocator >();
            } );
    }
}

BOOST_AUTO_TEST_CASE( MemoryPoolTest )
{
    unsigned int to = 1'000'000;
    double noAllocator, pool, basic;

    noAllocator = testMemoryPool< NoAllocator >( to, "No Allocator" );
    {
        tools::Timer        t( "Boost Pool" );
        boost::pool<>       memoryPool( 4'096, 50 );
        size_t              sizeToAllocate( sizeof( BasicAllocator ) );

        for ( unsigned int i = 0; i < to; ++i )
        {
            void*   buffer = memoryPool.malloc(); // always malloc a chunk of 4096

            // use ::new + static_cast<void*> to avoid having the placement new hijacked (users could have overload taking NonVoid*)
            BasicAllocator* p = ::new ( static_cast<void*>( buffer ) ) BasicAllocator;
            p->~BasicAllocator();

            memoryPool.free( p );
        }
    }
    {
        tools::Timer        t( "Custom Pool" );
        tools::MemoryPool   memoryPool( 50, 4'096 );
        size_t              sizeToAllocate( sizeof( BasicAllocator ) );

        for ( unsigned int i = 0; i < to; ++i )
        {
            void*   buffer = memoryPool.malloc( sizeToAllocate );

            BasicAllocator* p = ::new ( static_cast<void*>( buffer ) ) BasicAllocator;
            p->~BasicAllocator();

            memoryPool.free( p );
        }
        pool = t.elapsed();
    }
    basic = testMemoryPool< BasicAllocator >( to, "Basic Allocator" );

    BOOST_CHECK( noAllocator < pool && pool < basic );
}

namespace
{
    template <class Allocator>
    double    testMemoryPool( unsigned int numberOfAllocs,
                              const std::string& timerMessage,
                              std::function< void* () > mallocFunction,
                              std::function< void ( void* ) > freeFunction )
    {
        return tools::Timer::named_elapsed( timerMessage, [numberOfAllocs, &mallocFunction, &freeFunction]
        {
            for ( unsigned int i = 0; i < numberOfAllocs; ++i )
            {
                void*   buffer = mallocFunction();

                auto p = ::new ( static_cast<void*>( buffer ) ) Allocator;
                p->~Allocator();

                freeFunction( p );
            }
        } );
    }
}

BOOST_AUTO_TEST_CASE( SlowMemoryPoolTest )
{
    // slower because boost::function (even function from c++11) :
    //     - can't be inlined by the compiler
    //     - boost::function implements type-erasure which means it uses indirection to invoke the actual function.
    //       Means it first calls a virtual function which then invokes your function. So typically it involves (minimum) two function calls (one of them is virtual)
    // boost::function < virtual function < regular function (fastest)

    unsigned int to = 1'000'000;
    { 
        boost::pool<> memoryPool( 4'096, 50 ); // always malloc by chunk of 4096
        testMemoryPool< BasicAllocator >( to, "Slow Boost Pool", boost::bind( &boost::pool<>::malloc, boost::ref( memoryPool ) )
                                                               , boost::bind( &boost::pool<>::free, boost::ref( memoryPool ), _1 ) );
    }
    {
        tools::MemoryPool   memoryPool( 50, 4'096 );
        size_t              sizeToAllocate( sizeof( BasicAllocator ) );
        testMemoryPool< BasicAllocator >( to, "Slow Custom Pool", boost::bind( &tools::MemoryPool::malloc, boost::ref( memoryPool ), sizeToAllocate )
                                                                , boost::bind( &tools::MemoryPool::free, boost::ref( memoryPool ), _1 ) );
    }
    BOOST_CHECK( true );
}

BOOST_AUTO_TEST_SUITE_END() // MemoryPool
