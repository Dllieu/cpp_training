//--------------------------------------------------------------------------------
// (C) Copyright 2014-2015 Stephane Molina, All rights reserved.
// See https://github.com/Dllieu for updates, documentation, and revision history.
//--------------------------------------------------------------------------------
#include <boost/test/unit_test.hpp>
#include <map>

#include "tools/Timer.h"

BOOST_AUTO_TEST_SUITE( FunctorTestSuite )

#define FUNCTOR_IMPLEMENTATION( uselessParam ) auto useless = uselessParam;

namespace
{
    typedef std::vector< std::string > TypeArgument;
    inline void realImplementation( TypeArgument copiedData )
    {
        FUNCTOR_IMPLEMENTATION( copiedData );
    }

    struct ObjectFunctor
    {
        inline void    operator()( TypeArgument copiedData ) const
        {
            FUNCTOR_IMPLEMENTATION( copiedData );
        }
    };

    template <typename Functor>
    double    timeCalls( Functor f )
    {
        TypeArgument data;

        tools::Timer t;
        for ( auto i = 0; i < 500000; ++i )
            f( data );

        return t.elapsed();
    }

    void    dispatchCall()
    {
        // NOTHING
    }

    template <typename T, typename ...Ts>
    void    dispatchCall( T&& t, Ts&&... ts )
    {
        timeCalls( t );
        // Not a great test as the function we are using are easily optimized by the compiler
        // BOOST_CHECK( result > dispatchDecreasingCall( ts... ) );
        dispatchCall( ts... );
    }
}

BOOST_AUTO_TEST_CASE( FunctorCall )
{
    // std::bind can't be inlined
    // std::function might not be inline
    dispatchCall( std::bind( &realImplementation, std::placeholders::_1 ),
                  ObjectFunctor(),
                  []( TypeArgument copiedData ) { FUNCTOR_IMPLEMENTATION( copiedData ); },
                  &realImplementation );
}

#undef FUNCTOR_IMPLEMENTATION

BOOST_AUTO_TEST_SUITE_END() // FunctorTestSuite
