//--------------------------------------------------------------------------------
// (C) Copyright 2014-2015 Stephane Molina, All rights reserved.
// See https://github.com/Dllieu for updates, documentation, and revision history.
//--------------------------------------------------------------------------------
#include <boost/test/unit_test.hpp>
#include <set>
#include <stack>
#include <queue>
#include <unordered_set>
#include <unordered_map>
#include <bitset>

#include "generic/HashCombine.h"

BOOST_AUTO_TEST_SUITE( Container )

BOOST_AUTO_TEST_CASE( EraseRemoveTest )
{
    auto isOdd = []( int a ) { return ( a & 1 ) == 1; /* & < == */ };

    {
        std::vector< int > v = { 6, 8, 3 };
        std::vector< int > vOld = v;

        // remove shift element to be deleted at the end of the container (elements are ignored)
        // erase delete them definitely
        v.erase( std::remove_if( v.begin(), v.end(), isOdd ), v.end() );

        for ( auto it = vOld.begin(); it != vOld.end(); )
            if ( isOdd( *it ) )
                it = vOld.erase( it );
            else
                ++it;

        BOOST_CHECK( v == vOld );
    }

    {
        std::list< int > l = { 6, 8, 3 };
        l.remove_if( isOdd );
    }

    {
        std::set< int > s = { 6, 8, 3 };
        s.erase( 8 );
    }
}

BOOST_AUTO_TEST_CASE( ContainerTest )
{
    // Base container : forward_list / list / vector (priority_queue) / deque (stack - queue) / set (map)

    // list keep a link to previous / next element
    // forward_list keep a link to the next element only (have no size() operator because it could only be implemented as O(n) which differ with other stl container implementations)

    // unordered_set / unordered_map : set / map which store element in no particular order, allow fast retrieval of element based by value (real hash map)

    // multiset : set but the keys are not unique (but are still ordered)
    // multimap : can have key/value several time (but they are still ordered by key/value)

    // Which container should I use ?
    // - vector if no or few insertion / deletion
    // - deque if a lot of insertion / deletion at the begin / end
    // - list otherwise

    // emplace_back will construct the object in place as opposed to copying or moving

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

    {
        // Internally, the elements are not sorted in any particular order, but organized into buckets depending on their hash values
        // to allow for fast access to individual elements directly by their values (with a constant average time complexity on average)

        // explicit unordered_set( size_type n = /* see below */,
        //                         const hasher& hf = hasher(), // hash the key
        //                         const key_equal& eql = key_equal(), // when we compare a new given key, we first check the hash which will determinate in which "bucket" we are then we compare the remaining keys (at least one) using equal functor with the given key
        //                         const allocator_type& alloc = allocator_type() );
        std::unordered_set< unsigned > u; // behind
        u.insert( 5 );

        BOOST_CHECK( u.find( 5 ) != u.end() );
    }
}

namespace
{
    struct CustomKey
    {
        CustomKey( const std::string& literalKey, unsigned numberKey )
            : literalKey( literalKey )
            , numberKey( numberKey )
        {
            // NOTHING
        }

        CustomKey( CustomKey&& key )
            : literalKey( std::move( key.literalKey ) )
            , numberKey( key.numberKey )
        {
            // NOTHING
        }

        std::string     literalKey;
        unsigned        numberKey;
    };

    struct CustomHash
    {
        inline std::size_t     operator()( const CustomKey& customKey ) const
        {
            return generics::hashCombine(customKey.literalKey, customKey.numberKey);
        }
    };

    struct CustomEqual
    {
        inline bool        operator()( const CustomKey& k1, const CustomKey& k2 ) const
        {
            return k1.literalKey == k2.literalKey
                && k1.numberKey == k2.numberKey;
        }
    };
}

BOOST_AUTO_TEST_CASE( UnorderedMapTest )
{
    // template < class Key,                                    // unordered_map::key_type
    //        class T,                                      // unordered_map::mapped_type
    //        class Hash = hash<Key>,                       // unordered_map::hasher
    //        class Pred = equal_to<Key>,                   // unordered_map::key_equal
    //        class Alloc = allocator< pair<const Key,T> >  // unordered_map::allocator_type
    //        > class unordered_map;
    std::unordered_map< CustomKey, std::string,
                        std::function< std::size_t ( const CustomKey& ) >,
                        std::function< bool ( const CustomKey&, const CustomKey& ) > > unordered_map( 5 /* numberOfBucket */, CustomHash(), CustomEqual() );

    unordered_map.insert( std::make_pair( CustomKey( "0", 0 ), "element 0" ) );
    unordered_map.insert( std::make_pair( CustomKey( "1", 1 ), "element 1" ) );

    auto it = unordered_map.find( CustomKey( "0", 0 ) );
    BOOST_REQUIRE( it != unordered_map.end() );
    BOOST_CHECK( it->second == "element 0" );
}

BOOST_AUTO_TEST_CASE( BitsetTest )
{
    std::bitset< 3 >    bitset( "100" );

    BOOST_CHECK( bitset.count() == 1 );

    BOOST_CHECK( ! bitset.test( 0 ) );
    BOOST_CHECK( ! bitset.test( 1 ) );
    BOOST_CHECK( bitset.test( 2 ) );

    bitset[ 0 ] = 1;
    BOOST_CHECK( bitset.test( 0 ) );
}

BOOST_AUTO_TEST_CASE( SortingTest )
{
    std::list< unsigned > l = { 5, 2, 6, 9 };
    /*std::sort( std::begin( l ), std::end( l ) );*/ // Can't compile, sort need random it access, list is always ordered
    // Linked lists can be sorted in O( n log n ) using Mergesort
    // Interestingly, since linked lists already have the appropriate structure, sorting a linked list with Mergesort only requires O(1) extra space.
    l.sort();

    std::vector< unsigned > v = { 5, 2, 6, 9 };
    std::sort( std::begin( v ), std::end( v ) );
}

BOOST_AUTO_TEST_SUITE_END() // Container
