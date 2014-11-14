#include <boost/test/unit_test.hpp>

#include "pricing/StandardDeviation.h"

using namespace pricing;

BOOST_AUTO_TEST_SUITE( StandardDeviationTestSuite )

BOOST_AUTO_TEST_CASE( ComputeBasicStandardDeviationTestSuite )
{
    double points[] =
    {
        10.01, 10.02, 10.03, 10.04, 10.05
    };

    auto result = StandardDeviation()( std::vector< double >( std::begin( points ), std::end( points ) ) );
    BOOST_REQUIRE( std::fabs( result - 0.0141421 ) < 10e-6 );
}

BOOST_AUTO_TEST_CASE( ComputeStandardDeviationTestSuite )
{
    double points[] =
    {
        15.17, 16.94, 14.94, 14.99, 13.77, 13.75,
        12.67, 12.14, 12.59, 12.48, 14.81, 14.29,
        12.74, 12.52, 11.65, 12.24, 11.42, 12.25,
        12.72, 11.64, 11.09, 11.22, 11.50, 11.36,
        11.84, 12.18, 11.04, 10.90, 11.80, 11.84,
    };

    auto result = StandardDeviation()( std::vector< double >( std::begin( points ), std::end( points ) ) );
    BOOST_REQUIRE( std::fabs( result - 1.4699255 ) < 10e-6 );
}

BOOST_AUTO_TEST_SUITE_END() // StandardDeviationTestSuite