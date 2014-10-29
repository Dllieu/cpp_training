#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_SUITE( SmartPointer )

namespace
{
    struct A
    {
        A( const std::string& s ) : s( s ) {}
        std::string s;
    };

    A*      legacyMakeA( const std::string s ) { return new A( s ); }
}

// always use make shared_ptr : http://herbsutter.com/2013/05/29/gotw-89-solution-smart-pointers/
// only case you can't : - shared ptr need to be made from a raw pointer (legacy code)
//                       - or if you need custom deletor
BOOST_AUTO_TEST_CASE( PointerTestSuite )
{
    std::unique_ptr< A > legacy( legacyMakeA( "legacy" ) );
    auto goodWay = std::make_shared< A >( "good way" ); // make_unique in C++14

    std::string stringDeleted;
    {
        std::unique_ptr< A, std::function< void ( A*) > > customDeleter( new A( "custom deleter" ), [ &stringDeleted ]( A* a ) { stringDeleted = a->s; delete a; } );
    }
    BOOST_CHECK( stringDeleted == "custom deleter" );
}

BOOST_AUTO_TEST_SUITE_END() // SmartPointer
