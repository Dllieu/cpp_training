#include <boost/test/unit_test.hpp>
#include <boost/assign/list_of.hpp>
#include <numeric>
#include <functional>
#include <iostream>
#include <memory>
#include <stack>
#include <set>

BOOST_AUTO_TEST_SUITE( Interview )


BOOST_AUTO_TEST_CASE( MinimumSumTestSuite )
{
    // Minimal sum of two numbers given a list of integers
    std::vector< unsigned >  v = boost::assign::list_of( 1 )( 2 )( 7 )( 9 )( 8 );

    auto minimalSum = [] ( std::vector< unsigned > values /* per value */ ) {
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
    void    displayAllPermutations( std::string& str, int begin, int end, std::set< std::string >& result )
    {

        int range = end - begin;

        if ( ! range )
            return;

        if ( range == 1 )
        {
            result.insert( str );
            std::cout << str << std::endl;
            return;
        }

        for ( int i = 0; i < range; ++i )
        {
            std::swap( str[ begin ], str[ begin + i ] );
            displayAllPermutations( str, begin + 1, end, result );
            std::swap( str[ begin ], str[ begin + i ] );
        }
    }

    void    displayAllPermutations( std::string& str, std::set< std::string >& result )
    {
        displayAllPermutations( str, 0, str.size(), result );
    }
}

BOOST_AUTO_TEST_CASE( PermutationTestSuite )
{
    std::string toBePermuted( "abc" );

    std::set< std::string > resultWithoutSTL;
    {
        BOOST_MESSAGE("-- Permutation without STL --");
        displayAllPermutations( toBePermuted, resultWithoutSTL );
    }

    std::set< std::string > resultWithSTL;
    {
        BOOST_MESSAGE("-- Permutation with STL --");
        do
        {
            std::cout << toBePermuted << std::endl;
            resultWithSTL.insert( toBePermuted );
        } while ( std::next_permutation( toBePermuted.begin(), toBePermuted.end() ) );
    }

    BOOST_CHECK( resultWithoutSTL == resultWithSTL );
}

BOOST_AUTO_TEST_CASE( ProductOfArrayTestSuite )
{
    // Given an array A[N] containing N numbers. Crate an array Output[N] where Output[i] is equal to the product of all the elements of A[N] except A[i]
    // Do this in O(n)
    std::vector< int > refArray = boost::assign::list_of( 6 )( 8 )( 3 );
    std::vector< int > expectedResult;

    int productResult = std::accumulate( refArray.begin(), refArray.end(), 1, std::multiplies< int >() );
    for ( std::size_t i = 0; i < refArray.size(); ++i )
        expectedResult.push_back( productResult / refArray[ i ] );

    // Do this in O(n) without using the operator /
    std::vector< int >  leftResult;
    {
        int tmp = 1;
        for ( std::size_t i = 0; i < refArray.size(); ++i )
        {
            leftResult.push_back( tmp );
            tmp *= refArray[ i ];
        }
    }

    std::vector< int >  rightResult;
    {
        int tmp = 1;
        for ( int i = refArray.size() - 1; i >= 0; --i )
        {
            rightResult.insert( rightResult.begin(), tmp );
            tmp *= refArray[ i ];
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

        for ( char c : expression )
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

BOOST_AUTO_TEST_CASE( CheckBalancedParenthesis )
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
        virtual ~Parent() { std::cout << "Destroy " << name() << std::endl; }
        virtual std::string name() { return "Parent"; }
    };

    struct Child : public Parent
    {
        Child() { std::cout << "Create " << name() << std::endl; }
        ~Child() { std::cout << "Destroy " << name() << std::endl; }
        std::string name() override { return "Child"; }
    };

    std::string    f1( Parent p ) { return p.name(); } // Destroy parent at the end but doesn't create one with the magic of rvalue ref
    std::string    f2( Parent& p ) { return p.name(); }
    std::string    f3( Parent* p ) { return p->name(); }
}

BOOST_AUTO_TEST_CASE( HierarchyTestCase )
{
    Child child;

    BOOST_CHECK( f1( child ) == "Parent" );
    BOOST_CHECK( f2( child ) == "Child" );
    BOOST_CHECK( f3( &child ) == "Child" );
}

BOOST_AUTO_TEST_SUITE_END() // Interview
