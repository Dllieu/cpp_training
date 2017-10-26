//--------------------------------------------------------------------------------
// (C) Copyright 2014-2015 Stephane Molina, All rights reserved.
// See https://github.com/Dllieu for updates, documentation, and revision history.
//--------------------------------------------------------------------------------
#include <boost/test/unit_test.hpp>
#include <iostream>

BOOST_AUTO_TEST_SUITE( AllocatorTestSuite )

namespace
{
    //// Skeleton (howardhinnant.github.io)

    //template <class T>
    //class allocator
    //{
    //public:
    //    using value_type = T;
    //
    //    //     using pointer       = value_type*;
    //    //     using const_pointer = typename std::pointer_traits<pointer>::template
    //    //                                                     rebind<value_type const>;
    //    //     using void_pointer       = typename std::pointer_traits<pointer>::template
    //    //                                                           rebind<void>;
    //    //     using const_void_pointer = typename std::pointer_traits<pointer>::template
    //    //                                                           rebind<const void>;
    //
    //    //     using difference_type = typename std::pointer_traits<pointer>::difference_type;
    //    //     using size_type       = std::make_unsigned_t<difference_type>;
    //
    //    //     template <class U> struct rebind {typedef allocator<U> other;};
    //
    //    allocator() noexcept {}  // not required, unless used
    //    template <class U> allocator( allocator<U> const& ) noexcept {}
    //
    //    value_type* allocate( std::size_t n )
    //    {
    //        return static_cast<value_type*>( ::operator new ( n*sizeof( value_type ) ) );
    //    }
    //
    //    void deallocate( value_type* p, std::size_t ) noexcept
    //    {
    //        ::operator delete( p );
    //    }
    //
    //    //     value_type*
    //    //     allocate(std::size_t n, const_void_pointer)
    //    //     {
    //    //         return allocate(n);
    //    //     }
    //
    //    //     template <class U, class ...Args>
    //    //     void
    //    //     construct(U* p, Args&& ...args)
    //    //     {
    //    //         ::new(p) U(std::forward<Args>(args)...);
    //    //     }
    //
    //    //     template <class U>
    //    //     void
    //    //     destroy(U* p) noexcept
    //    //     {
    //    //         p->~U();
    //    //     }
    //
    //    //     std::size_t
    //    //     max_size() const noexcept
    //    //     {
    //    //         return std::numeric_limits<size_type>::max();
    //    //     }
    //
    //    //     allocator
    //    //     select_on_container_copy_construction() const
    //    //     {
    //    //         return *this;
    //    //     }
    //
    //    //     using propagate_on_container_copy_assignment = std::false_type;
    //    //     using propagate_on_container_move_assignment = std::false_type;
    //    //     using propagate_on_container_swap            = std::false_type;
    //    //     using is_always_equal                        = std::is_empty<allocator>;
    //};
    //
    //template <class T, class U>
    //bool operator==( allocator<T> const&, allocator<U> const& ) noexcept
    //{
    //    return true;
    //}
    //
    //template <class T, class U>
    //bool operator!=( allocator<T> const& x, allocator<U> const& y ) noexcept
    //{
    //    return !( x == y );
    //}


    template < std::size_t N, std::size_t alignment = alignof( std::max_align_t ) >
    class arena
    {
        alignas( alignment ) char   buf_[ N ];
        char*                       ptr_;

    public:
        arena() noexcept : ptr_( buf_ ) {}
        arena( const arena& ) = delete;
        arena( arena&& ) = delete;
        arena& operator=( const arena& ) = delete;
        arena& operator=( arena&& ) = delete;

        template < std::size_t ReqAlign > char* allocate( std::size_t n );
        void deallocate( char* p, std::size_t n ) noexcept;

        static constexpr std::size_t size() noexcept { return N; }
        std::size_t used() const noexcept { return static_cast<std::size_t>( ptr_ - buf_ ); }
        void reset() noexcept { ptr_ = buf_; }

    private:
        // result % aligment == 0
        static std::size_t align_up( std::size_t n ) noexcept
        {
            static_assert( alignment == 1 || alignment != 0 && ( alignment & 1 ) == 0, "Wrong alignment" );
            return n + ( alignment - 1 ) & ~( alignment - 1 );
        }

        bool pointer_in_buffer( char* p ) noexcept
        {
            return buf_ <= p && p <= buf_ + N;
        }
    };

    template < std::size_t N, std::size_t alignment >
    template < std::size_t ReqAlign >
    char* arena<N, alignment>::allocate( std::size_t n )
    {
        std::cout << "alignment: " << alignment << std::endl;
        std::cout << "ReqAlign: " << ReqAlign << std::endl;
        static_assert( ReqAlign <= alignment, "alignment is too small for this arena" );
        assert( pointer_in_buffer( ptr_ ) && "short_alloc has outlived arena" );
        auto const aligned_n = align_up( n );
        if ( size_t(buf_ + N - ptr_) >= aligned_n )
        {
            char* r = ptr_;
            ptr_ += aligned_n;
            return r;
        }
        return static_cast<char*>( ::operator new( n ) );
    }

    template <std::size_t N, std::size_t alignment>
    void  arena<N, alignment>::deallocate( char* p, std::size_t n ) noexcept
    {
        assert( pointer_in_buffer( ptr_ ) && "short_alloc has outlived arena" );
        if ( pointer_in_buffer( p ) )
        {
            n = align_up( n );
            if ( p + n == ptr_ )
                ptr_ = p;
        }
        else
            ::operator delete( p );
    }

    /*
    * @brief This allocator will dole out memory in units of alignment which defaults to alignof(std::max_align_t). This is the same alignment which malloc and new hand out memory.
    *        If there is room on the stack supplied buffer (you specify the buffer size), the memory will be allocated on the stack, else it will ask new for the memory.
    *        If memory is tight (when is it not!), you can reduce the alignment requirements via the defaulted third template argument of short_alloc.
    *        If you do this, and then the container attempts to allocate memory with alignment requirements greater than you have specified, a compile-time error will result.
    *        Thus you can safely experiment with reducing the alignment requirement, without having to know implementation details of the container you're using short_alloc with.
    */
    template <class T, std::size_t N, std::size_t Align = alignof( std::max_align_t )>
    class short_alloc
    {
    public:
        using value_type = T;
        static const auto constexpr alignment = Align;
        static const auto constexpr size = N;
        using arena_type = arena<size, alignment>;

    private:
        arena_type& a_;

    public:
        short_alloc( const short_alloc& ) = default;
        short_alloc& operator=( const short_alloc& ) = delete;

        short_alloc( arena_type& a ) noexcept : a_( a ) {}
        template <typename U>
        short_alloc( const short_alloc<U, N, alignment>& a ) noexcept
            : a_( a.a_ )
        {}

        template <typename _Up> struct rebind { using other = short_alloc<_Up, N, alignment>; };

        T* allocate( std::size_t n )
        {
            return reinterpret_cast<T*>( a_.template allocate<alignof( T )>( n * sizeof( T ) ) );
        }
        void deallocate( T* p, std::size_t n ) noexcept
        {
            a_.deallocate( reinterpret_cast<char*>( p ), n*sizeof( T ) );
        }

        template < typename T1, std::size_t N1, std::size_t A1, typename U, std::size_t M, std::size_t A2 >
        friend bool operator==( const short_alloc<T1, N1, A1>& x, const short_alloc<U, M, A2>& y ) noexcept;

        template <typename U, std::size_t M, std::size_t A> friend class short_alloc;
    };

    template <typename T, std::size_t N, std::size_t A1, typename U, std::size_t M, std::size_t A2>
    inline bool operator==( const short_alloc<T, N, A1>& x, const short_alloc<U, M, A2>& y ) noexcept
    {
        return N == M && A1 == A2 && &x.a_ == &y.a_;
    }

    template <class T, std::size_t N, std::size_t A1, class U, std::size_t M, std::size_t A2>
    inline bool operator!=( const short_alloc<T, N, A1>& x, const short_alloc<U, M, A2>& y ) noexcept
    {
        return !( x == y );
    }

    // Create a vector<T> template with a small buffer of 200 bytes.
    //   Note for vector it is possible to reduce the alignment requirements
    //   down to alignof(T) because vector doesn't allocate anything but T's.
    //   And if we're wrong about that guess, it is a comple-time error, not
    //   a run time error.
    template <class T, std::size_t BufSize = 200>
    using SmallVector = std::vector<T, short_alloc< T, BufSize, alignof( T ) < 8 ? 8 : alignof( T ) > >;
}

BOOST_AUTO_TEST_CASE( CustomAllocatorTest )
{
    // Create the stack-based arena from which to allocate
    SmallVector<int>::allocator_type::arena_type a;
    // Create the vector which uses that arena.oc<T, BufSize, alignof( T )>>;

    SmallVector<int> v{ a };
    // Exercise the vector and note that new/delete are not getting called.
    v.push_back( 1 );

    v.shrink_to_fit();
    v.push_back( 2 );
    v.push_back( 3 );
    v.push_back( 4 );
    // Yes, the correct values are actually in the vector
    for ( auto i : v )
        std::cout << i << ' ';
    std::cout << '\n';
}

BOOST_AUTO_TEST_SUITE_END() // ! AllocatorTestSuite
