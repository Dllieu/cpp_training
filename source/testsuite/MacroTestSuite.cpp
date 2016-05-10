//--------------------------------------------------------------------------------
// (C) Copyright 2014-2015 Stephane Molina, All rights reserved.
// See https://github.com/Dllieu for updates, documentation, and revision history.
//--------------------------------------------------------------------------------
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_SUITE( MacroTestSuite )

#define CT_ASSERT( ... ) \
    static_assert( __VA_ARGS__, #__VA_ARGS__ )

#define LOG_TYPE( type ) \
    BOOST_TEST_MESSAGE( #type << ": " << sizeof( type ) )
    
BOOST_AUTO_TEST_CASE( TypeTest )
{
    // constexpr int i = 2;
    // CT_ASSERT( i != 2 ); // error C2338: i != 2

    LOG_TYPE( bool );
    LOG_TYPE( char );
    LOG_TYPE( short );
    LOG_TYPE( int );
    LOG_TYPE( long );
    LOG_TYPE( float );
    LOG_TYPE( double );
    LOG_TYPE( long double );
    LOG_TYPE( std::string );

    BOOST_CHECK( true );
}

#undef CT_ASSERT
#undef LOG_TYPE

// - Preventing optimizations
// folly code
// Rather than using volatile keyword that could modify greatly the code generated
#ifdef _MSC_VER
    #pragma optimize("", off)
    template <class T>
    void    do_not_optimize_away( T&& datum )
    {
        // couldn't make it work (optimization still taking place, using volatile instead on vc140 :( )
        datum = datum;
    }
    #pragma optimize("", on)
#elif defined(__clang__)
    template <class T>
    __attribute__( ( __optnone__ ) ) void   do_not_optimize_away( T&& datum )
    {}
#else /*e.g. GCC*/
    template <class T>
    void    do_not_optimize_away( T&& datum )
    {
        asm volatile( "" : "+r" ( datum ) ); // only prevent optimizations on datum
    }
    // asm volatile ("") prevent optimizations
    // Also, it could be usefull to use clobbering (e.g. : asm volatile ("" ::: "memory")) : creates a compiler level memory barrier forcing optimizer to not re-order memory accesses across the barrier
    // This will cause GCC to not keep memory values cached in registers across the assembler instruction and not optimize stores or loads to that memory.
    // That does not prevent a CPU from reordering loads and stores with respect to another CPU, though; you need real memory barrier instructions for that.
#endif

// - likely / unlikely
// GCC macros that tell the processor whether a condition is likely to be true, so that the processor can prefetch instructions on the correct "side" of the branch
// Arrange the code so that the likeliest branch is executed without performing any jmp instruction (which has the bad effect of flushing the processor pipeline)
#ifdef __GNUC__
    #define likely(x)       __builtin_expect(!!(x), 1)
    #define unlikely(x)     __builtin_expect(!!(x), 0)
#else
    #define likely(x)       (x)
    #define unlikely(x)     (x)
#endif

BOOST_AUTO_TEST_SUITE_END() // MacroTestSuite
