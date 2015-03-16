//--------------------------------------------------------------------------------
// (C) Copyright 2014-2015 Stephane Molina, All rights reserved.
// See https://github.com/Dllieu for updates, documentation, and revision history.
//--------------------------------------------------------------------------------
#include <boost/test/unit_test.hpp>

#include "containers/SparseArray.h"

using namespace containers;

BOOST_AUTO_TEST_SUITE( CustomContainer )

namespace
{
    enum PricingResult
    {
        SPOT,
        PREMIUM,
        DELTA,
        THETA,
        GAMMA,
        VOMMA,
        VONNA,
        PRICINGRESULT_SIZE
    };

    static_assert( PRICINGRESULT_SIZE > 0, "Invalid PricigResult size" );

    template < typename T >
    auto    constBracketOperator( const T& sparseArray, int index ) -> decltype( sparseArray[ index ] )
    {
        return sparseArray[ index ];
    }
}

BOOST_AUTO_TEST_CASE( SparseArrayTestSuite )
{
    SparseArray< double, PricingResult::PRICINGRESULT_SIZE > sparseArray;

    sparseArray[ PricingResult::DELTA ] = 12.0;
    BOOST_CHECK( sparseArray.isInitialized( PricingResult::DELTA ) );

    for ( auto i = 0; i < PricingResult::PRICINGRESULT_SIZE; ++i )
        if ( i == PricingResult::DELTA )
            BOOST_CHECK( sparseArray[ PricingResult::DELTA ] == 12.0 );
        else
            BOOST_CHECK_THROW( constBracketOperator( sparseArray, i ), std::out_of_range );

    BOOST_CHECK( sparseArray.size() == 1 );
}

BOOST_AUTO_TEST_SUITE_END() // CustomContainer
