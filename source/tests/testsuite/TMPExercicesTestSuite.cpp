// http://ultra.sdk.free.fr/misc/docs/Addison.Wesley.C++.Template.Metaprogramming.Concepts.Tools.and.Techniques.from.Boost.and.Beyond.pdf
// From Addison.Wesley.C++.Template.Metaprogramming.Concepts.Tools.and.Techniques.from.Boost.and.Beyond.pdf

#include <boost/test/unit_test.hpp>
#include <boost/type_traits.hpp>
#include <boost/mpl/if.hpp>

#include <type_traits>

BOOST_AUTO_TEST_SUITE( TMPExercices )

namespace
{
    template< bool /*is_reference*/ >
    struct add_const_ref_ {};

    template<>
    struct add_const_ref_< true >
    {
        template< typename T >
        struct deduce_type { typedef T type; };
    };

    template<>
    struct add_const_ref_< false >
    {
        template< typename T >
        struct deduce_type { typedef const T& type; };
    };

    template< typename T >
    struct add_const_ref
    {
        typedef typename add_const_ref_< boost::is_reference< T >::type::value >::template deduce_type< T >::type type;
        typedef typename boost::mpl::if_< typename boost::is_reference< T >,
                                          typename T,
                                          typename boost::add_reference< typename boost::add_const< T >::type >::type
                                        >::type type2;
    };
}

// returns T if it is a reference type, and otherwise returns T const&
BOOST_AUTO_TEST_CASE( AddConstRef )
{
    BOOST_CHECK( ( ! std::is_same< int, add_const_ref< int >::type >::value ) );
    BOOST_CHECK( ( std::is_same< const int&, add_const_ref< int >::type >::value ) );
    BOOST_CHECK( ( std::is_same< const int&, add_const_ref< int >::type2 >::value ) );
    BOOST_CHECK( ( std::is_same< int&, add_const_ref< int& >::type2 >::value ) );
}

namespace
{
    // case not handled (too many template specialization otherwise)
    //     - const Type
    //     - function with more than 1 argument

    template< typename T, typename X /* to replace */, typename Y /* replace with */ >
    struct replace_type { typedef typename T type; };

    template< typename X, typename Y >
    struct replace_type< X, X, Y > { typedef typename Y type; };

    template< typename X, typename Y >
    struct replace_type< X&, X, Y > { typedef typename Y& type; };

    template< typename X, typename Y >
    struct replace_type< X*, X, Y > { typedef typename Y* type; };

    template< typename T, typename X, typename Y, int N >
    struct replace_type< T[N], X, Y >
    {
        typedef typename replace_type< T, X, Y >::type atomicType;
        typedef typename atomicType type[N];
    };

    template < typename T, typename X, typename Y >
    struct replace_type< T (*) (), X, Y >
    {
        typedef typename replace_type< T, X, Y >::type returnType;
        typedef typename returnType (* type) ();
    };
 
    template< typename TR, typename T1, typename X, typename Y >
    struct replace_type< TR (*) (T1), X, Y >
    {
        typedef typename replace_type< TR, X, Y >::type returnType;
        typedef typename replace_type< T1, X, Y >::type argumentType;
 
        typedef typename returnType (* type) (argumentType);
    };
}

// metafunction replace_type< T, X, Y > takes an arbitrary type T as its first parameter, and replaces all occurrences of a type X within T with Y:
BOOST_AUTO_TEST_CASE( ReplaceType )
{
    BOOST_CHECK( ( std::is_same< int, replace_type< void, void, int >::type >::value ) );
    BOOST_CHECK( ( std::is_same< int, replace_type< int, double, float >::type >::value ) );
    BOOST_CHECK( ( std::is_same< int*, replace_type< void*, void, int >::type >::value ) );
    BOOST_CHECK( ( std::is_same< int&, replace_type< double&, double, int >::type >::value ) );
    BOOST_CHECK( ( std::is_same< int[5], replace_type< double[5], double, int >::type >::value ) );
    BOOST_CHECK( ( std::is_same< int (*) (), replace_type< double (*) (), double, int >::type >::value ) );
    BOOST_CHECK( ( std::is_same< int* (*) ( float ), replace_type< double* (*) ( float ), double, int >::type >::value ) );
    BOOST_CHECK( ( std::is_same< int (*) ( int& ), replace_type< double (*) ( double& ), double, int >::type >::value ) );
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
        typedef typename boost::remove_reference< Target >::type targetNoRef;
        assert( dynamic_cast< targetNoRef* >( &s ) == &s );
        return static_cast< Target >( s );
    }

    struct Base { virtual ~Base() {} /* dynamic_cast require one virtual to be considered as polymorphic */ };
    struct Derived : public Base {};
}

BOOST_AUTO_TEST_CASE( PolymorphicDowncast )
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

BOOST_AUTO_TEST_SUITE_END() // TMPExercices
