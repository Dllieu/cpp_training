//--------------------------------------------------------------------------------
// (C) Copyright 2014-2015 Stephane Molina, All rights reserved.
// See https://github.com/Dllieu for updates, documentation, and revision history.
//--------------------------------------------------------------------------------
#pragma once

#include <array>
#include <algorithm>
#include <chrono>
#include <numeric>
#include <tuple>
#include <iostream>

#include "CacheInformation.h"
#include "generic/TuplePrinter.h"

namespace tools
{
    static constexpr const int                        NumberTrials = 20;
    static constexpr const std::chrono::milliseconds  MinTimePerTrial( 200 );

    template < typename F >
    auto    benchmark_impl( size_t n, F&& f )
    {
        volatile decltype( f() ) res; // to avoid optimizing f() away

        std::array< double, NumberTrials > trials;
        for ( auto i = 0; i < NumberTrials; ++i )
        {
            auto runs = 0;

            std::chrono::high_resolution_clock::time_point now;
            auto startTimer = std::chrono::high_resolution_clock::now();
            do
            {
                res = f();
                ++runs;
                now = std::chrono::high_resolution_clock::now();
            } while ( now - startTimer < MinTimePerTrial );
            trials[ i ] = std::chrono::duration_cast< std::chrono::duration< double > >( now - startTimer ).count() / runs;
        }
        static_cast< void >( res );

        std::sort( trials.begin(), trials.end() );
        return std::accumulate( trials.begin() + 2, trials.end() - 2, 0.0 ) / ( trials.size() - 4 ) * 1E6 / n;
    }

    template < typename... Fs >
    auto    benchmark( size_t n, Fs&&... fs )
    {
        // std::make_tuple reverse the call oder (VS2015 only?)
        auto result = std::make_tuple( benchmark_impl( n, std::forward< Fs >( fs ) )... );
        generics::printTuple( std::cout, result, ';' );
        std::cout << std::endl;
        return result;
    }

    template < typename ELEMENT_TYPE, typename F, typename... Ns >
    void    run_test( const std::string& header, F&& f, Ns... range )
    {
        std::cout << "infos;n;" << header << std::endl;
        for ( auto n : { range... } )
        {
            display_information< ELEMENT_TYPE >( n );
            f( n );
        }
    }
}
