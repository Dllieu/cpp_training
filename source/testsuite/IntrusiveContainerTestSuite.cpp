//--------------------------------------------------------------------------------
// (C) Copyright 2014-2015 Stephane Molina, All rights reserved.
// See https://github.com/Dllieu for updates, documentation, and revision history.
//--------------------------------------------------------------------------------
#include <boost/test/unit_test.hpp>
#include <boost/intrusive/list.hpp>

BOOST_AUTO_TEST_SUITE( IntrusiveContainerTestSuite )

namespace
{
    struct NonIntrusiveClass
    {
        int someData;
    };
}

// About Instrusive vs Non-instrusive containers
//
// - A non-intrusive container has some limitations:
//   - An object can only belong to one container : If you want to share an object between two containers, you either have to store multiple copies of those objects or you need to use containers of pointers : std::list<Object*>.
//   - The use of dynamic allocation to create copies of passed values can be a performance and size bottleneck in some applications.
//     Normally, dynamic allocation imposes a size overhead for each allocation to store bookkeeping information and a synchronization to protected concurrent allocation from different threads.
//
// - Intrusive containers have some important advantages :
//   - Operating with intrusive containers doesn't invoke any memory management at all. The time and size overhead associated with dynamic memory can be minimized.
//   - Iterating an Intrusive container needs less memory accesses than the semantically equivalent container of pointers : iteration is faster.
//   - Intrusive containers offer better exception guarantees than non-intrusive containers.In some situations intrusive containers offer a no - throw guarantee that can't be achieved with non-intrusive containers.
//   - The computation of an iterator to an element from a pointer or reference to that element is a constant time operation( computing the position of T* in a std::list<T*> has linear complexity ).
//   - Intrusive containers offer predictability when inserting and erasing objects since no memory management is done with intrusive containers.
//     Memory management usually is not a predictable operation so complexity guarantees from non-intrusive containers are looser than the guarantees offered by intrusive containers.
//
// Issue                                                            | Intrusive         | Non-Intrusive
// ------------------------------------------------------------------------------------------------------------------------------
// Memory Management                                                | External          | Internal through allocator
// Insertion/Erasure time                                           | Faster            | Slower
// Memory locality                                                  | Better            | Worse
// Can hold non-copyable and non-movable objects by value           | Yes               | No
// Exception guarantees                                             | Better            | Worse
// Computation of iterator from value                               | Constant          | Non-constant
// Insertion/erasure predictability                                 | High              | Low
// Memory use                                                       | Minimal           | More than minimal
// Insert objects by value retaining polymorphic behavior           | Yes               | No (slicing)
// User must modify the definition of the values to insert          | Yes               | No
// Containers are copyable                                          | No                | Yes
// Inserted object's lifetime managed by                            | User              | Container
// Container invariants can be broken without using the container   | More likely       | Less likely
// Thread-safety analysis                                           | Harder            | Easier
BOOST_AUTO_TEST_CASE( NonIntrusiveListTest )
{
    // The main difference between intrusive containers and non-intrusive containers is that in C++ non-intrusive containers store copies of values passed by the user.
    // Containers use the Allocator template parameter to allocate the stored values
    std::list< NonIntrusiveClass > list;
    // To store the newly allocated copy of myclass, the container needs additional data : std::list usually allocates nodes that contain pointers to the next and previous node and the value itself. Something similar to
    // struct list_node
    // {
    //     list_node*   previous;
    //     list_node*   next;
    //     ClassToStore value;
    // };
    NonIntrusiveClass classToStore;
    list.push_back( classToStore ); // by copy
    BOOST_CHECK( &list.back() != &classToStore );
}

namespace
{
    // Two way to declare 'hook' for the boost intrusive list
    struct BaseHookElement : public boost::intrusive::list_base_hook<>
    {
        explicit BaseHookElement(int i) : someData( i ) {}
        BaseHookElement( BaseHookElement&& ) = default; // for vector::emplace_back

        BaseHookElement( const BaseHookElement& ) = delete;
        BaseHookElement& operator=( const BaseHookElement& ) = delete;
        BaseHookElement& operator=( BaseHookElement&& ) = delete;

        int     someData;
    };
    using BaseHookList = boost::intrusive::list< BaseHookElement >;

    struct MemberHookElement
    {
        explicit MemberHookElement(int i) : someData( i ) {}
        MemberHookElement( MemberHookElement&& ) = default; // for vector::emplace_back

        MemberHookElement( const MemberHookElement& ) = delete;
        MemberHookElement& operator=( const MemberHookElement& ) = delete;
        MemberHookElement& operator=( MemberHookElement&& ) = delete;

        boost::intrusive::list_member_hook<>    memberHook;
        int                                     someData;
    };
    using MemberHookList = boost::intrusive::list< MemberHookElement,
                                                   boost::intrusive::member_hook< MemberHookElement, boost::intrusive::list_member_hook<>, &MemberHookElement::memberHook > >;

    template < typename ListType, typename ElementType >
    bool    test_intrusive_list()
    {
        std::vector< ElementType > storedElements; // store elements but don't manipulate them directly
        // Create all the elements first
        for ( auto i = 0; i < 10; ++i )
            storedElements.emplace_back( ElementType( i ) );

        // storedElements must outlive list
        ListType list;
        for ( auto it = storedElements.begin(); it != storedElements.end(); ++it )
            list.push_back( *it );

        return std::is_sorted( list.begin(), list.end(), [] ( auto& a, auto& b ) { return a.someData < b.someData; } );
    }
}

// An intrusive container does not store copies of passed objects, but it stores the objects themselves. The additional data needed to insert the object in the container must be provided by the object itself.
// For example, to insert MyClass in an intrusive container that implements a linked list, MyClass must contain the needed next and previous pointers
BOOST_AUTO_TEST_CASE( IntrusiveListTest )
{
    // list is a doubly linked list. The memory overhead it imposes is 2 pointers per node. An empty, non constant-time size list also has the size of 2 pointers.
    // list has many more constant-time operations than slist and provides a bidirectional iterator. It is recommended to use list instead of slist if the size overhead is acceptable
    BOOST_CHECK( ( test_intrusive_list< BaseHookList, BaseHookElement >() ) );
    BOOST_CHECK( ( test_intrusive_list< MemberHookList, MemberHookElement >() ) );
}

BOOST_AUTO_TEST_SUITE_END() // IntrusiveContainerTestSuite
