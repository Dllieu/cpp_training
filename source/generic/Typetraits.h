//--------------------------------------------------------------------------------
// (C) Copyright 2014-2015 Stephane Molina, All rights reserved.
// See https://github.com/Dllieu for updates, documentation, and revision history.
//--------------------------------------------------------------------------------
#ifndef __GENERICS_TYPETRAITS_H__
#define __GENERICS_TYPETRAITS_H__

#include <type_traits>
#include <utility>
#include <array>

namespace generics
{
    namespace
    {
        template < typename ... Ts >
        struct __is_single_parameter_pack_helper
        {
            using type = typename std::conditional< 1 == sizeof...( Ts ), std::true_type, std::false_type >::type;
        };

        template< size_t N, typename ... Ts>
        struct __is_last_parameter_helper
        {
            using type = typename std::conditional< N + 1 == sizeof...( Ts ) - 1, std::true_type, std::false_type >::type;
        };
    }

    template < typename ... Ts >
    struct isSingleParameterPack :
        public std::integral_constant< bool, __is_single_parameter_pack_helper< Ts... >::type::value >
    {};

    template < size_t N, typename ... Ts >
    struct isLastParameterPack :
        public std::integral_constant< bool, __is_last_parameter_helper< N, Ts... >::type::value >
    {};

    template < typename T, typename... Ts >
    struct is_any : std::false_type
    {};

    template < typename T, typename First >
    struct is_any< T, First > : std::is_same<T, First>
    {};

    template < typename T, typename First, typename... Rest >
    struct is_any< T, First, Rest... > : std::integral_constant< bool, std::is_same< T, First >::value || is_any< T, Rest... >::value >
    {};

    template < typename ENUM_TYPE >
    constexpr auto enum_cast( ENUM_TYPE v )
    {
        return static_cast<std::underlying_type_t< ENUM_TYPE >>( v );
    }

    namespace
    {
        template <bool> struct __static_if_tag {};

        template < typename T, typename F > auto static_if( __static_if_tag<true>, T&& t, F&& f ) { return t(); }
        template < typename T, typename F > auto static_if( __static_if_tag<false>, T&& t, F&& f ) { return f(); }
    }

    template < bool B, typename T, typename F >
    auto    static_if( T&& t, F&& f ) { return static_if( __static_if_tag<B>{}, std::forward< T >( t ), std::forward< F >( f ) ); }

    template < bool B, typename T >
    auto    static_if( T&& t ) { return static_if( __static_if_tag<B>{}, std::forward< T >( t ), []( auto&&... ){} ); }

    template < typename F, typename G >
    auto compose( F&& f, G&& g )
    {
        return [ f, g ] ( auto&&... x ) { return f( g( std::forward< decltype( x ) >( x )... ) ); };
    }

    namespace
    {
        // Convenience
        template < typename F, typename G >
        auto    operator*( F&& f, G&& g )
        {
            return compose( std::forward< F >( f ), std::forward< G >( g ) );
        }
    }

    template < typename F, typename... Fs >
    auto    compose( F&& f, Fs&&... fs )
    {
        return std::forward< F >( f ) * compose( std::forward< Fs >( fs )... );
    }

    template < std::size_t N1, std::size_t N2, std::size_t... I1, std::size_t... I2 >
    constexpr std::array< const char, N1 + N2 - 1 > concatenate( const char( &a1 )[ N1 ], const char( &a2 )[ N2 ], std::index_sequence<I1...>, std::index_sequence<I2...> )
    {
        return{ { a1[ I1 ]..., a2[ I2 ]... } };
    }

    template < std::size_t N1, std::size_t N2 >
    constexpr auto concatenate( const char( &a1 )[ N1 ], const char( &a2 )[ N2 ] )
    {
        return concatenate( a1, a2, std::make_index_sequence< N1 - 1 >{}, std::make_index_sequence< N2 >{} );
    }
}

#endif // ! __GENERICS_TYPETRAITS_H__
