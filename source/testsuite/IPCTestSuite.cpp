//--------------------------------------------------------------------------------
// (C) Copyright 2014-2015 Stephane Molina, All rights reserved.
// See https://github.com/Dllieu for updates, documentation, and revision history.
//--------------------------------------------------------------------------------
#include <boost/test/unit_test.hpp>

//#define IGNORE_TEST
#if defined(_WIN32) || defined(__WIN32__) || defined(WIN32)
    #include <boost/interprocess/windows_shared_memory.hpp>
#endif
#include <boost/interprocess/sync/interprocess_mutex.hpp>
#include <boost/interprocess/sync/scoped_lock.hpp>
#include <boost/interprocess/mapped_region.hpp>
#include <thread>
#include <iostream>

namespace bip = boost::interprocess;

BOOST_AUTO_TEST_SUITE( IPCTestSuite )

namespace
{
    constexpr const char* SharedMemoryName = "IPCTestSuiteSharedMemory";
    constexpr const char* DataToShare = "2109w3lnk";

    // Constant expressions can be evaluated during translation
    constexpr size_t    length(const char* str)
    {
        return *str ? 1 + length(str + 1) : 0;
    }
    constexpr size_t DataToShareSize = length(DataToShare);

    struct DataShared
    {
        bip::interprocess_mutex                 mutex;
        std::array< char, DataToShareSize >     data;
        bool                                    mustStop;
    };
}

#if !defined(IGNORE_TEST) && (defined(_WIN32) || defined(__WIN32__) || defined(WIN32))

// General
// - Shared memory is the fastest interprocess communication mechanism.
// - The operating system maps a memory segment in the address space of several processes, so that several processes can read and write in that memory segment without calling operating system functions.
// - To use shared memory, we have to perform 2 basic steps:
//   * Request to the operating system a memory segment that can be shared between processes.
//   * Associate a part of that memory or the whole memory with the address space of the calling process.

// On Windows,
// - The windows shared memory is created backed by the pagefile and it's automatically destroyed when the last process attached to the shared memory is destroyed.
//
// - Because of this reason, there is no effective way to simulate kernel or filesystem persistence using native windows shared memory -> Boost.Interprocess emulates shared memory using memory mapped files
//   -> that's why there's an explicit windows_shared_memory for a real shared memory on windows (shared_memory_object otherwise for other OS, on windows the underlying is a memory mapped files)
//
// - Sharing memory between services and user applications is also different. To share memory between services and user applications the name of the shared memory must start with the global namespace prefix "Global\\".

// On Linux,
// - A shared_memory is persistent, the user must explicitly destroy it
// - When a shared memory is created, its size is 0, the user must use truncate (with bpi) to set the size
BOOST_AUTO_TEST_CASE( WindowsSharedMemoryCreateTest )
{
    // 1 - Request segment
    bip::windows_shared_memory sharedMemory( bip::create_only, SharedMemoryName, bip::read_write, sizeof( DataShared ) );

    // 2 - Map the whole shared memory in this process
    bip::mapped_region region( sharedMemory, bip::read_write );

    auto dataShared = new ( region.get_address() ) DataShared;

    // unsafe until here
    {
        bip::scoped_lock<decltype( dataShared->mutex )> lock( dataShared->mutex );
        std::memcpy( dataShared->data.data(), DataToShare, DataToShareSize );
    }

    for (;;)
    {
        {
            bip::scoped_lock<decltype( dataShared->mutex)> lock( dataShared->mutex );
            if ( dataShared->mustStop )
                break;
        }
        std::this_thread::yield();
    }

    std::cout << "Received stop" << std::endl;
    BOOST_CHECK( true );

    // windows_shared_memory is destroyed when the last attached process dies... (no need RTTI for windows)
}

BOOST_AUTO_TEST_CASE( WindowsSharedMemoryOpenTest )
{
    bip::windows_shared_memory sharedMemory( bip::open_only, SharedMemoryName, bip::read_write );
    bip::mapped_region region( sharedMemory, bip::read_write );

    auto dataShared = static_cast<DataShared*>( region.get_address() );
    BOOST_REQUIRE( strncmp( dataShared->data.data(), DataToShare, DataToShareSize ) == 0 );

    std::cout << "Prepare sending stop" << std::endl;
    bip::scoped_lock<decltype( dataShared->mutex )> lock( dataShared->mutex );
    dataShared->mustStop = true;

    BOOST_CHECK(true);
}

#endif

BOOST_AUTO_TEST_SUITE_END() // ! IPCTestSuite
