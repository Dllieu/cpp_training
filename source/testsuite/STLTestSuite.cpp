//--------------------------------------------------------------------------------
// (C) Copyright 2014-2015 Stephane Molina, All rights reserved.
// See https://github.com/Dllieu for updates, documentation, and revision history.
//--------------------------------------------------------------------------------
#include <boost/test/unit_test.hpp>
#include <algorithm>
#include <iterator>
#include <tuple>
#include <set>
#include <iostream>
#include <numeric>
#include <random>
#include <unordered_map>
#include <typeindex>

#include "generic/TupleForEach.h"
#include "generic/TuplePrinter.h"

BOOST_AUTO_TEST_SUITE( STL )

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

BOOST_AUTO_TEST_CASE( IteratorTest )
{
    std::vector< int > v = { 9, 5, 3, 2, 1 };
    bool isD1 = isDecreasing( v );
    bool isD2 = isDecreasing( v.begin(), v.end() );

    BOOST_CHECK( isD1 == isD2 && isD1 == true );

    v = { 3, 5, 9, 2, 1 };
    isD1 = isDecreasing( v );
    isD2 = isDecreasing( v.begin(), v.end() );

    BOOST_CHECK( isD1 == isD2 && isD1 == false );
}

BOOST_AUTO_TEST_CASE( SortTest )
{
    std::vector< int > v = { 6, 8, 3 };

    // 3-part hybrid sorting algorithm: introsort is performed first (introsort itself being a hybrid of quicksort and heap sort)
    // to a maximum depth given by 2 log2 n, where n is the number of elements, followed by an insertion sort on the result
    std::sort( v.begin(), v.end() );
    BOOST_CHECK( std::is_sorted( &v[0], &v[0] + v.size() ) );
}

BOOST_AUTO_TEST_CASE( TupleTest )
{
    int i;
    std::string s;
    double d;

    auto f = []() { return std::make_tuple( 1, "Value", 5.6 ); };
    std::tie( i, s, d ) = f();

    BOOST_CHECK( ( std::tuple<int, std::string, double>( i, s, d ) == f() ) );

    // see TuplePrinter
    std::cout << f() << std::endl;
    generics::for_each_tuple( f(), []( const auto& dd ){ BOOST_CHECK( true ); } );
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

BOOST_AUTO_TEST_CASE( SwapTest )
{
    int a = 5;
    int b = a + 1;

    swap( a, b );
    std::swap( a, b );
    BOOST_CHECK( a + 1 == b );
}

BOOST_AUTO_TEST_CASE( DivModuloTest )
{
    auto r = std::div( 15, 5 );
    BOOST_CHECK( r.quot == 3 );
    BOOST_CHECK( r.rem == 0 );

    // rather than doing a / b then much later a % b
    // we can ensure with std::div than both operation are called after each other, helping the compiler to optimize it to use the by product division:
    // On x86 the remainder is a by-product of the division itself so any half-decent compiler should be able to just use it (and not perform a div again). This is probably done on other architectures too.
    // Instruction: DIV src
    // - Note: Unsigned division.Divides accumulator( AX ) by "src".If divisor is a byte value, result is put to AL and remainder to AH.If divisor is a word value,
    //         then DX : AX is divided by "src" and result is stored in AX and remainder is stored in DX.
    //int c = a / b;
    //int d = a % b; /* Likely uses the result of the division. */
}

BOOST_AUTO_TEST_CASE( TransparentOperatorFunctorTest )
{
    // since vs2015 (http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2012/n3421.htm)
    std::vector< int > v{ 5, 2, 9, 4, 3, 2 };

    //std::sort( v.begin(), v.end(), std::greater< int >() ); // old : might potentially have implicit conversion
    //std::sort( v.begin(), v.end(), []( auto a, auto b ) { return b > a; } ); // too verbose
    std::sort( v.begin(), v.end(), std::greater<>() ); // type deduced

    BOOST_CHECK( std::is_sorted( v.begin(), v.end(), std::greater<>() ) );
}

namespace
{
    struct Base {};
    struct Derived : Base {};
}

BOOST_AUTO_TEST_CASE( TypeIndexTest )
{
    // The type_index class is a wrapper class around a std::type_info object, that can be used as index
    std::unordered_map< std::type_index, std::string > typeNames{
        { std::type_index( typeid( int ) ), "int" },
        { std::type_index( typeid( double ) ), "double" },
        { std::type_index( typeid( Base ) ), "Base" },
        { std::type_index( typeid( Derived ) ), "Derived" },
    };

    // typeid returns type identification information at run time, it returns a type_info object, which is equality-comparable with other type_info objects
    BOOST_CHECK( typeid( int ) == typeid( int ) ); // can't be computed at compile time
    Derived d;
    Base* b = &d;
    BOOST_CHECK( typeid( b ) == typeid( Base* ) );
    BOOST_CHECK( typeid( b ) != typeid( Derived* ) );

    BOOST_CHECK( typeNames[ std::type_index( typeid( int ) ) ] == "int" );
    BOOST_CHECK( typeNames[ std::type_index( typeid( Base ) ) ] == "Base" );
    BOOST_CHECK( typeNames[ std::type_index( typeid( Derived ) ) ] == "Derived" );
}

BOOST_AUTO_TEST_CASE( ContiguousContainerTest )
{
    std::vector< int > v;
    v.reserve(300);
    auto oldCapacity = v.capacity();
    
    v = { 1, 3, 5 }; // waste of at least 297 elements
    std::vector< int >(v.begin(), v.end()).swap(v); // eliminate excess capacity, size will be at least 3, impl might establish minimum capacities
    BOOST_CHECK( v.capacity() <= oldCapacity && v.capacity() >= 3 );
    
    v.shrink_to_fit();
    BOOST_CHECK( v.capacity() == 3 );
}

namespace
{
    struct GenerateHelper
    {
        GenerateHelper()
            : n( 0 )
        {
            // NOTHING
        }

        int operator()()
        {
            return n++;
        }

        int n;
    };
}

BOOST_AUTO_TEST_CASE( AlgoTest )
{
    std::vector< int > v( 6 );

    // Fills the range [first, last) with sequentially increasing values, starting with value and repetitively evaluating ++value
    std::iota( v.begin(), v.end(), 0 /*value*/ ); // numeric
    std::copy( v.begin(), v.end(), std::ostream_iterator< int >( std::cout /*buffer*/, " " /*delimiter*/) );
    std::cout << std::endl;

    auto it = std::find( v.begin(), v.end(), 3 );
    BOOST_CHECK( it != v.end() );

    auto itAdvance = it;
    // call n times operator+/- if random operator, otherwise operator++/--
    std::advance( itAdvance, 2 );

    // rotate [new first, last[ with elements from [first, new first[
    std::rotate( v.begin() /*first*/, it/*new first to be swapped*/, itAdvance /*last*/ );
    BOOST_CHECK( ( v == std::vector< int >{ 3, 4, 0, 1, 2, 5 } ) );

    std::vector< int > w( 4 ); // size 4, all 0s
    std::generate( w.begin(), w.end(), GenerateHelper() );
    BOOST_CHECK( ( w == std::vector< int >{ 0, 1, 2, 3 } ) );

    std::shuffle( v.begin(), v.end(), std::mt19937{ std::random_device{}() } ); // Reorders the elements in the given range
    std::sort( v.begin(), v.end() );
    // includes return true if the sorted range [v.begin, end[ contains all the sorted element of [w.begin, end[
    BOOST_CHECK( std::includes( v.begin(), v.end(), w.begin(), w.end()/*, ComparatorFunctor*/ ) );

    auto maxElement = std::max_element( v.begin(), v.end() );
    // heap organize the elements with the highest_value on top, other elements depends of the implementation, but are usually in reverse order (at leat with make_heap)
    std::make_heap( v.begin(), v.end() );
    BOOST_CHECK( std::is_heap( v.begin(), v.end() ) );
    // rearrange element in the heap range so that the highest element is moved to the end, and reorganize the heap so that the 2nd highest element is moved on top of the heap
    std::pop_heap( v.begin(), v.end() );
    BOOST_CHECK( v.back() == *maxElement );
    v.pop_back();

    v.push_back( 50 );
    // this function extends the range considered a heap to [first,last) by placing the value in (last-1) into its corresponding location within it
    std::push_heap( v.begin(), v.end() );
    BOOST_CHECK( v.front() == 50 );

    v = { 1, 2, 3, 4 };
    w = { 1, 2, 1001, 3, 4 };
    // return the first element for both sequences that does not match (if match return last of both iterator)
    auto missmatchPairIt = std::mismatch( v.begin(), v.end(), w.begin(), w.end() );
    BOOST_CHECK( missmatchPairIt.first == v.begin() + 2 && missmatchPairIt.second == w.begin() + 2 );

    BOOST_CHECK( std::none_of( v.begin(), v.end(), [] ( auto i ) { return i < 0; } ) );
    BOOST_CHECK( std::all_of( v.begin(), v.end(), [] ( auto i ) { return i > 0; } ) );

    w = v;
    // Rearranges the elements from the range [first,last), in such a way that all the elements for which pred returns true precede all those for which it returns false.
    // The iterator returned points to the first element of the second group
    it = std::partition( v.begin(), v.end(), [] ( auto i ) { return ( i & 1 ) == 0; } );
    BOOST_CHECK( ( *it & 1 ) != 0 );
    BOOST_CHECK( ( v == std::vector< int >{ 4, 2, 3, 1 } ) );

    std::stable_partition( w.begin(), w.end(), [] ( auto i ) { return ( i & 1 ) == 0; } );
    BOOST_CHECK( ( w == std::vector< int >{ 2, 4, 1, 3 } ) );

    v = { 2, 3, 4, 2, 3, 1 };
    w = { 2, 3 };
    BOOST_CHECK( std::accumulate( w.begin(), w.end(), 0 ) == 5 );
    BOOST_CHECK( std::accumulate( w.begin(), w.end(), std::string(), []( const auto& result, auto element ) { return result + std::to_string( element ); } ) == "23" );

    // Searches the range [first1,last1) for the FIRST occurrence of the sequence defined by [first2,last2), and returns an iterator to its first element, or last1 if no occurrences are found.
    it = std::search( v.begin(), v.end(), w.begin(), w.end() );
    std::advance( it, 2 );
    BOOST_CHECK( *it == 4 );

    // Searches the range [first1,last1) for the LAST occurrence of the sequence defined by [first2,last2), and returns an iterator to its first element, or last1 if no occurrences are found.
    it = std::find_end( v.begin(), v.end(), w.begin(), w.end() );
    std::advance( it, 2 );
    BOOST_CHECK( *it == 1 );

    std::sort( v.begin(), v.end() );

    decltype( v ) z( v.size() );
    // Constructs a sorted range beginning in the location pointed by result with the set difference of the sorted range [first1,last1) with respect to the sorted range [first2,last2)
    // return An iterator to the end of the constructed range.
    it = std::set_difference( v.begin(), v.end(), w.begin(), w.end(), z.begin() );
    // 1, 2, 3, 4, 0, 0
    z.resize( it - z.begin() );
    BOOST_CHECK( ( z == std::vector< int >{ 1, 2, 3, 4 } ) );

    z.resize( v.size() );
    it = std::set_intersection( v.begin(), v.end(), w.begin(), w.end(), z.begin() );
    // 2, 3, 0, 0, 0, 0
    z.resize( it - z.begin() );
    BOOST_CHECK( ( z == std::vector< int >{ 2, 3 } ) );

    v = { 1, 2, 2, 3 };
    // lower_bound / upper_bound requires it
    BOOST_REQUIRE( std::is_sorted( v.begin(), v.end() ) );
    // lower_bound / upper_bound are O(log(n)) for random iterator, O(n) otherwise
    auto lowerBound = std::lower_bound( v.begin(), v.end(), 2 ); // first occurence that is >= val
    BOOST_CHECK( *lowerBound == 2 /*the first one*/ );
    auto upperBound = std::upper_bound( v.begin(), v.end(), 2 ); // first occurence that is > val
    BOOST_CHECK( *upperBound == 3 );

    BOOST_CHECK( upperBound - lowerBound == 2 );
}

BOOST_AUTO_TEST_SUITE_END() // STL
