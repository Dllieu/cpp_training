//--------------------------------------------------------------------------------
// (C) Copyright 2014-2015 Stephane Molina, All rights reserved.
// See https://github.com/Dllieu for updates, documentation, and revision history.
//--------------------------------------------------------------------------------
#include <boost/test/unit_test.hpp>
#include <boost/type_index.hpp>
#include <type_traits>

#include <boost/utility/string_ref.hpp>

#include <iostream>
#include <array>
#include "generic/TypeTraits.h"

BOOST_AUTO_TEST_SUITE( TypeTraitsTestSuite )

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

        // making it explicit but no need to std::move (static_cast< T&& >( param ) which is already the case in that branch)
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
    // User defined literal can only accept these types
    //
    // const char*
    // unsigned long long int
    // long double
    // char
    // wchar_t
    // char16_t
    // char32_t
    // const char*, std::size_t
    // const wchar_t*, std::size_t
    // const char16_t*, std::size_t
    // const char32_t*, std::size_t
    constexpr bool operator""   _w( long double i ) { return false; }
    constexpr bool operator""   _w( const char* ) { return true; }
    constexpr size_t operator"" _w( const char*, size_t t ) { return t; }

    constexpr unsigned long long int operator"" _powSquare( unsigned long long int i ) { return i * i; }

    constexpr size_t operator"" _KB( unsigned long long size ) { return static_cast<size_t>( size * 1024 ); }
}

BOOST_AUTO_TEST_CASE( UserDefinedLiteralTest )
{
    static_assert_is_same< decltype( 1 ), int >();
    static_assert_is_same< decltype( 1UL ), unsigned long >();
    static_assert_is_same< decltype( 1LL ), long long >();

    static_assert_is_same< decltype( 'c' ), char >();
    static_assert_is_same< std::decay_t< decltype( u'c' ) >, char16_t >();
    static_assert_is_same< std::decay_t< decltype( U'c' ) >, char32_t >();
    static_assert_is_same< decltype( L'c' ), wchar_t >();

    static_assert( !45.24_w,            "Bad call" );
    static_assert( 321_w,               "Bad call" ); // call operator"" _w("321");
    static_assert( "wqeqw"_w == 5,      "Bad call" );

    static_assert( 2_powSquare == 4,    "Bad call" );

    static_assert( 4_KB == 4096,        "Bad call" );
}


namespace
{
    // TODO : do a loop when constraints are relaxed for constexpr
    constexpr size_t operator"" _ES( const char* s, size_t l )
    {
        //static_assert( l == 4, "Incorrect size" ); // Can't use function parameters in constant expression (not a constant input unlike if it was a template parameter)

        // sizeof( char ) * 4 hold in size_t
        return l == 4 ? s[ 0 ] << 24 | s[ 1 ] << 16 | s[ 2 ] << 8 | s[ 3 ] // << more 'precedence' than |
                      : throw std::logic_error( "l != 4" ); // Calling the function in a context that requires a constant expression will fail to compile
    }

    constexpr std::array< char, 4 > to_array( size_t v )
    {
        return std::array< char, 4 >{ static_cast< char >( ( v >> 24 ) & 0xFF ),
                                      static_cast< char >( ( v >> 16 ) & 0xFF ),
                                      static_cast< char >( ( v >> 8 ) & 0xFF ),
                                      static_cast< char >( v & 0xFF ) };
    }

    enum class Color : size_t
    {
        Black  = "BLCK"_ES,
        Green  = "GREN"_ES,
        Blue   = "BLUE"_ES,
        Pink   = "PINK"_ES,
    };
}

BOOST_AUTO_TEST_CASE( EnumUserDefinedTest )
{
    constexpr auto p = Color::Pink;
    constexpr size_t realValue = "PINK"_ES;

    //static_assert( realValue == p, "" ); // error: no operator== between size_t and Color
    static_assert( realValue == generics::enum_cast( p ), "" );

    auto arrayValue = to_array( generics::enum_cast( p ) );
    // TODO : use constexpr string_view when available and use static_assert instead
    BOOST_CHECK( std::string( arrayValue.data(), arrayValue.size() ) == "PINK" );
}

BOOST_AUTO_TEST_CASE( IsAnyTest )
{
    static_assert( generics::is_any< int, bool, double, int >::value, "" );
    static_assert( ! generics::is_any< int, bool, double, std::string >::value, "" );
    BOOST_CHECK( true );
}

namespace
{
    // VERSION TEST
    template < int MAJOR, int MINOR, int BUILD >
    struct CompileTimeVersion
    {
        using major = std::integral_constant< int, MAJOR >;
        using minor = std::integral_constant< int, MINOR >;
        using build = std::integral_constant< int, BUILD >;

        using type = std::tuple< major, minor, build >;
    };

    #define VERSION_OFFICIAL 3,1,4
    static_assert( CompileTimeVersion< VERSION_OFFICIAL >::major::value == 3, "" );

    #define CHECK_VERSION( VERSIONTOCHECK1, VERSIONTOCHECK2 ) \
        static_assert( std::is_same< CompileTimeVersion< VERSIONTOCHECK1 >::type, \
                                     CompileTimeVersion< VERSIONTOCHECK2 >::type >::value, "version mismatch" );
    
    #define VERSION_SAME 3,1,4
    CHECK_VERSION( VERSION_OFFICIAL, VERSION_SAME ); // OK

    #define VERSION_MISMATCH1 3,1,5
    //CHECK_VERSION( VERSION_OFFICIAL, VERSION_MISMATCH1 ); // KO: version mismatch

    #define VERSION_MISMATCH2 7,1,0
    //CHECK_VERSION( VERSION_OFFICIAL, VERSION_MISMATCH2 ); // KO: version mismatch
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
    // Old way of doing it
    template <typename T>
    auto has_stream_insertion_helper( ... ) // ... to disambiguate call
        -> std::false_type;
    
    template <typename T>
    auto has_stream_insertion_helper( int ) // int to disambiguate call
        -> decltype( std::declval<std::ostream&>() << std::declval<T>(), std::true_type{} );
    
    template <typename T>
    using has_stream_insertion = decltype( has_stream_insertion_helper<T>( 0 ) );

    // Not working well with VS2015 Update 1
    //template <typename T, typename = void>
    //struct stream_insertion_exists : std::false_type {};
    //
    //template <typename T>
    //struct stream_insertion_exists<T, std::void_t< decltype( std::declval<std::ostream&>() << std::declval<T&>() ) > > : std::true_type
    //{};

    class ClassNoStream {};
    class ClassHasStream {};

    std::ostream&   operator<<( std::ostream& os, const ClassHasStream& );
}

BOOST_AUTO_TEST_CASE( HasStreamInsertionTest )
{
    static_assert( has_stream_insertion< int >::value, "" );
    static_assert( !has_stream_insertion< ClassNoStream >::value, "" );
    static_assert( has_stream_insertion< ClassHasStream >::value, "" );

    BOOST_CHECK( true );
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
    class Minute {};

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
            : year_( std::get< const Year& >( std::tie( t1, t2, t3 ) ) )
            , month_( std::get< const Month& >( std::tie( t1, t2, t3 ) ) )
            , day_( std::get< const Day& >( std::tie( t1, t2, t3 ) ) )
        {}

        // otherway with default value
        template < typename... Ts >
        Date( const Ts&... ts )
            : year_( get_or_default< Year >( std::tie( ts... ) ) )
            , month_( get_or_default< Month >( std::tie( ts... ) ) )
            , day_( get_or_default< Day >( std::tie( ts... ) ) )
        {}

    private:
        Year    year_;
        Month   month_;
        Day     day_;
    };
}

BOOST_AUTO_TEST_CASE( PermutationOverloadTest )
{
    static_assert( std::is_constructible< Date, Year, Month, Day >::value, "Trivial Constructor KO" );
    static_assert( std::is_constructible< Date, Day, Month, Year >::value, "Constructor unordered arguments KO" );

    static_assert( std::is_constructible< Date, Minute/**/, Month, Year, Day >::value, "Constructor unordered arguments with extra value KO" );
    static_assert( std::is_constructible< Date, Minute/**/, Day >::value, "Constructor unordered arguments with extra value default value KO" );

    BOOST_CHECK( true );
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

namespace
{
    template <typename... Ts>
    void    sequence_with_tuple( const std::tuple<Ts...>& t)
    {
        static_assert_is_same< std::make_index_sequence< sizeof...(Ts) >(), std::index_sequence_for< Ts... >() >();
    }

    /*
    * @brief Old way to generate sequence
    */
    template <std::size_t... Is>
    struct sequence
    {
        using type = sequence<Is...>; //!< For test
    };

    template <std::size_t N, std::size_t... Is>
    struct generate_sequence : generate_sequence< N - 1, N - 1, Is... > {};

    // generate<0 ... N - 1>
    template <std::size_t... Is>
    struct generate_sequence<0, Is...> : sequence< Is... > {};
}

BOOST_AUTO_TEST_CASE( IndexSequenceTest )
{
    sequence_with_tuple(std::tuple< int, double, double >{});

    static_assert_is_same< typename sequence<0, 1, 2, 3>, generate_sequence<4>::type >();
    BOOST_CHECK( true );
}

BOOST_AUTO_TEST_SUITE_END() // TypeTraitsTestSuite
