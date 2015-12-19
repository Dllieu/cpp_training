//--------------------------------------------------------------------------------
// (C) Copyright 2014-2015 Stephane Molina, All rights reserved.
// See https://github.com/Dllieu for updates, documentation, and revision history.
//--------------------------------------------------------------------------------
#pragma once

namespace tools
{
    constexpr auto operator""   _KB( size_t s ) { return s * 1024; }
    constexpr auto operator""   _MB( size_t s ) { return s * 1024 * 1000; }

    // Max number of segment in L1 = 32KB / 64 = 512
    enum class CacheSize
    {
        L1 = 32_KB,
        L2 = 256_KB,
        L3 = 6_MB,
        DRAM
    };

    template < typename T >
    CacheSize   byteToAppropriateCacheSize( size_t numberElements )
    {
        auto byteSize = numberElements * sizeof( T );
        if ( byteSize < generics::enum_cast( CacheSize::L1 ) )
            return CacheSize::L1;

        if ( byteSize < generics::enum_cast( CacheSize::L2 ) )
            return CacheSize::L2;

        if ( byteSize < generics::enum_cast( CacheSize::L3 ) )
            return CacheSize::L3;

        return CacheSize::DRAM;
    }

    const char* to_string( CacheSize cacheSize );

    template < typename T >
    void    display_information( size_t n )
    {
        auto byteNumber = n * sizeof( T );
        std::cout << "(CL=" << std::ceil( byteNumber / 64 );
        std::cout << "|SN=" << std::ceil( byteNumber / 1024. ) << "KB[" << to_string( byteToAppropriateCacheSize< T >( n ) ) << "]);" << n << ";";
    }
}
