//--------------------------------------------------------------------------------
// (C) Copyright 2014-2015 Stephane Molina, All rights reserved.
// See https://github.com/Dllieu for updates, documentation, and revision history.
//--------------------------------------------------------------------------------
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_SUITE( FeatureTestSuite )

BOOST_AUTO_TEST_CASE( DeclTypeTest )
{
    {
        int a = 0;
        // if parenthesis expression
        //   type: T& if lvalue
        //   type: T&& if rvalue
        /* int& */ decltype( ( a ) ) b = a;
        ++b;
        BOOST_CHECK( a == b && b == 1 );
    }
    {
        int a = 0;
        // if not a parenthesis expression
        //   type: T
        /* int */ decltype( a ) b = a;
        ++b;
        BOOST_CHECK( a + 1 == b && b == 1 );
    }
}

namespace
{
    struct VBase { virtual ~VBase() = default; };
    struct VDerived : public VBase { virtual ~VDerived() = default; };

    // Could find a way to generate it automatically
    enum class ClassHierarchy
    {
        Base,
        Derived,
        DerivedLeft,
        DerivedRight,
    };

    struct Base
    {
        Base( ClassHierarchy classHierarchy = ClassHierarchy::Base ) : baseId( classHierarchy ) {}
        static bool   classof( const Base& ) { return true; }
        ClassHierarchy baseId;
    };

// Always try overload classof( const REAL_CLASS& ) first (i.e. better match)
#define IMPLEMENT_DERIVED( NAME, DIRECT_BASE_CLASS ) \
    struct NAME : public DIRECT_BASE_CLASS \
    { \
        NAME( ClassHierarchy classHierarchy = ClassHierarchy::NAME ) : DIRECT_BASE_CLASS( classHierarchy ) {} \
        static bool   classof( const NAME& ) { return true; } \
        static bool   classof( const Base& b ) { return b.baseId == ClassHierarchy::##NAME; } \
    }

    IMPLEMENT_DERIVED( Derived,         Base );
    IMPLEMENT_DERIVED( DerivedLeft,     Derived );
    IMPLEMENT_DERIVED( DerivedRight,    Derived );

#undef IMPLEMENT_DERIVED

    template < typename TO, typename FROM >
    bool  is_a( const FROM& f ) { return TO::classof( f ); }
}

// About dynamic_cast && type_id
// - Big picture of RTTI:
//
// struct S : public B {
//      virtual void f();
//      virtual void g();
//      static vtable_s*  vtable_s_;   ---->  point to .rdata (read data only) memory
// };
//
// struct vtable_s {
//      RTTIObjectLocator_S  rttiInfosS_; // only used for RTTI (i.e. dynamic_cast or type_id), generally 40bytes
//      void (*f)()          ptrToF_;  ----> point to .text memory to point to the correct method (i.e. S::f()) // always used when indirection, unlikely to be inlined, likely to cache miss if not frequent
//      void (*g)()          ptrToG_;  ||||
// };
//
// // Most of the underlying struct have more informations such as size_t attributes / signatures / offset / ... (sizeof is bigger than it seems)
// struct RTTIObjectLocator_S {
//      RuntimeTypeDescriptor*        descriptor; ----> .rdata to class that can provide a name (most likely const char*) // used by type_id
//      RuntimeHierarchy_S*           hierarchyS; ----> .rdata // used by dynamic_cast (more costly as it loop over all hierarchy until it find one that match)
//      //... other internal attributes
// };
//
// struct RuntimeHierarchy_S {
//      size_t                      numberBaseClass;
//      RuntimeBaseClassDescriptor* classDescriptors; // all the descriptor for all the base class
//      //... other internal attributes
// };
//
// struct RuntimeBaseClassDescriptor {
//      RuntimeTypeDescriptor*        descriptor; ----> .rdata to class that can provide a name (most likely const char*) // will be used by dynamic_cast, if match then "return" __PMD
//      _PMD                          where;      ----> .text where the real struct is // usefull for dynamic_cast
//      //... other internal attributes
// };
//
// gcc can disable rtti : -fno-rtti
BOOST_AUTO_TEST_CASE( RTTITest )
{
    std::unique_ptr< VBase > base = std::make_unique< VDerived >();
    BOOST_CHECK( dynamic_cast< VDerived* >( base.get() ) != nullptr ); // need to loop over the hierarchy until we find a match

    Base b;
    Derived d;
    DerivedLeft dl;
    DerivedRight dr;

    BOOST_CHECK( typeid( b ) == typeid( Base ) ); // exact type, based on the direct RTTIDescriptor (no loop on hierarchy needed)
    BOOST_CHECK( typeid( d ) != typeid( Base ) );

    BOOST_CHECK( is_a< Base >( b ) );
    BOOST_CHECK( is_a< Base >( d ) );
    BOOST_CHECK( is_a< Base >( dr ) );
    BOOST_CHECK( is_a< Derived >( dr ) );
    BOOST_CHECK( ! is_a< DerivedLeft >( dr ) );
    BOOST_CHECK( ! is_a< DerivedLeft >( d ) );
}

BOOST_AUTO_TEST_SUITE_END() // FeatureTestSuite
