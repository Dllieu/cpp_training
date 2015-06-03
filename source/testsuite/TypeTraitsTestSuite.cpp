//--------------------------------------------------------------------------------
// (C) Copyright 2014-2015 Stephane Molina, All rights reserved.
// See https://github.com/Dllieu for updates, documentation, and revision history.
//--------------------------------------------------------------------------------
#include <boost/test/unit_test.hpp>
#include <boost/type_index.hpp>
#include <type_traits>

#include <iostream>
#include "generic/TypeTraits.h"

BOOST_AUTO_TEST_SUITE( TypeTraits )

namespace
{
    template < typename A, typename B >
    void static_assert_is_same()
    {
        static_assert( std::is_same< A, B >::value, "type mismatch" );
        BOOST_CHECK( true );
    };

    // T&& is a universal , it can either mean lvalue (&) or rvalue (&&)
    template <typename T>
    T&& forward( T&& param )
    {
        if ( std::is_lvalue_reference< T >::value )
            return param; // if lvalue, return lvalue ( T& or const T& )
        return std::move( param );
    }
}

BOOST_AUTO_TEST_CASE( DecayTest )
{
    static_assert_is_same< int, std::decay_t<int&> >();
    static_assert_is_same< int, std::decay_t<int&&> >();
    static_assert_is_same< int, std::decay_t<const int&> >();
    static_assert_is_same< int*, std::decay_t<int[ 2 ]> >();
    static_assert_is_same< int( *)( int ), std::decay_t<int( int )>>();

    int i = 5;
    static_assert_is_same< decltype( forward( i ) ), int& >();
    static_assert_is_same< decltype( forward( 5 ) ), int&& >();
}

namespace
{
    class A {};
    class B { virtual ~B() {} };
}

BOOST_AUTO_TEST_CASE( HasVirtualDestructorTest )
{
    BOOST_CHECK( ! std::has_virtual_destructor< A >::value );
    BOOST_CHECK( std::has_virtual_destructor< B >::value );
}

namespace
{
    class RealClass
    {
    public:
        int     realA() { return 0; }
        double  realB() { return 1; }
    };

    class ForwardClass
    {
    public:
        int         forwardA();
        double      forwardB();

    private:
        RealClass   realClass_;
    };
}

#define IMPLEMFORWARDCLASS( methodName, methodFromRealClassToBeForwarded ) \
    std::decay_t< std::result_of_t< decltype ( &RealClass::##methodFromRealClassToBeForwarded )( RealClass ) > > ForwardClass::##methodName() \
    { \
        return realClass_.##methodFromRealClassToBeForwarded(); \
    }

IMPLEMFORWARDCLASS( forwardA, realA );
IMPLEMFORWARDCLASS( forwardB, realB );

#undef IMPLEMFORWARDCLASS

BOOST_AUTO_TEST_CASE( TraitsTest )
{
    ForwardClass forwardClass;
    RealClass realClass;

    BOOST_CHECK( forwardClass.forwardA() == realClass.realA() );
    BOOST_CHECK( forwardClass.forwardB() == realClass.realB() );
}

BOOST_AUTO_TEST_CASE( VariableTypeTest )
{
    ForwardClass forwardClass;
    (void)forwardClass; // unreferenced local variable
    // class ::anonymous::ForwardClass
    BOOST_CHECK( ! boost::typeindex::type_id_with_cvr< decltype( forwardClass ) >().pretty_name().empty() );
}

namespace
{
    template < typename T >
    std::enable_if_t< std::is_pointer< T >::value >     callWithPointer( T, bool& isPointer ) { isPointer = true; }

    template < typename T, std::size_t sz >
    void callWithPointer( T(&)[sz], bool& isPointer ) { isPointer = false; }
}

BOOST_AUTO_TEST_CASE( ArrayParameterTest )
{
    bool isPointer = false;
    int i[5];

    callWithPointer( &i, isPointer );
    BOOST_CHECK( isPointer );

    callWithPointer( i, isPointer );
    BOOST_CHECK( !isPointer );
}

namespace
{
    template <typename T>
    auto has_to_string_helper( ... ) //... to disambiguate call
        -> std::false_type;

    //true case, to_string valid for T
    template <typename T>
    auto has_to_string_helper( int ) //int to disambiguate call
        -> decltype( std::to_string( std::declval<T>() ), std::true_type{} );

    //alias to make it nice to use
    template <typename T>
    using has_to_string = decltype( has_to_string_helper<T>( 0 ) );
}

BOOST_AUTO_TEST_CASE( HasToStringTest )
{
    BOOST_CHECK( has_to_string< int >::value );
    BOOST_CHECK( ! has_to_string< std::vector< int > >::value );
}

namespace
{
    // don't work with vc120
    /*template <typename...>
    using void_t = void;

    template <typename T, typename = void>
    struct stream_insertion_exists : std::false_type {};

    template <typename T>
    struct stream_insertion_exists<T, void_t<
        decltype( std::declval<std::ostream&>() << std::declval<T>() )
    > > : std::true_type{};*/

    template <typename T>
    auto has_stream_insertion_helper( ... ) // ... to disambiguate call
        -> std::false_type;

    template <typename T>
    auto has_stream_insertion_helper( int ) // int to disambiguate call
        -> decltype( std::declval<std::ostream&>() << std::declval<T>(), std::true_type{} );

    template <typename T>
    using has_stream_insertion = decltype( has_stream_insertion_helper<T>( 0 ) );

    class ClassNoStream {};
    class ClassHasStream {};
    std::ostream&   operator<<( std::ostream&, const ClassHasStream& );
}

BOOST_AUTO_TEST_CASE( HasStreamInsertionTest )
{
    BOOST_CHECK( has_stream_insertion< int >::value );
    BOOST_CHECK( ! has_stream_insertion< ClassNoStream >::value );
    BOOST_CHECK( has_stream_insertion< ClassHasStream >::value );
}

namespace
{
    namespace
    {
        template <typename T, typename... Ts> struct has_T;

        template <typename T> struct has_T<T> : std::false_type{};

        template <typename T, typename... Ts> struct has_T<T, T, Ts...>
        : std::true_type{};

        template <typename T, typename Tail, typename... Ts>
        struct has_T<T, Tail, Ts...> : has_T<T, Ts...>{};

        template <typename T, typename... Ts>
        const T& get_or_default_impl( std::true_type, const std::tuple<Ts...>& t, const T& )
        {
            return std::get<T>( t );
        }

        template <typename T, typename... Ts>
        const T& get_or_default_impl( std::false_type, const std::tuple<Ts...>&, const T& default_value )
        {
            return default_value;
        }
    }

    template <typename T, typename... Ts>
    const T& get_or_default( const std::tuple<Ts...>& t, const T& default_value = T{} )
    {
        return get_or_default_impl<T>( has_T<T, Ts...>{}, t, default_value );
    }

    class Year {};
    class Month {};
    class Day {};

    class Date
    {
    public:
        Date( const Year& year, const Month& month, const Day& day )
            : year_( year ), month_( month ), day_( day )
        {}

        // check at compile time if all the required type exist
        // need c++14 (std::tie constexpr)
        template < typename T1, typename T2, typename T3 >
        Date( const T1& t1, const T2& t2, const T3& t3 )
            : year_( std::get< Year >( std::tie( t1, t2, t3 ) ) )
            , month_( std::get< Month >( std::tie( t1, t2, t3 ) ) )
            , day_( std::get< Day >( std::tie( t1, t2, t3 ) ) )
        {}

        // otherway with default value
        template < typename... Ts >
        Date( const Ts&... ts )
            : get_or_default< Year >( std::tie( ts... ) )
            , get_or_default< Month >( std::tie( ts... ) )
            , get_or_default< Day >( std::tie( ts... ) )
        {}

    private:
        Year    year_;
        Month   month_;
        Day     day_;
    };
}

BOOST_AUTO_TEST_CASE( PermutationOverloadTest )
{
    Year y;
    Month m;
    Day d;

    Date date( y, m, d );
    // uncomment when vs2015
    // Date date( m, d, y );
}

// http://pdimov.com/
namespace
{
    template < typename T >
    using mp_add_pointer = T*;

    template < typename... T >
    struct mp_list {};

    template < typename A, template < typename... > class B >
    struct mp_rename_impl;

    template < template < typename... > class A, typename... Args, template < typename... > class B >
    struct mp_rename_impl< A< Args... >, B >
    {
        using type = B< Args... >;
    };

    template < typename A, template < typename... > class B >
    using mp_rename = typename mp_rename_impl< A, B >::type;
}

BOOST_AUTO_TEST_CASE( MpRenameTest )
{
    using pInt = mp_add_pointer< int >;
    static_assert_is_same< pInt, int* >();

    using list = mp_list< int, bool, std::string >;
    static_assert_is_same< mp_rename< list, std::tuple >, std::tuple< int, bool, std::string > >();
}

namespace
{
    template < typename T >
    struct mp_size_impl;

    // std::integral_constant is a standard C++11 type that wraps an integral constant( that is, a compile - time constant integer value ) into a type.
    // Since metaprogramming operates on type lists, which can only hold types, it's convenient to represent compile-time constants as types.
    // This allows us to treat lists of types and lists of values in a uniform manner. It is therefore idiomatic in metaprogramming to take and return types instead of values, and this is what we have done.
    // If at some later point we want the actual value, we can use the expression mp_size<L>::value to retrieve it.
    template < typename... Ts > using mp_length = std::integral_constant< std::size_t, sizeof...( Ts ) >;

    template < typename T >
    using mp_size = mp_rename< T, mp_length >;

    template < template < typename... > class F, typename L >
    using mp_apply = mp_rename< L, F >;

    template < typename T >
    struct mp_test_impl;

    template < template < typename... > class L, typename... Ts >
    struct mp_test_impl< L< Ts... > >
    {
        using type = L<int>;
    };

    template < typename T >
    using mp_test = mp_test_impl< T >;
}

BOOST_AUTO_TEST_CASE( MpSizeTest )
{
    using list = mp_list< int, bool, std::string >;
    static_assert( mp_size< list >::value == 3, "" );
    static_assert( mp_apply< mp_length, list >::value == 3, "" );
}

namespace
{
    // transform L by applying F on all elements of L
    /*template < typename F, typename L >
    using mp_transform;*/

    template < typename L, typename... T >
    struct mp_push_back_impl;

    template < template < typename... > class L, typename... Args, typename... T >
    struct mp_push_back_impl< L< Args... >, T... >
    {
        using type = L< Args..., T... >;
    };

    template < typename L, typename... T >
    using mp_push_back = typename mp_push_back_impl< L, T... >::type;
}

BOOST_AUTO_TEST_CASE( MpPushBackTest )
{
    using list = mp_list< int >;
    static_assert_is_same< mp_list< int, double, void* >, mp_push_back< list, double, void* > >();
}

namespace
{
    template < template< typename... > class F, typename L >
    struct mp_transform_impl;

    template < template < typename... > class F, typename L >
    using mp_transform = typename mp_transform_impl< F, L >::type;

    template < template < typename... > class F, template < typename... > class L, typename... T >
    struct mp_transform_impl< F, L< T... > >
    {
        using type = L< F< T >... >;
    };

    //// Can't be deduced to this type for some reason, typename L always prefered over template < typename... > class L (when L empty)
    //template < template< typename... > class F, template < typename... > class L >
    //struct mp_transform_impl< F, L<> >
    //{
    //    using type = L<>;
    //};

    //template < template< typename... > class F, template < typename... > class L, typename Arg0, typename... Args >
    //struct mp_transform_impl< F, L< Arg0, Args... > >
    //{
    //    using first_ = F< Arg0 >;
    //    using rest_ = mp_transform< F, L< Args... > >;

    //    using type = mp_push_back< first_, rest_ >;
    //};
}

BOOST_AUTO_TEST_CASE( MpTransformTest )
{
    using list = mp_list< int, double >;
    static_assert_is_same< mp_transform< mp_add_pointer, list >, mp_list< int*, double* > >();
}

BOOST_AUTO_TEST_SUITE_END() // TypeTraits
