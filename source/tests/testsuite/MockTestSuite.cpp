//--------------------------------------------------------------------------------
// (C) Copyright 2014-2015 Stephane Molina, All rights reserved.
// See https://github.com/Dllieu for updates, documentation, and revision history.
//--------------------------------------------------------------------------------
#include <boost/test/unit_test.hpp>
#include <turtle/mock.hpp>

namespace
{
    class IBackend
    {
    public:
        virtual bool connect( const std::string& login, const std::string& password ) = 0;
    };

    class BackendClient
    {
    public:
        BackendClient( IBackend& backend )
            : backend_( backend )
        {
            // NOTHING
        }

        bool    connect( const std::string& login, const std::string& password )
        {
            return backend_.connect( login, password );
        }

    private:
        IBackend&   backend_;
    };
}

namespace // Fixture
{
    // Mock implementation of IBackend
    MOCK_BASE_CLASS( MockBackend, IBackend )
    {
        MOCK_METHOD( connect, 2 );
    };

    struct Fixture
    {
        Fixture()
            : login( "login" )
            , password( "password" )
            , mockBackend()
        {
            // NOTHING
        }

        std::string     login;
        std::string     password;
        MockBackend     mockBackend;
    };
}

BOOST_FIXTURE_TEST_SUITE( MockTestSuite, Fixture )

BOOST_AUTO_TEST_CASE( SimpleCallWithConstraint )
{
    // Check that it's called once with specific login and password, it will return true when called
    MOCK_EXPECT( mockBackend.connect ).with( [this] ( const std::string& value ) -> bool { return this->login == value; },
                                             password )
                                      .once()
                                      .returns( true );

    BackendClient backendClient( mockBackend );
    BOOST_CHECK( backendClient.connect( login, password ) );
}

BOOST_AUTO_TEST_SUITE_END() // MockTestSuite
