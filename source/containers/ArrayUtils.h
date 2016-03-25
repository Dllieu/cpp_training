//--------------------------------------------------------------------------------
// (C) Copyright 2014-2015 Stephane Molina, All rights reserved.
// See https://github.com/Dllieu for updates, documentation, and revision history.
//--------------------------------------------------------------------------------
#pragma once

#include <array>
#include <utility>

// TODO: Remove when C++17
namespace std
{
    template < typename T, typename... Tail, typename Elem = typename std::decay< T >::type >
    std::array< Elem, 1 + sizeof...( Tail ) >   make_array( T&& head, Tail&&... values )
    {
        return { std::forward< T >( head ), std::forward< Tail >( values )... };
    }
}
