//--------------------------------------------------------------------------------
// (C) Copyright 2014-2015 Stephane Molina, All rights reserved.
// See https://github.com/Dllieu for updates, documentation, and revision history.
//--------------------------------------------------------------------------------
#include <boost/test/unit_test.hpp>

#include "generic/ProxyFunctor.h"

BOOST_AUTO_TEST_SUITE( ProxyFunctorTestSuite )

BOOST_AUTO_TEST_CASE( CachingPolicyTest )
{
    auto n = 0;
    auto functor = [ &n ]( int i ){ return n += i; };

    using policy_type = generics::ProxyPolicyCache< int, int >;
    //using policy_type = generics::ProxyPolicyDisplay< int, int >;
    
    policy_type policy;
    generics::ProxyFunctor< int, int > proxyFunctor( policy, functor );

    BOOST_CHECK( proxyFunctor( 1 ) == 1 && n == 1 );
    BOOST_CHECK( proxyFunctor( 1 ) == 1 && n == 1 );


    std::tuple< int, double > t = std::make_tuple( 5, 10.6 );
}

BOOST_AUTO_TEST_SUITE_END() // ! ProxyFunctorTestSuite
