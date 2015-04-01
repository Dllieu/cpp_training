//--------------------------------------------------------------------------------
// (C) Copyright 2014-2015 Stephane Molina, All rights reserved.
// See https://github.com/Dllieu for updates, documentation, and revision history.
//--------------------------------------------------------------------------------
#include <boost/test/unit_test.hpp>
#include <atomic>
#include <thread>

BOOST_AUTO_TEST_SUITE( MemoryOrderingTestSuite )
// enum memory_order {
//     memory_order_relaxed,
//     memory_order_consume,
//     memory_order_acquire,
//     memory_order_release,
//     memory_order_acq_rel,
//     memory_order_seq_cst
// };
// relaxed is the weakest, memory_order_seq_cst the strongest, you can think of memory_order_seq_cst the most upgraded memory order (which can have slight overhead compared to downgraded version)
// the idea is to always use memory_order_seq_cst, but if applicable, use downgraded memory order
// some memory order are not yet implemented fully in certain compiler, thus forwarding to the upgraded version of the corresponding memory model (e.g. memory_order_consume treated as memory_order_acquire)

// https://jfdube.wordpress.com/2012/03/08/understanding-memory-ordering/
// Behind the hood, the compiler, the processor and the cache might reorder memory operations (reads and writes) for many reasons.
// This is normally not a problem on programs running on single-core machines.
// On the other hand, multi-threaded programs running on multi-cores machines can suffer from that: the sequence in which reads and writes operations are performed by
// the processor can be different from what the order of execution of those operations are from the programmer’s point of view.

// Memory barriers are a set of processor instructions used to force pending memory operations to complete before continuing.
// There are three types of barrier semantics: acquire, release and fence barriers.

// Acquire Semantics
// Whenever you use atomic operations to gain access to some data, your code must make sure that other processors sees the lock before any other changes that will be made. This is what we call acquire semantics, because the code is trying to acquire ownership of some data. This is also referred as read barriers or import barriers.
//   operation 1
//   operation 2
// <-operation 3-Acquire-> 3 is visible before 4-5
//   operation 4
//   operation 5

// Release Semantics
// On the other hand, if you use atomic operations to release recently modified data, your code must make sure that the new data is visible before releasing it. This was exactly the case with the previous string-table example. This is what we call release semantics, because the code is trying to release ownership of some data. This is also referred as write barriers or export barriers.
//   operation 1
//   operation 2
// <-operation 3-Release-> 1-2 are visible before 3
//   operation 4
//   operation 5

// Fence Semantics
// A fence semantics combines both acquire and release semantics behavior.
//   operation 1
//   operation 2
// <-operation 3-Fence-> 1-2 are visible before 3, 3 is visible before 4-5
//   operation 4
//   operation 5

BOOST_AUTO_TEST_CASE( SequentialConsistency )
{
    // If you omit the optional std::memory_order argument on all atomic library functions, the default value is std::memory_order_seq_cst, which turns all atomic variables into sequentially consistent atomics
    // With memory_order_seq_cst atomics, the whole algorithm is guaranteed to appear sequentially consistent (executed same order as in the code)(impossible to experience memory reordering) as long as there are no data races.
    
    std::atomic< bool > ready( false );
    std::atomic< int > data( 0 );

    std::thread t( [&]
    {
        data.store( 1/*, std::memory_order_seq_cst*/ ); // can't be reordered
        ready.store( true ); // can't be reordered
    } );

    if ( ready.load() )
        BOOST_CHECK( data.load() == 1 );

    t.join();

    BOOST_CHECK( ready.load() );
    BOOST_CHECK( data.load() == 1 );

    // http://herbsutter.com/2012/08/02/strong-and-weak-hardware-memory-models/
    // In this example the fact that we use std::memory_order_seq_cst everywhere might incure an overhead as we might force the use of a memory barrier which could be not needed for certain processor (e.g. x86 which have a strong memory model)
    // Acquire Release semantic is more adapted in this case, although, in this test, processor is clever enough to only add one memory barrier between data and ready as there's only two instructions
    // But we can think as when using memory_order_seq_cst, at worst case, we could have two memory barrier which surround the call of the atomic object
}

BOOST_AUTO_TEST_CASE( AcquireRelease )
{
    std::atomic< bool > ready( false );
    std::atomic< int > data( 0 );

    // Acquire Release semantic to establish Synchronizes-with relationship (ready -> data)
    std::thread t( [&]
    {
        data.store( 1, std::memory_order_relaxed ); // can be reordered
        ready.store( true, std::memory_order_release ); // make sure that this statement is executed after the code above (i.e. might potentially make a memory barrier before this instruction)
    } );

    if ( ready.load( std::memory_order_acquire ) ) // make sure that this statement is executed before the code below (i.e. might potentially make a memory barrier after this instruction)
        BOOST_CHECK( data.load( std::memory_order_relaxed ) == 1 ); // can be reordered

    t.join();

    BOOST_CHECK( ready.load( std::memory_order_acquire ) ); // make sure that this statement is executed before the code below
    BOOST_CHECK( data.load( std::memory_order_relaxed ) == 1 ); // can be reordered
}

BOOST_AUTO_TEST_SUITE_END() // MemoryOrderingTestSuite
