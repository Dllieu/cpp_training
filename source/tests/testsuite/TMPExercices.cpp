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

    template <>
    struct add_const_ref_< true >
    {
        template < typename T >
        struct deduce_type { typedef T type; };
    };

    template <>
    struct add_const_ref_< false >
    {
        template < typename T >
        struct deduce_type { typedef const T& type; };
    };

    // returns T if it is a reference type, and otherwise returns T const&
    template < typename T >
    struct add_const_ref
    {
        typedef typename add_const_ref_< boost::is_reference< T >::type::value >::template deduce_type< T >::type type;
        typedef typename boost::mpl::if_< typename boost::is_reference< T >,
                                          typename T,
                                          typename boost::add_reference< typename boost::add_const< T >::type >::type
                                        >::type type2;
    };
}

BOOST_AUTO_TEST_CASE( AddConstRef )
{
    BOOST_CHECK( ( ! std::is_same< int, add_const_ref< int >::type >::value ) );
    BOOST_CHECK( ( std::is_same< const int&, add_const_ref< int >::type >::value ) );
    BOOST_CHECK( ( std::is_same< const int&, add_const_ref< int >::type2 >::value ) );
    BOOST_CHECK( ( std::is_same< int&, add_const_ref< int& >::type2 >::value ) );
}

BOOST_AUTO_TEST_SUITE_END() // TMPExercices
