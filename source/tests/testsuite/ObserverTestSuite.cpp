#include <boost/test/unit_test.hpp>
#include <set>

BOOST_AUTO_TEST_SUITE( Observer )

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

    class Observer : public AbstractObserver
    {
    public:
        void    update( const std::string& message )
        {
            std::cout << "Update received: " << message << std::endl;
            lastUpdateMessage = message;
        }

        std::string     lastUpdateMessage;
    };
}

BOOST_AUTO_TEST_CASE( BasicObserverTestSuite )
{
    Subject                                 subject;
    std::shared_ptr< AbstractObserver >     observer = std::make_shared< Observer >();
    std::string                             notificationMessage( "News!" );

    subject.addObserver( observer );
    subject.notify( notificationMessage );

    Observer* realObserver = dynamic_cast<Observer*>( observer.get() );
    BOOST_REQUIRE( realObserver );
    BOOST_CHECK_EQUAL( realObserver->lastUpdateMessage, notificationMessage );
}

BOOST_AUTO_TEST_SUITE_END() // Observer
