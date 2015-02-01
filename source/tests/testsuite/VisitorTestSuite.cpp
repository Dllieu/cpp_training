//--------------------------------------------------------------------------------
// (C) Copyright 2014-2015 Stephane Molina, All rights reserved.
// See https://github.com/Dllieu for updates, documentation, and revision history.
//--------------------------------------------------------------------------------
#include <memory>
#include <iostream>
#include <boost/test/unit_test.hpp>

#include "generic/Visitor.h"

BOOST_AUTO_TEST_SUITE( Visitor )

namespace
{
    struct Option : public designpattern::AbstractVisitable< Option >
    {
        // NOTHING
    };

    struct Future : public designpattern::AbstractVisitable< Future >
    {
        // NOTHING
    };

    struct VarianceSwap : public designpattern::AbstractVisitable< VarianceSwap >
    {
        // NOTHING
    };
}

namespace
{
    struct FutureVisitorFunctor
    {
        bool operator()( const Future& future ) const
        {
            std::cout << "FUTURE VISITED" << std::endl;
            return true;
        }
    };
}

BOOST_AUTO_TEST_CASE( VariadicVisitorTestSuite )
{
    Option o;
    Future f;
    VarianceSwap v;

    auto variadicVisitor = designpattern::makeVariadicVisitor([&](const Option& option) { std::cout << "OPTION VISITED" << std::endl; },
                                                              FutureVisitorFunctor());

    BOOST_CHECK( o.accept(variadicVisitor) );
    BOOST_CHECK( f.accept(variadicVisitor) );
    BOOST_CHECK( ! v.accept(variadicVisitor) );
}

namespace
{
    struct VisitableA;
    struct VisitableB;

    struct AbstractBasicVisitor
    {
        virtual ~AbstractBasicVisitor() {}
        virtual void    visit( const VisitableA& v ) const = 0;
        virtual void    visit( const VisitableB& v ) const = 0;
    };

    struct AbstractBasicVisitable
    {
        virtual void    accept( const AbstractBasicVisitor* visitor ) const = 0;
    };

    struct VisitableA : public AbstractBasicVisitable
    {
        VisitableA() : haveBeenVisited( false ) {}

        void    accept( const AbstractBasicVisitor* v ) const override
        {
            if ( v )
                v->visit( *this );
        }

        mutable bool        haveBeenVisited;
    };

    struct VisitableB : public AbstractBasicVisitable
    {
        VisitableB() : haveBeenVisited( false ) {}

        void    accept( const AbstractBasicVisitor* v ) const override
        {
            if ( v )
                v->visit( *this );
        }

        mutable bool        haveBeenVisited;
    };

    struct BasicVisitor : public AbstractBasicVisitor
    {
        void    visit( const VisitableA& v ) const override
        {
            v.haveBeenVisited = true;
        }

        void    visit( const VisitableB& v ) const override
        {
            v.haveBeenVisited = true;
        }
    };
}

BOOST_AUTO_TEST_CASE( BasicVisitorTestSuite )
{
    std::unique_ptr< AbstractBasicVisitor > v = std::make_unique<  BasicVisitor >();
    VisitableA  a;
    VisitableB  b;

    BOOST_CHECK( ! a.haveBeenVisited );
    BOOST_CHECK( ! b.haveBeenVisited );

    a.accept( v.get() );
    b.accept( v.get() );

    BOOST_CHECK( a.haveBeenVisited );
    BOOST_CHECK( b.haveBeenVisited );
}

BOOST_AUTO_TEST_SUITE_END() // Visitor
