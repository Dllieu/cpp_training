//--------------------------------------------------------------------------------
// (C) Copyright 2014-2015 Stephane Molina, All rights reserved.
// See https://github.com/Dllieu for updates, documentation, and revision history.
//--------------------------------------------------------------------------------
#include <boost/test/unit_test.hpp>
#include <set>
#include <iostream>
#include "generic/Observer.h"

BOOST_AUTO_TEST_SUITE( ObserverTestSuite )

namespace
{
    struct ObserverA
    {
        virtual void foo(const std::string& s) const
        {
            std::cout << "A: " << s << std::endl;
        }
    };

    struct ObserverB : ObserverA
    {
        void foo(const std::string& s)  const override
        {
            std::cout << "B: " << s << std::endl;
        }
    };

    struct ObserverC : ObserverA
    {
        void foo(const std::string& s) const override
        {
            std::cout << "C: " << s << std::endl;
            BOOST_CHECK(true);
        }
    };
}

BOOST_AUTO_TEST_CASE( GenericObserverTest )
{
    auto genericObservable = designpattern::Observable<ObserverA>();

    genericObservable.subscribe(std::make_shared<ObserverC>());
    genericObservable.notify(&ObserverA::foo, "hello");
}

namespace
{
    class AbstractObserver
    {
    public:
        virtual ~AbstractObserver() {}

        virtual void    update( const std::string& message ) = 0;
    };
}

namespace
{
    class Subject
    {
    public:
        void    notify( const std::string& message )
        {
            for ( auto& observer : observers )
                //if ( observer ) // overkill check
                    observer->update( message );
        }

        void    addObserver( const std::shared_ptr< AbstractObserver >& observer )
        {
            if ( observer )
                observers.insert( observer );
        }

        void    removeObserver( const std::shared_ptr< AbstractObserver >& observer )
        {
            auto& it = observers.find( observer );
            if ( it != observers.end() )
                observers.erase( observer );
        }

    private:
        std::set< std::shared_ptr< AbstractObserver > >  observers;
    };

    class BasicObserver : public AbstractObserver
    {
    public:
        void    update( const std::string& message ) override
        {
            std::cout << "Update received: " << message << std::endl;
            lastUpdateMessage = message;
        }

        std::string     lastUpdateMessage;
    };
}

BOOST_AUTO_TEST_CASE( BasicObserverTest )
{
    Subject                                 subject;
    std::shared_ptr< AbstractObserver >     observer = std::make_shared< BasicObserver >();
    std::string                             notificationMessage( "News!" );

    subject.addObserver( observer );
    subject.notify( notificationMessage );

    std::shared_ptr< BasicObserver > realObserver = std::dynamic_pointer_cast< BasicObserver >(observer);
    BOOST_REQUIRE( realObserver );
    BOOST_CHECK_EQUAL( realObserver->lastUpdateMessage, notificationMessage );
}

BOOST_AUTO_TEST_SUITE_END() // ObserverTestSuite
