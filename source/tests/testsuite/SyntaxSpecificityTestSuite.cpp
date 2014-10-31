#include <boost/test/unit_test.hpp>
#include <typeinfo>

// Operator that can't be specialized
// sizeof . .* .-> :: ?:

namespace
{
    // if referenced it won't link since no header included define this variable
    // the place where this variable is defined is were the memory is allocated (i.e. not there: declaration (here) != definition)
    extern int externValue;
    // will still link, not so external, since it's definition is in place...
    extern int externValueWithDeclaration = 0;

    // A declared variable can be Zero Initialized, Value Initialized or Default Initialized.
    // The C++03 Standard 8.5/5 aptly defines each:
    // To zero-initialize an object of type T means:
    // - if T is a scalar type (3.9), the object is set to the value of 0 (zero) converted to T;
    // - if T is a non-union class type, each nonstatic data member and each base-class subobject
    // is zero-initialized;
    // - if T is a union type, the object's first named data member is zero-initialized;
    // - if T is an array type, each element is zero-initialized;
    // - if T is a reference type, no initialization is performed.
    // 
    // To default-initialize an object of type T means:
    // - if T is a non-POD class type (clause 9), the default constructor for T is called (and the initialization is ill-formed if T has no accessible default constructor);
    // - if T is an array type, each element is default-initialized;
    // - otherwise, the object is zero-initialized.
    // 
    // To value-initialize an object of type T means:
    // - if T is a class type (clause 9) with a user-declared constructor (12.1), then the default constructor for T is called (and the initialization is ill-formed if T has no accessible default constructor);
    // - if T is a non-union class type without a user-declared constructor, then every non-static data member and base-class component of T is value-initialized;
    // - if T is an array type, then each element is value-initialized;
    // - otherwise, the object is zero-initialized

    // An aggregate is an array or a class with no user-declared constructors, no private or protected non-static data members, no base classes, and no virtual functions
    class Aggregate // POD type
    {
    public:
      char          c;
      static double sd;
      double        d;
      int           i;
    };

    const int x = 3;

#pragma warning( push )
#pragma warning( disable : 4700 )
    void        foo1()
    {
        // a.c = 72, a::sd skipped because static (implicitly set to 0 in this situation), a.d = 1.2
        // a.i = 0 (If there are fewer initializers in the list than there are members in the aggregate, then each member not explicitly initialized shall be value-initialized)
        Aggregate a = { 72, 1.2 };

        {
            int x = x; // undefined behavior, it won't use the global x because the affectation is after the definition
        }
        {
            int x[x] = { 1, 2, 3 }; // correct, x[3], use global x because x[] is not defined yet
            std::cout << sizeof(x) / sizeof(int) << std::endl; // sizeof(int) * 3 / sizeof(int)
        }
        {
            enum bar
            {
                x = x // correct, it will use the global because the enum is not defined yet at this point
            };
        }

        if ( int i = 5 )
            i = 2;
        else
            i = 3;
    }
#pragma warning( pop )
}

namespace
{
    struct Base
    {
        virtual ~Base() {}

        virtual int     f( int i = 1 )
        {
            return i * 2;
        }
    };

    struct Derived : public Base
    {
        ~Derived() {}

        int     f( int i = 2 )
        {
            return i * 3;
        }
    };

    void     defaultValueInheritanceExample()
    {
        Base* b = new Derived;

        // A virtual function call uses the default arguments in the declaration of the virtual function determined by the static type of the pointer
        // or reference denoting the object. An overriding function in a derived class does not acquire default arguments from the function it overrides
        std::cout << b->f() << std::endl; // will display '1' * 3 == 3 (take the default parameter of the base)
        delete b;
        return void();
    }
}

namespace
{
    namespace imbriquedNamespace
    {
        struct Bar
        {
        };

        struct Foo
        {
        };

        void    bar( const Bar& b )
        {
            // Remove constness
            {
                Bar* ncB1 = (Bar*)&b;
                Bar* ncB2 = const_cast< Bar* >( &b );
            }

            // int -> enum
            {
                enum color
                {
                    Black = 0,
                    Pink
                };

                color c = (color)1;
                c = static_cast< color >( 1 );
            }

            // ptr <-> int
            {
                int p = (int)&b;
                p = reinterpret_cast< int >( &b );

                Bar* bp = (Bar*)p;
                bp = reinterpret_cast< Bar* >( p );
            }

            // cast to unrelated cast
            {
                // Only reinterpret_cast can be used to convert a pointer to an object to a pointer to an unrelated object type
                // dynamic_cast can't be used because the argument is not polymorphic (compiler error dependent)
                // (C-cast cannot be used either)
                const Foo* f = reinterpret_cast< const Foo* >( &b );
            }

            // about dynamic_cast
            // cast which handle "cleanly" polymorphism, You can cast a pointer or reference to any polymorphic type to any other class type
            // (a polymorphic type has at least one virtual function, declared or inherited)
            // can use it for casting downward and upward or even up another chain
            // can also cast null pointers even between pointers to unrelated classes, and can also cast pointers of any type to void pointers
            // if cannot cast a ptr -> return 0
            // if cannot cast a ref -> throw std::bad_cast exception

            // about static_cast
            // can perform conversions between pointers to related classes, not only from the derived class to its base (use dynamic_cast)
            // can also from a base class to its derived (Derived* d = static_cast< Derived* >( &baseClass )  (dynamic_cast would return 0))
            // can also be used to perform any other non-pointer conversion that could also be performed implicitly or convert numeric data types (enum to int, float to int)
            // no run-time type check is made, so static_cast return as if nothing went wrong which could lead to undefined behavior at run time

            // about const_cast
            // modify the constness
            // remove volatile and __unaligned (e.g. "int __unaligned *p;" the compiler assumes that the pointer addresses data that is not aligned, so it have to read it one byte at a time)
        }
    }

    void    foo2()
    {
        imbriquedNamespace::Bar    b;

        // no need to specify the namespace thanks to the argument
        // if a funciton 'bar' with the same parameter was implemented in the current namespace or upper, it will become an ambigous call (compile error)
        bar( b );

        int*    pi = 0;
        int     i = 0;

        // about typeid
        // typeid can be applied to any type or typed expression.
        // If applied to a reference type (lvalue), the type_info returned identifies the referenced type. Any const or volatile qualified type is identified as its unqualified type. 
        // A typedef type is considered the same as its aliased type.
        // When typeid is applied to a reference or dereferenced pointer to an object of a polymorphic class type, it considers its dynamic type (RTTI)
        // The lifetime of the object returned by typeid extends to the end of the program
        // typeid work with null ptr if it's not a derived type
        std::cout << typeid( pi ).name() /* int* */ << std::endl
                  << typeid( i ).name() /* int */ << std::endl;

        try
        {
            Derived* d = 0;
            typeid( *d );
        }
        catch ( std::bad_typeid& )
        {
            // pointer to a polymorphic type which has a null pointer throw when using typeid
        }

        Base* bb = new Derived[5];
        // deleting an array of polymorphic objects using a base class pointer is undefined
        delete [] bb; // Undefined!
    }
}

// (translation unit : object file from this cpp + headers included)
// internal linkage (can be accessed only by this translation unit)
static int x = 1;
// internal linkage
const int y = 2;
// external linkage (things that exist beyond this particular translation unit, accessable through the whole program (e.g. combination of all the translation units))
int w = 3;
// external linkage with value 0 (variables with static storage duration are zero initialized)
// All objects which do not have dynamic storage duration, do not have thread storage duration, and are not local have static storage duration
int ww;
// external linkage
extern const int z = 4;

namespace
{
    struct Foo
    {
        static int m;
        operator int()
        {
            return 123;
        }

        const char* bar( int i = m ) // allowed because it's a static, couldn't have use member attribute if non static
        {
            return "String literals have static storage duration, therefore they can be referenced anywhere in the translation unit, even though it is defined in a function";
        }
    };

    int Foo::m = 22;

    void    otherSpecifics()
    {
        // if either operand is negative, modulo result if compiler implementation defined
        int m = -5 % 9;

         int* x = new int[ 4 ];
         std::cout << x << std::endl;
         std::cout << x + 3 << std::endl;
         // In pointer addition and subtraction if both operands and the result do not point inside the same array (or 1 past the last element of the array) the result is undefined
         std::cout << x + 10 << std::endl; // Undefined!
         delete [] x;

         int y = 0x100;
         y <<= 64; // y value undefined : The result of a shift operation is undefined if the right operand is negative, or greater than or equal to the length in bits of the promoted left operand

         Foo f;
         std::cout << ( 0 ? 1 : f ) << std::endl; // display "123"
         const char* str = f.bar();

         int u;
         u = 1, 2, 3; // u = 1, other sequences are independants

         char c = '2';
         BOOST_CHECK( sizeof( + c ) == sizeof( int ) );
    }
}

namespace
{
    struct A
    {
        virtual void    ambigous() {}
        virtual void    ambigous2() {}
    };

    // make sur that children class inherits members of A only once
    struct B : virtual public A
    {
        virtual void ambigous2() {}
    };

    // make sur that children class inherits members of A only once
    struct C : virtual public A
    {
        virtual void ambigous2() {}
    };

    struct D : public B, public C
    {
        // Virtual functions must have a unique "final overrider" that overrides all other instances of that function in its inheritance heirarchy
        virtual void ambigous2() {}
    };

    void    callVirtualInheritance()
    {
        D   d;
        // works because of the virtual inheritance (since D (B or C) doesn't override it)
        d.ambigous();
        d.ambigous2();
    }
}

namespace
{
    void    labelSpecification()
    {
        int i = 1;

        /*
        Transfer back past an initialized variable with automatic stoage involve destruction of variables with automatic storage duration
        that are in scope at the point transferred from but not at the point transferred to
        */
label:
        // A() -> ~A() -> goto -> A() -> ~A()
        A a;
        if ( i-- )
            goto label;
    }
}
