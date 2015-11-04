//--------------------------------------------------------------------------------
// (C) Copyright 2014-2015 Stephane Molina, All rights reserved.
// See https://github.com/Dllieu for updates, documentation, and revision history.
//--------------------------------------------------------------------------------
#ifndef __GENERICS_TUPLEPRINTER_H__
#define __GENERICS_TUPLEPRINTER_H__

#include "TypeTraits.h"

namespace generics
{
    using singleParameterPack = std::true_type;
    using notSingleParameterPack =  std::false_type;

    template < typename C, typename T, size_t N, typename... Ts >
    void aux_put( std::basic_ostream< C, T >& os, const std::tuple< Ts... >& t, char delimiter, singleParameterPack )
    {
        os << std::get< N >( t );
    }

    template < typename C, typename T, size_t N, typename... Ts >
    void aux_put( std::basic_ostream< C, T >& os, const std::tuple< Ts... >& t, char delimiter, notSingleParameterPack )
    {
        os << std::get< N >( t ) << delimiter;

        aux_put< C, T, N + 1 >( os, t, delimiter, isLastParameterPack< N, Ts... >() );
    }

    using lastParameterPack = std::true_type;
    using notLastParameterPack = std::false_type;

    template < typename C, typename T, typename... Ts >
    void put( std::basic_ostream< C, T >& os, const std::tuple< Ts... >& t, char delimiter, lastParameterPack )
    {
        os << std::get< 0 >( t );
    }

    template < typename C, typename T, typename... Ts >
    void put( std::basic_ostream< C, T >& os, const std::tuple< Ts... >& t, char delimiter, notLastParameterPack )
    {
        aux_put< C, T, 0 >( os, t, delimiter, std::false_type() );
    }

    template < typename C, typename T, typename... Ts >
    void printTuple( std::basic_ostream< C, T >& os, const std::tuple< Ts... >& t, char delimiter = ' ' )
    {
        put( os, t, delimiter, isSingleParameterPack< Ts... >() );
    }
}

namespace std
{
    /*
    * @brief Old way of doing so with tag dispatching
    */
    /*template < typename C, typename T, typename... Ts >
    std::basic_ostream< C, T >&    operator<<( std::basic_ostream< C, T >& os, const tuple< Ts... >& t )
    {
        generics::printTuple( os, t );
        return os;
    }*/

    template <typename C, typename T, typename Tuple, std::size_t... Is>
    void    print_tuple_impl( basic_ostream< C, T >& os, const Tuple& t, std::index_sequence< Is... > )
    {
        using swallow = int[];
        // first 0 in case empty sequence
        // (expr1, expr2) -> expr2
        (void)swallow{0, ( void( os << ( Is == 0 ? "" : ", " ) << std::get<Is>( t ) ), 0 )...};
    }

    template < typename C, typename T, typename... Ts >
    basic_ostream< C, T >&    operator<<( basic_ostream< C, T >& os, const tuple< Ts... >& t )
    {
        print_tuple_impl( os, t, std::index_sequence_for< Ts... >() );
        return os;
    }
}

#endif /* ! __GENERICS_TUPLEPRINTER_H__ */
