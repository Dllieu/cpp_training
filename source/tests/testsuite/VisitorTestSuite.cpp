//--------------------------------------------------------------------------------
// (C) Copyright 2014-2015 Stephane Molina, All rights reserved.
// See https://github.com/Dllieu for updates, documentation, and revision history.
//--------------------------------------------------------------------------------
#include <iostream>
#include <memory>
#include <boost/test/unit_test.hpp>

#include "generic/Visitor.h"

BOOST_AUTO_TEST_SUITE( Visitor )

namespace
{
    struct AbstractVisitable
    {
        virtual ~AbstractVisitable() {};
        virtual void    accept( designpattern::AbstractVisitor& visitor ) const = 0;
    };

    struct Option : public AbstractVisitable
    {
        Option() : haveBeenVisited( false ) {}

        void    accept( designpattern::AbstractVisitor& visitor ) const
        {
            designpattern::genericVisit( visitor, this );
        }

        mutable bool haveBeenVisited;
    };

    struct Future : public AbstractVisitable
    {
        Future() : haveBeenVisited( false ) {}

        void    accept( designpattern::AbstractVisitor& visitor ) const
        {
            designpattern::genericVisit( visitor, this );
        }

        mutable bool haveBeenVisited;
    };

    struct ProductVisitor : public designpattern::AbstractAcyclicVisitor< Option >
                          , public designpattern::AbstractAcyclicVisitor< Future >
    {
        void    visit( const Option& option )
        {
            option.haveBeenVisited = true;
        }

        void    visit( const Future& future )
        {
            future.haveBeenVisited = true;
        }
    };
}

BOOST_AUTO_TEST_CASE( GenericLambdaVisitor )
{
    bool isOption = false;
    auto visitor = designpattern::makeVisitor( [&]( const Option& option) { std::cout << "option" << std::endl; isOption = true; },
                                               [&]( const Future& future) { std::cout << "future" << std::endl; } );

    Option o;
    o.accept( visitor );
    BOOST_CHECK( isOption );
}

BOOST_AUTO_TEST_CASE( AcyclicVisitorTestSuite )
{
    ProductVisitor  visitor;
    Option          option;
    Future          future;

    BOOST_CHECK( ! option.haveBeenVisited );
    BOOST_CHECK( ! future.haveBeenVisited );

    option.accept( visitor );
    future.accept( visitor );

    BOOST_CHECK( option.haveBeenVisited );
    BOOST_CHECK( future.haveBeenVisited );
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

        void    accept( const AbstractBasicVisitor* v ) const
        {
            if ( v )
                v->visit( *this );
        }

        mutable bool        haveBeenVisited;
    };

    struct VisitableB : public AbstractBasicVisitable
    {
        VisitableB() : haveBeenVisited( false ) {}

        void    accept( const AbstractBasicVisitor* v ) const
        {
            if ( v )
                v->visit( *this );
        }

        mutable bool        haveBeenVisited;
    };

    struct BasicVisitor : public AbstractBasicVisitor
    {
        void    visit( const VisitableA& v ) const
        {
            v.haveBeenVisited = true;
        }

        void    visit( const VisitableB& v ) const
        {
            v.haveBeenVisited = true;
        }
    };
}

BOOST_AUTO_TEST_CASE( BasicVisitorTestSuite )
{
    std::unique_ptr< AbstractBasicVisitor > v( new BasicVisitor() );
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
