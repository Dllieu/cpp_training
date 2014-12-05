//--------------------------------------------------------------------------------
// (C) Copyright 2014-2015 Stephane Molina, All rights reserved.
// See https://github.com/Dllieu for updates, documentation, and revision history.
//--------------------------------------------------------------------------------
#include <boost/test/unit_test.hpp>
#include <boost/timer/timer.hpp>
#include <map>

// http://www.codeproject.com/Articles/18389/Fast-C-Delegate-Boost-Function-drop-in-replacement
BOOST_AUTO_TEST_SUITE( FunctionCallTestSuite )

#define FUNCTOR_IMPLEMENTATION( uselessParam ) auto useless = uselessParam;

namespace
{
    typedef /*std::vector< std::string >*/int TypeArgument;
    inline void realImplementation( TypeArgument copiedData )
    {
        FUNCTOR_IMPLEMENTATION( copiedData );
    }

    struct ObjectFunctor
    {
        inline void    operator()( TypeArgument copiedData ) const
        {
            FUNCTOR_IMPLEMENTATION( copiedData );
        }
    };

    template <typename Functor>
    boost::timer::nanosecond_type    timeCalls( Functor f, const std::string& name )
    {
        TypeArgument data;

        boost::timer::auto_cpu_timer t( name + ": %u\n" );
        // kind of useless as it make branch prediction easier + good caching
        for ( auto i = 0; i < 100000; ++i )
            f( data );

        return t.elapsed().user;
    }

    boost::timer::nanosecond_type    dispatchDecreasingCall()
    {
        return 0;
    }

    template <typename T, typename ...Ts>
    boost::timer::nanosecond_type    dispatchDecreasingCall( std::string&& name, T&& t, Ts&&... ts )
    {
        auto result = timeCalls( t, name );
        // Not a great test as the function we are using are easily optimized by the compiler
        //BOOST_CHECK( result >= dispatchDecreasingCall( ts... ) );
        dispatchDecreasingCall( ts... );
        return result;
    }
}

BOOST_AUTO_TEST_CASE( FunctorCall )
{
    // std::bind can't be inlined

    // about boost::function
    // - Function object wrappers will be the size of a struct containing a member function pointer and two data pointers.
    //   The actual size can vary significantly depending on the underlying platform; on 32-bit Mac OS X with GCC, this amounts to 16 bytes,
    //   while it is 32 bytes Windows with Visual C++. Additionally, the function object target may be allocated on the heap,
    //   if it cannot be placed into the small-object buffer in the boost::function object.
    // - Copying function object wrappers may require allocating memory for a copy of the function object target.
    //   The default allocator may be replaced with a faster custom allocator or one may choose to allow the function object wrappers to only store function object targets
    //   by reference (using ref) if the cost of this cloning becomes prohibitive. Small function objects can be stored within the boost::function object itself, improving copying efficiency.
    // - With a properly inlining compiler, an invocation of a function object requires one call through a function pointer.
    //   If the call is to a free function pointer, an additional call must be made to that function pointer (unless the compiler has very powerful interprocedural analysis).

    // The cost of boost::function can be reasonably consistently measured at around 20ns +/- 10 ns on a modern >2GHz platform versus directly inlining the code.
    // However, the performance of your application may benefit from or be disadvantaged by boost::function depending on how your C++ optimiser optimises.
    // Similar to a standard function pointer, differences of order of 10% have been noted to the benefit or disadvantage of using boost::function to call a function
    // that contains a tight loop depending on your compilation circumstances.
    dispatchDecreasingCall( "bind",               std::bind( &realImplementation, std::placeholders::_1 ),
                            "object functor",     ObjectFunctor(),
                            "lambda",             []( TypeArgument copiedData ) { FUNCTOR_IMPLEMENTATION( copiedData ); },
                            "direct call",        &realImplementation );
}

#undef FUNCTOR_IMPLEMENTATION

namespace
{
    class A
    {
    public:
        virtual void    virtualF() { BOOST_CHECK( true ); }
        inline void     f() { BOOST_CHECK( true ); }
    };
}

BOOST_AUTO_TEST_CASE( VirtualMethodCall )
{
    A a;

    // The big cost of virtual functions isn't really the lookup of a function pointer in the vtable (that's usually just a single cycle),
    // but that the indirect jump usually cannot be branch-predicted. This can cause a large pipeline bubble as the processor
    // cannot fetch any instructions until the indirect jump (the call through the function pointer) has retired and a new instruction pointer computed.
    // So, the cost of a virtual function call is much bigger than it might seem from looking at the assembly

    // a virtual function call may cause an instruction cache miss: if you jump to a code address that is not in cache then the whole program comes
    // to a dead halt while the instructions are fetched from main memory. This is always a significant stall: on Xenon, about 650 cycles (by my tests).
    // However this isn't a problem specific to virtual functions because even a direct function call will cause a miss if you jump to instructions
    // that aren't in cache. What matters is whether the function has been run before recently (making it more likely to be in cache),
    // and whether your architecture can predict static (not virtual) branches and fetch those instructions into cache ahead of time.
    boost::timer::nanosecond_type timer = 0;
    {
        boost::timer::auto_cpu_timer t( "virtual function: %u\n" );
        for ( auto i = 0; i < 100000; ++i )
            a.virtualF();
        timer = t.elapsed().user;
    }
    {
        boost::timer::auto_cpu_timer t( "direct method: %u\n" );
        for ( auto i = 0; i < 100000; ++i )
            a.f();
        BOOST_CHECK( timer >= t.elapsed().user );
    }
}

BOOST_AUTO_TEST_SUITE_END() // FunctionCallTestSuite
