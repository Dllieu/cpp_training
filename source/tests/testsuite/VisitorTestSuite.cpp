//--------------------------------------------------------------------------------
// (C) Copyright 2014-2015 Stephane Molina, All rights reserved.
// See https://github.com/Dllieu for updates, documentation, and revision history.
//--------------------------------------------------------------------------------
#include <memory>
#include <iostream>
#include <boost/test/unit_test.hpp>
#include <boost/variant.hpp>

#include "generic/Visitor.h"

BOOST_AUTO_TEST_SUITE( Visitor )

namespace
{
    struct Option : public designpattern::AbstractVisitable
    {
        Option() : haveBeenVisited( false ) {}

        void    accept( designpattern::AbstractVisitor& visitor ) const
        {
            designpattern::genericVisit( visitor, this );
        }

        mutable bool haveBeenVisited;
    };

    struct Future : public designpattern::AbstractVisitable
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

//http://stackoverflow.com/questions/7867555/best-way-to-do-variant-visitation-with-lambdas

// TODO : http://www.drdobbs.com/generic-programmingtypelists-and-applica/184403813?_requestid=1305388
namespace
{

}

//namespace
//{
//    typedef boost::variant< std::string > var_t;
//
//    template <typename ReturnType, typename... Lambdas>
//    struct lambda_visitor;
//
//    template <typename ReturnType, typename Lambda1, typename... Lambdas>
//    struct lambda_visitor< ReturnType, Lambda1, Lambdas...> :
//
//
//    template <typename ReturnType, typename Lambda1, typename... Lambdas>
//    struct lambda_visitor< ReturnType, Lambda1, Lambdas...>
//        : public lambda_visitor<ReturnType, Lambdas...>, public Lambda1{
//        using Lambda1::operator();
//        using lambda_visitor< ReturnType, Lambdas...>::operator();
//        typedef ReturnType ReturnType_t;
//
//        lambda_visitor(Lambda1 l1, Lambdas... lambdas) : Lambda1(l1), lambda_visitor< ReturnType, Lambdas...>(lambdas...) {
//        }
//
//        lambda_visitor(Lambda1 && l1, Lambdas && ... lambdas) : Lambda1(l1), lambda_visitor< ReturnType, Lambdas...>(lambdas...) {
//        }
//    };
//
//    template <typename ReturnType, typename Lambda1>
//    struct lambda_visitor<ReturnType, Lambda1>
//        : public boost::static_visitor<ReturnType>, public Lambda1{
//        using Lambda1::operator();
//        typedef ReturnType ReturnType_t;
//
//        lambda_visitor(Lambda1 l1) : boost::static_visitor<ReturnType >(), Lambda1(l1) {
//        }
//
//        lambda_visitor(Lambda1 && l1) : boost::static_visitor<ReturnType >(), Lambda1(l1) {
//        }
//    };
//
//    template <typename ReturnType>
//    struct lambda_visitor<ReturnType> : public boost::static_visitor<ReturnType>{
//
//        typedef ReturnType ReturnType_t;
//        lambda_visitor() : boost::static_visitor<ReturnType >() {
//        }
//    };
//
//    template <typename ReturnType>
//    struct default_blank_visitor {
//
//        typedef ReturnType ReturnType_t;
//        inline ReturnType operator() (const boost::blank&) const {
//            return (ReturnType)0;
//        };
//    };
//
//    template<>
//    struct default_blank_visitor<void> {
//
//        typedef void ReturnType_t;
//        inline void operator() (const boost::blank&) const {};
//    };
//
//    template <typename ReturnType, typename... Lambdas>
//    lambda_visitor<ReturnType, default_blank_visitor< ReturnType >, Lambdas...> make_lambda_visitor(Lambdas... lambdas) {
//        return
//        {
//            default_blank_visitor<ReturnType >(), lambdas...
//        };
//        // you can use the following instead if your compiler doesn't
//        // support list-initialization yet
//        //return lambda_visitor<ReturnType, default_blank_visitor<ReturnType> , Lambdas...>( default_blank_visitor<ReturnType>(), lambdas...);
//    };
//    /*
//    template <typename ReturnType, typename... Lambdas>
//    lambda_visitor<ReturnType, default_blank_visitor< ReturnType >, Lambdas...> make_lambda_visitor(Lambdas && ... lambdas) {
//    return
//    {
//    default_blank_visitor<ReturnType > (), lambdas...
//    };
//    // you can use the following instead if your compiler doesn't
//    // support list-initialization yet
//    //return lambda_visitor<ReturnType, default_blank_visitor<ReturnType> , Lambdas...>( default_blank_visitor<ReturnType>(), lambdas...);
//    };*/
//
//    template <typename ReturnType, typename... Lambdas>
//    lambda_visitor<ReturnType, Lambdas...> make_lambda_visitor_override_blank(Lambdas... lambdas) {
//        return
//        {
//            lambdas...
//        };
//        // you can use the following instead if your compiler doesn't
//        // support list-initialization yet
//        //return lambda_visitor<ReturnType, Lambdas...>(lambdas...);
//    }
//}

namespace
{
    
}


BOOST_AUTO_TEST_CASE( asdasd )
{
    auto v = designpattern::makeVariadicVisitor([&](const Option& option) { std::cout << "OPTION" << std::endl; },
                                                [&](const Future& option) { std::cout << "FUTURE" << std::endl; },
                                                [&](int option) { std::cout << "FUTURE" << std::endl; });


    Option o0;
    o0.accept(v);

    Future f;
    f.accept(v);
    //designpattern::makeSuperVisitor(5, 4.0);

    //auto superVisitor = designpattern::makeSuperVisitor(
    //    [&](const Option& option) { std::cout << "option super visitor" << std::endl; }
    //    //,[&](const Future& future) { std::cout << "future" << std::endl; }
    //);

    //Option o0;
    //o0.accept(superVisitor);
}

BOOST_AUTO_TEST_SUITE_END() // Visitor
