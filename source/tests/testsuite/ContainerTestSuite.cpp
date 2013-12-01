#include <boost/test/unit_test.hpp>
#include <boost/assign/list_of.hpp> 
#include <set>
#include <stack>
#include <queue>

BOOST_AUTO_TEST_SUITE( Container )

BOOST_AUTO_TEST_CASE( EraseRemoveTestSuite )
{
    auto isOdd = []( int a ) { return ( a & 1 ) == 1; /* & < == */ };

    {
        std::vector< int > v = boost::assign::list_of( 6 )( 8 )( 3 );
        std::vector< int > vOld = v;

        // remove shift element to be deleted at the end of the container (elements are ignored)
        // erase delete them definitely
        v.erase( std::remove_if( v.begin(), v.end(), isOdd ), v.end() );

        for ( std::vector<int>::iterator it = vOld.begin(); it != vOld.end(); )
            if ( isOdd( *it ) )
                it = vOld.erase( it );
            else
                ++it;

        BOOST_CHECK( v == vOld );
    }

    {
        std::list< int > l = boost::assign::list_of( 6 )( 8 )( 3 );
        l.remove_if( isOdd );
    }

    {
        std::set< int > s = boost::assign::list_of( 6 )( 8 )( 3 );
        s.erase( 8 );
    }
}

BOOST_AUTO_TEST_CASE( ContainerTestSuite )
{
    // Base container : forward_list / list / vector (priority_queue) / deque (stack - queue) / set (map)

    // list keep a link to previous / next element
    // forward_list keep a link to the next element only (have no size() operator because it could only be implemented as O(n) which differ with other stl container implementations)

    // unorder_set / unorder_map : set / map which store element in no particular order, allow fast retrieval of element based by value

    // multiset : set but the keys are not unique (but are still ordered)
    // multimap : can have key/value several time (but they are still ordered by key/value)

    int i = 5;
    {
        // - Ordered
        // - Unique Keys

        // template < class T,                // set::key_type/value_type
        //    class Compare = less<T>,        // set::key_compare/value_compare
        //    class Alloc = allocator<T> > >  // set::allocator_type
        // class set;
        // noticeable methods : insert / erase / ( ( key_comp / value_comp ) (functors used for the comparison) )
        std::set< int > s;
        std::pair< std::set< int >::iterator, bool > resultFromInsertion = s.insert( i );
        BOOST_CHECK( resultFromInsertion.second == true );
    }
    {
        // - Double-ended queues are sequence containers with dynamic sizes that can be expanded or contracted on both ends
        // - the individual elements can be accessed directly through random access iterators, with storage handled automatically by expanding and contracting the container as needed
        // - provide a similar functionality as vectors, but with efficient insertion and deletion of elements also at the beginning of the sequence, and not only at its end
        // - not guaranteed to store all its elements in contiguous storage locations, thus not allowing direct access by offsetting pointers to elements
        // - the elements of a deque can be scattered in different chunks of storage, with the container keeping the necessary information internally
        // to provide direct access to any of its elements in constant time and with a uniform sequential interface
        // - For operations that involve frequent insertion or removals of elements at positions other than the beginning or the end,
        // deques perform worse and have less consistent iterators and references than lists

        // template < class T, class Alloc = allocator<T> > class deque;
        // Alloc : Type of the allocator object used to define the storage allocation model
        std::deque< int >   d;

        d.push_back( i );
        BOOST_CHECK( ! d.empty() && d[ 0 ] == i );
        d.push_front( i + 1 );
        BOOST_CHECK( d.front() - 1 == d.back() );
        d.pop_back();
        d.pop_front();
        BOOST_CHECK( d.empty() );
    }

    {
        // underlying container shall support front / back / push_back / push_front (deque and list fullfill these requirements)

        // template <class T, class Container = deque<T> > class queue;
        // noticeable methods : front / back / push / pop
        std::queue< int >   q; /* FIFO */

        BOOST_CHECK( q.empty() );
    }

    {
        // - Priority queues are a type of container adaptors, specifically designed such that its first element is always the greatest of the elements it contains,
        // according to some strict weak ordering criterion.
        // - This context is similar to a heap, where elements can be inserted at any moment, and only the max heap element can be retrieved (the one at the top in the priority queue)
        // - Underlying container shall support front / push_back / pop_back (vector and deque fullfill these requirements)

        // template <class T, class Container = vector<T>, class Compare = less<typename Container::value_type> > class priority_queue;
        // noticeable methods : top / push / pop
        std::priority_queue< int >  p;

        p.push( 1 );
        p.push( 3 );
        p.push( 2 );
        BOOST_CHECK( p.top() == 3 );
        p.pop();
        BOOST_CHECK( p.top() == 2 );
    }

    {
        // - implemented as containers adaptors, which are classes that use an encapsulated object of a specific container class as its underlying container,
        // providing a specific set of member functions to access its elements, underlying container shall support back / push_back / pop_back

        // template <class T, class Container = std::deque<T> > class stack;
        // noticeable methods : top / push / pop
        std::stack< int >     s; /* LIFO */

        s.push( i );
        BOOST_CHECK( ( s.top() += 2 ) == i + 2 );
        s.pop();
        BOOST_CHECK( s.empty() );
    }
}

BOOST_AUTO_TEST_SUITE_END() // Container
