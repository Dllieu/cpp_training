#include <boost/test/unit_test.hpp>
#include <type_traits>
#include <algorithm>
#include <iterator>
#include <tuple>
#include <set>

#include <boost/assign/list_of.hpp> 

BOOST_AUTO_TEST_SUITE( STL )

BOOST_AUTO_TEST_CASE( DecayTestSuite )
{
    BOOST_CHECK( ( std::is_same<int, std::decay<int&>::type>::value ) );
    BOOST_CHECK( ( std::is_same<int, std::decay<int&&>::type>::value ) );
    BOOST_CHECK( ( std::is_same<int, std::decay<const int&>::type>::value ) );
    BOOST_CHECK( ( std::is_same<int*, std::decay<int[2]>::type>::value ) );
    BOOST_CHECK( ( std::is_same<int(*)(int), std::decay<int (int)>::type>::value ) );
}

namespace
{
    template <typename T>
    bool    isDecreasing( const T& container )
    {
        for ( std::size_t i = 1; i < container.size(); ++i )
            if ( container[ i - 1 ] <= container[ i ] )
                return false;
        return true;
    }

    template <typename T>
    bool    isDecreasing( const T& begin, const T& end )
    {
        return std::adjacent_find( begin, end, std::less_equal< std::iterator_traits< T >::value_type >() ) == end;
    }
}

BOOST_AUTO_TEST_CASE( IteratorTestSuite )
{
    std::vector< int > v = boost::assign::list_of( 9 )( 5 )( 3 )( 2 )( 1 );
    bool isD1 = isDecreasing( v );
    bool isD2 = isDecreasing( v.begin(), v.end() );

    BOOST_CHECK( isD1 == isD2 && isD1 == true );

    v = boost::assign::list_of( 3 )( 5 )( 9 )( 2 )( 1 );
    isD1 = isDecreasing( v );
    isD2 = isDecreasing( v.begin(), v.end() );

    BOOST_CHECK( isD1 == isD2 && isD1 == false );
}

BOOST_AUTO_TEST_CASE( SortTestSuite )
{
    std::vector< int > v = boost::assign::list_of( 6 )( 8 )( 3 );

    // 3-part hybrid sorting algorithm: introsort is performed first (introsort itself being a hybrid of quicksort and heap sort)
    // to a maximum depth given by 2 log2 n, where n is the number of elements, followed by an insertion sort on the result
    std::sort( v.begin(), v.end(), [](int a, int b) { return a < b; } );
    BOOST_CHECK( std::is_sorted( &v[0], &v[0] + v.size() ) );
}

BOOST_AUTO_TEST_CASE( TupleTestSuite )
{
    int i;
    std::string s;
    double d;

    auto f = []() { return std::make_tuple( 1, "Value", 5.6 ); };
    std::tie( i, s, d ) = f();

    BOOST_CHECK( ( std::tuple<int, std::string, double>( i, s, d ) == f() ) );
}

BOOST_AUTO_TEST_CASE( ContainerTestSuite )
{
    auto isOdd = []( int a ) { return ( a & 1 ) == 1; /* & < == */ };

    {
        std::vector< int > v = boost::assign::list_of( 6 )( 8 )( 3 );
        std::vector< int > vOld = v;

        // remove shift element to be deleted at the end of the container (elements are ignored)
        // erase delete them definitely
        v.erase( std::remove_if( v.begin(), v.end(), isOdd ), v.end() );

        for ( std::vector<int>::iterator it = vOld.begin(); it != vOld.end(); )
            if ( isOdd( *it ) )
                it = vOld.erase( it );
            else
                ++it;

        BOOST_CHECK( v == vOld );
    }

    {
        std::list< int > l = boost::assign::list_of( 6 )( 8 )( 3 );
        l.remove_if( isOdd );
    }

    {
        std::set< int > s = boost::assign::list_of( 6 )( 8 )( 3 );
        s.erase( 8 );
    }
}

namespace
{
    template <typename T>
    void    swap( T& a, T& b )
    {
        T tmp = std::move( a );
        a = std::move( b );
        b = std::move( tmp );
    }
}

BOOST_AUTO_TEST_CASE( SwapTestSuite )
{
    int a = 5;
    int b = a + 1;

    swap( a, b );
    std::swap( a, b );
    BOOST_CHECK( a + 1 == b );
}

BOOST_AUTO_TEST_SUITE_END() // STL
