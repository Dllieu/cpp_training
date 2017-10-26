//--------------------------------------------------------------------------------
// (C) Copyright 2014-2015 Stephane Molina, All rights reserved.
// See https://github.com/Dllieu for updates, documentation, and revision history.
//--------------------------------------------------------------------------------
#include <boost/test/unit_test.hpp>
#include <array>
#include <cstdlib>
#include <emmintrin.h>

#include "tools/Timer.h"

// Intrinsics resemble assembly language except that they leave the actual register allocation, instruction scheduling, and addressing modes to the compiler.
// Except for explicit unaligned load and store, compiler assume that packed memory operands of instrinsics are properly aligned
BOOST_AUTO_TEST_SUITE( SIMDTestSuite )

namespace
{
    template < size_t SIZE >
    void    copy_with_offset( std::array< int, SIZE >& to, const std::array< int, SIZE >& from, int offset )
    {
        static_assert( SIZE % 16 == 0, "size not multiple of 16" );

        // loop is memory bound if array not in the cache because it does nothing else that is time-consuming
        std::transform( from.begin(), from.end(), to.begin(), [ &offset ]( auto n ){ return n + offset; } );
    }

    template < size_t SIZE >
    void    copy_with_offset_with_simd( std::array< int, SIZE >& to, const std::array< int, SIZE >& from, int offset )
    {
        static_assert( SIZE % 16 == 0, "size not multiple of 16" );

        auto to4 = reinterpret_cast< __m128i* >( to.data() );
        auto from4 = reinterpret_cast< const __m128i* >( from.data() );
        // Sets the 4 signed 32-bit integer values in reverse order
        auto offset4 = _mm_setr_epi32( offset, offset, offset, offset );

        // still memory bounded, additional improvement that could be done:
        //   - more work in the loop using the time being wasted for memory
        //   - reducing the amount of memory used which also reduced the number of cache misses
        //   - making sure the memory was in the cache (by using a strip-mining technique with another function that accessed the same memory)
        for ( int i = 0; i < SIZE / 4; ++i )
            // void _mm_stream_si128(__m128i *p, __m128i a) : Stores the data in a to the address p without polluting the caches.
            //                                                If the cache line containing address p is already in the cache, the cache will be updated. Address p must be 16 byte aligned.
            _mm_stream_si128( to4 + i, _mm_add_epi32( from4[i], offset4 ) /* Adds the 4 signed or unsigned 32-bit integers in from4[i] to the 4 signed or unsigned 32-bit integers in offset4 */);
    }
}

BOOST_AUTO_TEST_CASE( StreamSiBenchmark )
{
    constexpr size_t size = 16 * 100;
    constexpr int offset = 54;
    std::array< int, size > from;
    std::generate( from.begin(), from.end(), std::rand /*C rand*/ );

    std::array< int, size > to;
    std::array< int, size > toSimd;
    auto elapsedNaive = tools::Timer::elapsed( [ &to, &from, &offset ] { copy_with_offset( to, from, offset ); } );
    auto elapsedSimd = tools::Timer::elapsed( [ &toSimd, &from, &offset ] { copy_with_offset_with_simd( toSimd, from, offset ); } );

    BOOST_CHECK( to == toSimd );
    BOOST_CHECK( elapsedNaive >= elapsedSimd );
}

BOOST_AUTO_TEST_SUITE_END() // SIMDTestSuite
