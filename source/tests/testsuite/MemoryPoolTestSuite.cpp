#include <boost/test/unit_test.hpp>

#include "tools/MemoryPool.h"
#include "tools/Timer.h"

BOOST_AUTO_TEST_SUITE( MemoryPool )

namespace
{
    class NoAllocator
    {
        char        buff[4096];
    };

    class CustomAllocator : public NoAllocator
    {
    public:
        /*static*/ void*   operator new( size_t size )
        {
            return memoryPool_.alloc( size );
        }

        /*static*/ void    operator delete( void * p )
        {
            memoryPool_.free( p );
        }

    private:
        static tools::MemoryPool   memoryPool_;
    };

    tools::MemoryPool   CustomAllocator::memoryPool_( 50, 4096 );

    template <class Allocator>
    double    testMemoryPool( unsigned int numberOfAllocs )
    {
        tools::Timer    t;

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
    unsigned int to = 100000;

    double basicAllocator = testMemoryPool< NoAllocator >( to );
    double customAllocator = testMemoryPool< CustomAllocator >( to );

    BOOST_CHECK( basicAllocator > customAllocator );
}

BOOST_AUTO_TEST_SUITE_END() // MemoryPool
