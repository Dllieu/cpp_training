//--------------------------------------------------------------------------------
// (C) Copyright 2014-2015 Stephane Molina, All rights reserved.
// See https://github.com/Dllieu for updates, documentation, and revision history.
//--------------------------------------------------------------------------------
#pragma once

#include <tuple>

namespace generics
{
    template <typename T, typename F, std::size_t... Is>
    void    for_each_tuple_impl( const T& t, F&& f, const std::index_sequence< Is... >& )
    {
        using swallow = int[];
        // first 0 in case empty sequence
        // (expr1, expr2) -> expr2
        (void)swallow{ 0, (void(f( std::get< Is >( t ) )), 0)... };
    }

    template <typename... Ts, typename F>
    void    for_each_tuple( const std::tuple< Ts... >& tuple, F&& f )
    {
        for_each_tuple_impl( tuple, f, std::index_sequence_for< Ts... >{} );
    }
}
