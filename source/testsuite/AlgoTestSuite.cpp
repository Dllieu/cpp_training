//--------------------------------------------------------------------------------
// (C) Copyright 2014-2015 Stephane Molina, All rights reserved.
// See https://github.com/Dllieu for updates, documentation, and revision history.
//--------------------------------------------------------------------------------

#pragma warning( push )
#pragma warning( disable : 4996 ) // warning raise when using unit_test with multi_array instanciation
#include <boost/test/unit_test.hpp>
#pragma warning( pop )

#include <boost/multi_array.hpp>
#include <numeric>
#include <functional>
#include <iostream>
#include <memory>
#include <stack>
#include <set>
#include <queue>
#include <climits>
#include <array>

BOOST_AUTO_TEST_SUITE( AlgoTestSuite )

namespace
{
    inline std::queue< unsigned >  sortedVectorToQueue( const std::vector< unsigned >& v )
    {
        BOOST_REQUIRE(!v.empty());
        BOOST_REQUIRE(std::is_sorted(std::begin(v), std::end(v)));
        return std::queue<unsigned>(std::deque<unsigned>(std::begin(v), std::end(v)));
    }

    std::pair< unsigned, unsigned >    getMinMaxValues( const std::vector< std::queue<unsigned > >& queues, unsigned& posMinElement )
    {
        std::pair< unsigned, unsigned > minMaxValues(UINT_MAX, 0);
        auto f = [ &minMaxValues, &posMinElement ]( unsigned pos, unsigned value )
        {
            if ( ( minMaxValues.first = std::min( minMaxValues.first, value ) ) == value)
                posMinElement = pos;
            minMaxValues.second = std::max( minMaxValues.second, value );
        };
        for ( unsigned i = 0; i < queues.size(); ++i )
            f( i, queues[i].front() );

        return minMaxValues;
    }

    // dequeue smallest element
    inline bool    iterate( std::vector< std::queue< unsigned > >& queues, unsigned currentMinPos )
    {
        queues[currentMinPos].pop();
        return !queues[currentMinPos].empty();
    }

    template <typename... Ts>
    std::pair< unsigned, unsigned >    getSmallestRange( const Ts&... vectors )
    {
        static_assert( sizeof...(Ts) > 0, "missing argument" );
        std::vector< std::queue< unsigned > > sortedQueues{ sortedVectorToQueue(vectors)... };

        std::pair< unsigned, unsigned > smallestRange;
        auto minDiff = UINT_MAX;
        unsigned currentMinPos;
        do
        {
            auto currentRange = getMinMaxValues( sortedQueues, currentMinPos );
            if (currentRange.second - currentRange.first < minDiff)
            {
                minDiff = currentRange.second - currentRange.first;
                smallestRange = currentRange;
            }
        } while ( iterate( sortedQueues, currentMinPos ) );

        return smallestRange;
    }
}

// You have k lists of sorted integers. Find the smallest range that includes at least one number from each of the k lists.
BOOST_AUTO_TEST_CASE( SmallestRangeTest )
{
    std::vector<unsigned> v1{ 4, 10, 15, 24, 26 };
    std::vector<unsigned> v2{ 0, 9, 12, 20 };
    std::vector<unsigned> v3{ 5, 18, 22, 30 };

    const auto& result = getSmallestRange(v1, v2, v3);
    // The smallest range here would be [20, 24] as it contains 24 from list 1, 20 from list 2, and 22 from list 3
    BOOST_CHECK( result.first == 20 && result.second == 24 );
}

BOOST_AUTO_TEST_CASE( RotateTest )
{
    std::vector< int > v( 10 ), v1, v2;
    std::iota( v.begin(), v.end(), 0 ); // 0 - 9
    v1 = v2 = v;

    auto rotate1 = []( auto first, auto newFirst, auto last )
    {
        auto next = newFirst;
        while ( first != next )
        {
            std::iter_swap( first++, next++ );
            if ( next == last )
                next = newFirst;
            else
                newFirst = next;
        };
    };

    auto rotate2 = []( auto first, auto newFirst, auto last )
    {
        std::reverse( first, newFirst );
        std::reverse( newFirst, last );
        std::reverse( first, last );
    };

    // Rotate left (0 1 2 3 -> 1 2 3 0)
    std::rotate( v.begin(), v.begin() + 1, v.end() );
    rotate1( v1.begin(), v1.begin() + 1, v1.end() );
    rotate2( v2.begin(), v2.begin() + 1, v2.end() );
    BOOST_CHECK( v == v1 && v == v2);
}

BOOST_AUTO_TEST_CASE( MinimumSumTest )
{
    // Minimal sum of two numbers given a list of integers
    std::vector< unsigned >  v { 1, 2, 7, 9, 8 };

    auto minimalSum = [] ( std::vector< unsigned > values /* per value */ )
    {
        std::sort( values.begin(), values.end() );

        unsigned result[] = { 0, 0 };
        for ( size_t i = 0; i < values.size(); ++i )
            result[ i & 1 ] = result[ i & 1 ] * 10 + values[ i ];

        return result[ 0 ] + result[ 1 ];
    };

    BOOST_CHECK( minimalSum( v ) == 207 ); // 128 + 79
}

namespace
{
    template < typename T >
    auto  generate_n_primes( unsigned int n )
    {
        if ( n == 0 )
            return std::vector< T >();

        std::vector< T > result{ 2 };
        --n;
        for ( T i = 3; n != 0 && i + 2 < std::numeric_limits< T >::max(); i += 2 )
            if ( std::all_of( std::next( result.cbegin() ), result.cend(), [ i ]( T v ){ return i % v != 0; } ) )
            {
                --n;
                result.emplace_back( i );
            }

        return result;
    }
}

BOOST_AUTO_TEST_CASE( PrimeTest )
{
    BOOST_CHECK( generate_n_primes< char >( 10000 ).size() == 30 );
    BOOST_CHECK( generate_n_primes< unsigned char >( 10000 ).size() == 54 );
    BOOST_CHECK( generate_n_primes< int >( 100 ).size() == 100 );
}

namespace
{
    void    generate_all_permutations( std::string& str, std::size_t begin, std::size_t end, std::set< std::string >& result )
    {

        auto range = end - begin;

        if ( ! range )
            return;

        if ( range == 1 )
        {
            result.insert( str );
            return;
        }

        for ( auto i = 0; i < range; ++i )
        {
            std::swap( str[ begin ], str[ begin + i ] );
            generate_all_permutations( str, begin + 1, end, result );
            std::swap( str[ begin ], str[ begin + i ] );
        }
    }

    void    generate_all_permutations( std::string str, std::set< std::string >& result )
    {
        generate_all_permutations( str, 0, str.size(), result );
    }
}

BOOST_AUTO_TEST_CASE( PermutationTest )
{
    std::string toBePermuted( "dpaskm" );

    // next_permutation require the input to be sorted
    std::sort( std::begin( toBePermuted ), std::end( toBePermuted ) );

    std::set< std::string > resultWithoutSTL;
    {
        BOOST_TEST_MESSAGE("-- Permutation without STL --");
        generate_all_permutations( toBePermuted, resultWithoutSTL );
    }

    std::set< std::string > resultWithSTL;
    {
        BOOST_TEST_MESSAGE("-- Permutation with STL --");
        do
        {
            resultWithSTL.insert( toBePermuted );
        } while ( std::next_permutation( toBePermuted.begin(), toBePermuted.end() ) );
    }

    BOOST_CHECK( resultWithoutSTL == resultWithSTL );
}

BOOST_AUTO_TEST_CASE( ProductOfArrayTest )
{
    // Given an array A[N] containing N numbers. Crate an array Output[N] where Output[i] is equal to the product of all the elements of A[N] except A[i]
    // Do this in O(n)
    std::vector< int > refArray = { 6, 8, 3 };
    std::vector< int > expectedResult;

    int productResult = std::accumulate( refArray.begin(), refArray.end(), 1, std::multiplies< int >() );
    for ( const auto& v : refArray )
    {
        BOOST_REQUIRE( v != 0 );
        expectedResult.push_back( productResult / v );
    }

    // Do this in O(n) without using the operator /
    std::vector< int >  leftResult;
    {
        int tmp = 1;
        for ( const auto& v : refArray )
        {
            leftResult.push_back( tmp );
            tmp *= v;
        }
    }

    std::vector< int >  rightResult;
    {
        int tmp = 1;

        for ( auto it = std::rbegin( refArray ); it != std::rend( refArray ); ++it )
        {
            rightResult.insert( rightResult.begin(), tmp );
            tmp *= *it;
        }
    }

    std::vector< int > actualResult;
    for ( std::size_t i = 0; i < refArray.size() ; ++i )
        actualResult.push_back( leftResult[ i ] * rightResult[ i ] );

    BOOST_CHECK( actualResult == expectedResult );
}

namespace
{
    static const std::string acceptedParenthesis = "({[<)}]>";

    bool    isBalancedParenthesis( const std::string& expression )
    {
        std::stack< char > stack;
        std::size_t mid = acceptedParenthesis.size() / 2;

        for ( const auto& c : expression )
        {
            std::size_t posParenthesis = acceptedParenthesis.find(c);
            if ( posParenthesis == std::string::npos )
                continue;

            if ( posParenthesis < mid )
                stack.push( c );
            else if ( ! stack.empty() && stack.top() == acceptedParenthesis[ posParenthesis - mid ] )
                stack.pop();
            else
                return false;
        }
        return stack.empty();
    }
}

BOOST_AUTO_TEST_CASE( CheckBalancedParenthesisTest )
{
    BOOST_CHECK( isBalancedParenthesis("no parenthesis") );
    BOOST_CHECK( ! isBalancedParenthesis("(") );
    BOOST_CHECK( isBalancedParenthesis("()") );
    BOOST_CHECK( isBalancedParenthesis("{}") );
    BOOST_CHECK( isBalancedParenthesis("[]") );
    BOOST_CHECK( isBalancedParenthesis("<>") );
    BOOST_CHECK( isBalancedParenthesis("()[]{<>}") );
    BOOST_CHECK( ! isBalancedParenthesis("([)]") );
}

namespace
{
    struct Parent
    {
        Parent() { std::cout << "Create " << name() << std::endl; }
        Parent( const Parent& P ) { std::cout << "Create copy constructor " << name() << std::endl; }
        Parent& operator=( const Parent& P ) { std::cout << "Create copy constructor " << name() << std::endl; }
        virtual ~Parent() { std::cout << "Destroy " << name() << std::endl; }
        virtual std::string name() const { return "Parent"; }
    };

    struct Child : public Parent
    {
        Child() { std::cout << "Create " << name() << std::endl; }
        ~Child() { std::cout << "Destroy " << name() << std::endl; }
        std::string name() const override { return "Child"; }
    };

    std::string    f1( Parent p ) { return p.name(); } // copy constructor + destructor of parent
    std::string    f2( Parent& p ) { return p.name(); }
    std::string    f3( Parent* p ) { return p->name(); }
}

BOOST_AUTO_TEST_CASE( HierarchyTest )
{
    Child child;

    BOOST_CHECK( f1( child ) == "Parent" );
    BOOST_CHECK( f2( child ) == "Child" );
    BOOST_CHECK( f3( &child ) == "Child" );

    Parent p = Parent(); // Won't call the operator= nor the copy constructor, just the default constructor
}

namespace
{
    // Overkill name
    // O(n), require that the list start from 1
    std::size_t     missingElementVectorStartingFromOne( const std::vector< int >& v )
    {
        BOOST_REQUIRE( *std::min_element( std::begin( v ), std::end( v ) ) == 1 );

        return ( v.size() + 1 ) * ( v.size() + 2 ) / 2 - std::accumulate( std::begin( v ), std::end( v ), 0 );
    }

    template < int N /*Expected number of elements*/ >
    std::vector< int >    missingElements( const std::vector< int >& v )
    {
        // NRVO (Named Return Value Optimization)
        // Compiler optimization technique that involves eliminating the temporary object created to hold a function's return value
        // (result will be created on the caller stack frame)
        std::vector< int > result;
        if ( v.size() >= N )
            return result;

        // Need to construct array with a initializer_list to default the contained values
        std::array< int, N > a{};

        auto minElement = *std::min_element( std::begin( v ), std::end( v ) );
        for ( const auto& i : v )
            a[ i - minElement ] = 1;

        // could do in log(n) if N == 2
        for ( auto i = 0; i < N; ++i )
            if ( ! a[ i ] )
                result.emplace_back( i + minElement );

        return result;
    }
}

// From a contigous array which hold consecutive values, find the missing value
BOOST_AUTO_TEST_CASE( MissingElementTest )
{
    // Single element
    {
        std::vector< int >  missing4{ 1, 2, 3, 5 };
        BOOST_CHECK( missingElementVectorStartingFromOne( missing4 ) == 4 );
    }

    // N elements
    {
        std::vector< int >  missingThreeFour{ 2, 5, 6 };
        auto result = missingElements< 5 >( missingThreeFour );
        BOOST_CHECK( ( result == std::vector< int >{ 3, 4 } ) );
    }
}

namespace
{
    bool    findMatrixElement( const boost::multi_array< int, 2 >& matrix, int toFind )
    {
        if ( matrix.empty() )
            return false;

        auto increment = matrix.num_elements() / 2;
        auto colSize = matrix.num_elements() / matrix.size();
        auto i = increment;
        do
        {
            auto row = i / colSize;
            auto col = i - row * colSize;
            auto v = matrix[ row ][ col ];
            if ( v == toFind )
                return true;

            increment = ( increment + ( increment != 1 ? 1 : 0 ) ) >> 1;
            if ( v < toFind )
                i += increment;
            else
                i -= increment;
        } while ( increment && i < matrix.num_elements() );
        return false;
    }
}

BOOST_AUTO_TEST_CASE( FindElementInSortedMatrixTest )
{
    const int rowSize = 6, colSize = 5;
    // The Boost Multidimensional Array Library provides a class template for multidimensional arrays, as well as semantically equivalent adaptors for arrays of contiguous data (i.e. cache friendly multi array)
    boost::multi_array< int, 2 > matrix( boost::extents[ rowSize ][ colSize ] );

    // Sorted matrix with rowSize * colSize elements (can't use std::iota on boost::multi_array)
    for ( auto i = 0, v = 0; i < rowSize; ++i )
        for ( auto j = 0; j < colSize; ++j )
            matrix[ i ][ j ] = ++v;

    BOOST_CHECK( findMatrixElement( matrix, 5 ) );
    for ( const auto& row : matrix )
        for ( const auto& v : row )
        {
            // Seems the inner for range loop need the bracket when we use this macro
            BOOST_CHECK( findMatrixElement( matrix, v ) );
        }

    BOOST_CHECK( ! findMatrixElement( matrix, -1 ) );
    BOOST_CHECK( ! findMatrixElement( matrix, rowSize * colSize + 1 ) );
}

namespace
{
    // 1 - flatten the matrix by layer
    // 2 - rotate each layer
    // 3 - replace the previous entries
    void    rotate_matrix( boost::multi_array< int, 2 >& matrix, int rowSize, int colSize, int rotations )
    {
        auto minSize = std::min( rowSize, colSize ) / 2;
        std::vector< std::vector< int > > flattenMatrixByLayer( minSize );
        for ( auto i = 0; i < minSize; ++i )
        {
            auto side = rowSize - 1 - 2 * i;
            auto top = colSize - 1 - 2 * i;

            auto& layer = flattenMatrixByLayer[ i ];
            layer.reserve( side * top );
            for ( auto j = 0; j < top; ++j )
                layer.push_back( matrix[ i ][ i + j ] );

            for ( auto j = 0; j < side; ++j )
                layer.push_back( matrix[ i + j ][ i + top ] );

            for ( auto j = 0; j < top; ++j )
                layer.push_back( matrix[ i + side ][ i + top - j ] );

            for ( auto j = 0; j < side; ++j )
                layer.push_back( matrix[ i + side - j ][ i ] );
        }

        for ( auto& v : flattenMatrixByLayer )
            std::rotate( v.begin(), v.begin() + rotations % v.size(), v.end() );

        for ( auto i = 0; i < minSize; ++i )
        {
            auto side = rowSize - 1 - 2 * i;
            auto top = colSize - 1 - 2 * i;

            const auto& layer = flattenMatrixByLayer[ i ];
            auto k = 0;
            for ( auto j = 0; j < top; ++j, ++k )
                matrix[ i ][ i + j ] = layer[ k ];

            for ( auto j = 0; j < side; ++j, ++k )
                matrix[ i + j ][ i + top ] = layer[ k ];

            for ( auto j = 0; j < top; ++j, ++k )
                matrix[ i + side ][ i + top - j ] = layer[ k ];

            for ( auto j = 0; j < side; ++j, ++k )
                matrix[ i + side - j ][ i ] = layer[ k ];
        }
    }
}

BOOST_AUTO_TEST_CASE( MatrixRotationTest )
{
    constexpr const int rowSize = 4, colSize = 4;
    constexpr const int rotations = 2;

    boost::multi_array< int, 2 > matrix( boost::extents[ rowSize ][ colSize ] );
    for ( auto i = 0, v = 0; i < rowSize; ++i )
        for ( auto j = 0; j < colSize; ++j )
            matrix[ i ][ j ] = ++v;

    rotate_matrix( matrix, rowSize, colSize, rotations );

    std::array< int, rowSize * colSize > result {
        3, 4, 8, 12,
        2, 11, 10, 16,
        1, 7, 6, 15,
        5, 9, 13, 14,
    };

    // std::cout is way too slow (specially for huge matrix)
    for ( auto i = 0; i < rowSize; ++i )
    {
        for ( auto j = 0; j < colSize; ++j )
        {
            BOOST_CHECK( matrix[ i ][ j ] == result[ i * rowSize + j ] );
            std::printf( "%d ", matrix[ i ][ j ] );
        }

        std::printf( "\n" );
    }
}

BOOST_AUTO_TEST_SUITE_END() // AlgoTestSuite
