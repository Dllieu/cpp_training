//--------------------------------------------------------------------------------
// (C) Copyright 2014-2015 Stephane Molina, All rights reserved.
// See https://github.com/Dllieu for updates, documentation, and revision history.
//--------------------------------------------------------------------------------
#ifndef __GENERICS_TYPETRAITS_H__
#define __GENERICS_TYPETRAITS_H__

#include <type_traits>

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

    // http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2014/n3911
    template < typename... >
    using void_t = void;
}

#endif // ! __GENERICS_TYPETRAITS_H__
