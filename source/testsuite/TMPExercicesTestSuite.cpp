//--------------------------------------------------------------------------------
// (C) Copyright 2014-2015 Stephane Molina, All rights reserved.
// See https://github.com/Dllieu for updates, documentation, and revision history.
//--------------------------------------------------------------------------------
// http://ultra.sdk.free.fr/misc/docs/Addison.Wesley.C++.Template.Metaprogramming.Concepts.Tools.and.Techniques.from.Boost.and.Beyond.pdf
// From Addison.Wesley.C++.Template.Metaprogramming.Concepts.Tools.and.Techniques.from.Boost.and.Beyond.pdf

#include <boost/test/unit_test.hpp>
#include <boost/type_traits.hpp>
#include <boost/mpl/if.hpp>
#include <iostream>

#include "generic/Typetraits.h"

BOOST_AUTO_TEST_SUITE( TMPTestSuite )

BOOST_AUTO_TEST_CASE( CommonTypeTest )
{
    // TODO: common_type_t
}

namespace
{
    template< bool /*is_reference*/ >
    struct add_const_ref_ {};

    template<>
    struct add_const_ref_< true >
    {
        template< typename T >
        struct deduce_type { using type = T; };
    };

    template<>
    struct add_const_ref_< false >
    {
        template< typename T >
        struct deduce_type { using type = const T&; };
    };

    template< typename T >
    struct add_const_ref
    {
        using type = typename add_const_ref_< boost::is_reference< T >::type::value >::template deduce_type< T >::type;
        using type2 = typename boost::mpl::if_< typename boost::is_reference< T >,
                                                typename T,
                                                typename boost::add_reference< typename boost::add_const< T >::type >::type
                                              >::type;
    };
}

// returns T if it is a reference type, and otherwise returns T const&
BOOST_AUTO_TEST_CASE( AddConstRefTest )
{
    static_assert( ( ! std::is_same< int, add_const_ref< int >::type >::value ), "" );
    static_assert( ( std::is_same< const int&, add_const_ref< int >::type >::value ), "" );
    static_assert( ( std::is_same< const int&, add_const_ref< int >::type2 >::value ), "" );
    static_assert( ( std::is_same< int&, add_const_ref< int& >::type2 >::value ), "" );

    BOOST_CHECK( true );
}

namespace
{
    // case not handled (too many template specialization otherwise)
    //     - const Type
    //     - function with more than 1 argument

    template< typename T, typename X /* to replace */, typename Y /* replace with */ >
    struct replace_type { using type = T; };

    template< typename X, typename Y >
    struct replace_type< X, X, Y > { using type = Y; };

    template< typename X, typename Y >
    struct replace_type< X&, X, Y > { using type = Y&; };

    template< typename X, typename Y >
    struct replace_type< X*, X, Y > { using type = Y*; };

    template< typename T, typename X, typename Y, int N >
    struct replace_type< T[N], X, Y >
    {
        using atomicType = typename replace_type< T, X, Y >::type;
        using type = atomicType[N];
    };

    template < typename T, typename X, typename Y >
    struct replace_type< T (*) (), X, Y >
    {
        using returnType = typename replace_type< T, X, Y >::type;
        using type = returnType (*) ();
    };
 
    template< typename TR, typename T1, typename X, typename Y >
    struct replace_type< TR (*) (T1), X, Y >
    {
        using returnType = typename replace_type< TR, X, Y >::type;
        using argumentType = typename replace_type< T1, X, Y >::type;
 
        using type = returnType (*) (argumentType);
    };
}

// metafunction replace_type< T, X, Y > takes an arbitrary type T as its first parameter, and replaces all occurrences of a type X within T with Y:
BOOST_AUTO_TEST_CASE( ReplaceTypeTest )
{
    static_assert( ( std::is_same< int, replace_type< void, void, int >::type >::value ), "" );
    static_assert( ( std::is_same< int, replace_type< int, double, float >::type >::value ), "" );
    static_assert( ( std::is_same< int*, replace_type< void*, void, int >::type >::value ), "" );
    static_assert( ( std::is_same< int&, replace_type< double&, double, int >::type >::value ), "" );
    static_assert( ( std::is_same< int[5], replace_type< double[5], double, int >::type >::value ), "" );
    static_assert( ( std::is_same< int (*) (), replace_type< double (*) (), double, int >::type >::value ), "" );
    static_assert( ( std::is_same< int* (*) ( float ), replace_type< double* (*) ( float ), double, int >::type >::value ), "" );
    static_assert( ( std::is_same< int (*) ( int& ), replace_type< double (*) ( double& ), double, int >::type >::value ), "" );

    BOOST_CHECK( true );
}

namespace
{
    template < typename Target, typename Source >
    inline Target  polymorphic_downcast( Source* s )
    {
        assert( dynamic_cast< Target >( s ) == s );
        return static_cast< Target >( s );
    }

    template < typename Target, typename Source >
    inline Target  polymorphic_downcast( Source& s )
    {
        using targetNoRef = typename boost::remove_reference< Target >::type;
        assert( dynamic_cast< targetNoRef* >( &s ) == &s );
        return static_cast< Target >( s );
    }

    struct Base { virtual ~Base() {} /* dynamic_cast require one virtual to be considered as polymorphic */ };
    struct Derived : public Base {};
}

BOOST_AUTO_TEST_CASE( PolymorphicDowncastTest )
{
    Base b;
    Derived d;

    auto testDowncastPtr = [] ( Base* p )
    {
        // might assert
        (Derived*) polymorphic_downcast< Derived* >( p );
        return true;
    };
    auto testDowncastRef = [] ( Base& p )
    {
        // might assert
        (Derived&) polymorphic_downcast< Derived& >( p );
        return true;
    };

    // testDowncastPtr( &b ); // assert
    BOOST_CHECK( testDowncastPtr( &d ) );

    // testDowncastRef( b ); // assert
    BOOST_CHECK( testDowncastRef( d ) );
}

namespace
{
    template < typename T >
    struct Field
    {
        typename T::value_type storage;
        typename T::value_type& operator[]( const T &c )
        {
            return storage;
        }
    };

    template < typename... Ts >
    struct list_type_impl {};

    template < typename T, typename T1, typename... Ts >
    struct list_type_impl< T, T1, Ts... > : T, list_type_impl< T1, Ts... >
    {
        static_assert( !generics::is_any< T, T1, Ts... >::value, "list_type: type duplication" );

        using T::operator[];
        using list_type_impl< T1, Ts... >::operator[];
    };

    template < typename T >
    struct list_type_impl< T >: T
    {
        using T::operator[];
    };

    template < typename T, typename... Ts >
    struct list_type : public list_type_impl< Field< T >, Field < Ts >... >
    {
        using list_type_impl< Field< T >, Field < Ts >... >::operator[];
    };
}

BOOST_AUTO_TEST_CASE( ListTypeTest )
{
    struct int_wrapper { using value_type = int; };
    struct string_wrapper { using value_type = std::string; };
    struct another_int_wrapper { using value_type = int; };

    list_type< int_wrapper, string_wrapper, another_int_wrapper > l;

    l[ int_wrapper() ] = 4;
    l[ another_int_wrapper() ] = 5;

    BOOST_CHECK( l[ int_wrapper() ] == 4 );
    BOOST_CHECK( l[ another_int_wrapper() ] == 5 );
}

namespace
{
    struct tag_list {};
    struct tag_string {};
    struct tag_arithmetic {};
    struct tag_enum {};
    struct tag_single {};

    template < typename T, size_t LENGTH >
    using derivated_tag = std::conditional_t< LENGTH == 1, std::conditional_t< std::is_enum< T >::value, tag_enum,
                                                                               std::conditional_t< std::is_arithmetic< T >::value, tag_arithmetic,
                                                                                                                                   tag_single > >,
                                                           std::conditional_t< std::is_same< T, char >::value, tag_string,
                                                                                                               tag_list >>;

    template < typename T, size_t LENGTH = 1, typename TAG = derivated_tag< T, LENGTH > >
    struct Prototype;
    
    template < typename T, size_t LENGTH >
    struct Prototype< T, LENGTH, tag_list > { void list() const {} };

    template < typename T, size_t LENGTH >
    struct Prototype< T, LENGTH, tag_arithmetic > { void arithmetic() const {} };

    template < typename T, size_t LENGTH >
    struct Prototype< T, LENGTH, tag_string > { void string() const {} };

    template < typename T, size_t LENGTH >
    struct Prototype< T, LENGTH, tag_single > { void single() const {} };

    template < typename T, size_t LENGTH >
    struct Prototype< T, LENGTH, tag_enum > { void enumCase() const {} };

    struct Foo {};
    enum class FooEnum { Case1, Case2 };
}

BOOST_AUTO_TEST_CASE( TagDispatchTest )
{
    Prototype< int, 2 > l;      l.list();
    Prototype< int > a;         a.arithmetic();
    Prototype< char > a2;       a2.arithmetic();
    Prototype< char, 10 > s;    s.string();
    Prototype< Foo > single;    single.single();
    Prototype< FooEnum > e;     e.enumCase();

    BOOST_CHECK( true );
}

namespace
{
    template < bool LightFeed >
    struct FeedData
    {
        FeedData() : value( 0 ) {}

        void    doStuff() const { generics::static_if< !LightFeed >( [] { std::cout << "!LightFeed infos..." << std::endl; } ); }
        void    generateData() { generics::static_if< LightFeed >( [ & ] { value = 1; }, [ & ] { value = 2; } ); }
        void    doOtherStuff() { value *= 2; }

        int     value;
    };

    // Scenario that could be useful: only call methods of IHandler not often, but call FeedData many times within IHandler (to limit the cost of the virtuality)
    struct IHandler
    {
        virtual ~IHandler() = default;
        virtual int  processFeed() = 0;
    };

    template < bool LightFeed >
    struct Handler final : public IHandler
    {
        virtual ~Handler() override final = default;
        virtual int processFeed() override final
        {
            FeedData< LightFeed > feed;
            feed.doStuff(); // only !LightFeed
            feed.generateData(); // different behavior depending of LightFeed
            feed.doOtherStuff(); // common case
            return feed.value;
        }
    };
}

BOOST_AUTO_TEST_CASE( StaticIfTest )
{
    auto isLightFeed = true; // value known at runtime (e.g. from the conf)
    
    std::unique_ptr< IHandler > handler;
    if ( isLightFeed )
        handler = std::make_unique< Handler< true > >();
    else
        handler = std::make_unique< Handler< false > >();

    BOOST_CHECK( handler->processFeed() == 2 );
}

namespace
{
    struct ProtocolV1 { int     get_version() const { return 1; } };
    struct ProtocolV2 { int     getValue() const { return 2; } };
    struct ProtocolV3 { int     version() const { return 3; } };

    template < typename P >
    struct GenericProtocol
    {
        // Uncomment (int = 0) once visual studio fix SFINAE
        template < typename T = P >
        auto requestVersion() const -> decltype( std::declval<T>().get_version() ) { return p_.get_version(); }

        template < typename T = P, int = 0 >
        auto requestVersion() const -> decltype( std::declval<T>().getValue() ) { return p_.getValue(); }

        template < typename T = P, int = 0, int = 0 >
        auto requestVersion() const -> decltype( std::declval<T>().version() ) { return p_.version(); }

        int    getVersion() const { return requestVersion(); }

    private:
        P   p_;
    };
}

BOOST_AUTO_TEST_CASE( GenericInterfaceTest )
{
    GenericProtocol< ProtocolV1 > v1;
    GenericProtocol< ProtocolV2 > v2;
    GenericProtocol< ProtocolV3 > v3;

    BOOST_CHECK( v1.getVersion() == 1 );
    BOOST_CHECK( v2.getVersion() == 2 );
    BOOST_CHECK( v3.getVersion() == 3 );
}

namespace
{
    template < int v >
    struct Int2Type { enum { value = v }; };

    // Avoid dynamic branching (couldn't compile for every path), avoid template specialization
    template < typename T, bool shouldClone >
    class NiftyContainer
    {
    private:
        T*  create( T* pObj, Int2Type< true > ) { return pObj->Clone(); }
        T*  create( T* pObj, Int2Type< false> ) { return new T( *pObj ); }

    public:
        std::unique_ptr< T >  create( T* pObj ) { return std::unique_ptr< T >( create( pObj, Int2Type< shouldClone >() ) ); }
    };
}

BOOST_AUTO_TEST_CASE( Int2TypeTest )
{
    auto i = 2;
    auto iBis( NiftyContainer< decltype( i ), false >().create( &i ) );

    BOOST_CHECK( *iBis == i );
}

BOOST_AUTO_TEST_CASE( ConcatStringTest )
{
    constexpr const char s1[] = "s1";
    constexpr const char s2[] = "s5";
    constexpr auto result = generics::concatenate( s1, s2 );

    BOOST_CHECK( result.data() == ( std::string( s1 ) + s2 ) );
}

namespace
{
    template<typename U, typename ... T>
    bool is_one_of(U&& u, T && ... t)
    {
        auto match = false;
        (void)std::initializer_list<bool>{ (match = match || u == t)... };
        return match;
    }
}

BOOST_AUTO_TEST_CASE( IsOneOfTest )
{
    const int n = 8;
    
    BOOST_CHECK( is_one_of( n, 4, 1, 8, 3 ) );
    BOOST_CHECK( ! is_one_of( n, 4, 1, 3 ) );
}

BOOST_AUTO_TEST_SUITE_END() // TMPTestSuite
