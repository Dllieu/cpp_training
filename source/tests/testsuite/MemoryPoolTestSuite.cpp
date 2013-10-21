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
        tools::Timer    t( timerMessage );

        for ( unsigned int i = 0; i < numberOfAllocs; ++i )
        {
            Allocator* p = new Allocator;
            delete p;
        }
        return t.elapsed();
    }
}

BOOST_AUTO_TEST_CASE( MemoryPoolTestSuite )
{
    unsigned int to = 1000000;
    double noAllocator, pool, basic;

    noAllocator = testMemoryPool< NoAllocator >( to, "No Allocator" );
    {
        tools::Timer        t( "Boost Pool" );
        boost::pool<>       memoryPool( 4096, 50 );
        size_t              sizeToAllocate( sizeof( BasicAllocator ) );

        for ( unsigned int i = 0; i < to; ++i )
        {
            void*   buffer = memoryPool.malloc(); // always malloc a chunk of 4096

            BasicAllocator* p = new (buffer) BasicAllocator;
            p->~BasicAllocator();

            memoryPool.free( p );
        }
    }
    {
        tools::Timer        t( "Custom Pool" );
        tools::MemoryPool   memoryPool( 50, 4096 );
        size_t              sizeToAllocate( sizeof( BasicAllocator ) );

        for ( unsigned int i = 0; i < to; ++i )
        {
            void*   buffer = memoryPool.malloc( sizeToAllocate );

            BasicAllocator* p = new (buffer) BasicAllocator;
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
    double    testMemoryPool( unsigned int numberOfAllocs, const std::string& timerMessage
                            , boost::function< void* () > mallocFunction
                            , boost::function< void ( void* ) > freeFunction )
    {
        tools::Timer    t( timerMessage );

        for ( unsigned int i = 0; i < numberOfAllocs; ++i )
        {
            void*   buffer = mallocFunction();

            Allocator* p = new (buffer) Allocator;
            p->~Allocator();

            freeFunction( p );
        }
        return t.elapsed();
    }
}

BOOST_AUTO_TEST_CASE( SlowMemoryPoolTestSuite )
{
    // slower because boost::function (even function from c++11) :
    //     - can't be inlined by the compiler
    //     - boost::function implements type-erasure which means it uses indirection to invoke the actual function.
    //       Means it first calls a virtual function which then invokes your function. So typically it involves (minimum) two function calls (one of them is virtual)
    // boost::function < virtual function < regular function (fastest)

    unsigned int to = 1000000;
    { 
        boost::pool<> memoryPool( 4096, 50 ); // always malloc by chunk of 4096
        testMemoryPool< BasicAllocator >( to, "Slow Boost Pool", boost::bind( &boost::pool<>::malloc, boost::ref( memoryPool ) )
                                                               , boost::bind( &boost::pool<>::free, boost::ref( memoryPool ), _1 ) );
    }
    {
        tools::MemoryPool   memoryPool( 50, 4096 );
        size_t              sizeToAllocate( sizeof( BasicAllocator ) );
        testMemoryPool< BasicAllocator >( to, "Slow Custom Pool", boost::bind( &tools::MemoryPool::malloc, boost::ref( memoryPool ), sizeToAllocate )
                                                                , boost::bind( &tools::MemoryPool::free, boost::ref( memoryPool ), _1 ) );
    }
    BOOST_CHECK( true );
}

BOOST_AUTO_TEST_SUITE_END() // MemoryPool
