#ifndef __VECTORGROWTHPOLICY_H__
#define __VECTORGROWTHPOLICY_H__

#include <vector>

namespace containers
{
    struct VectorGrowthPolicyStd
    {
        template < typename T > static void grow( std::vector< T >& ) {}
        template < typename T > static void shrink( std::vector< T >& ) {}
        template < typename T > static void clear( std::vector< T >& v )
        {
            // instead of doing clear() then shrink_to_fit() (Requests the container to reduce its capacity to fit its size)
            return std::vector< T >().swap( v );
        }
    };

    template < std::size_t SIZE_T_INCREMENT, std::size_t SIZE_T_MAX_SIZE >
    struct VectorGrowthPolicyIncremental
    {
        template < typename T > static void grow( std::vector< T >& v )
        {
            auto size = v.size();
            if ( v.capacity() == size )
                v.reserve( std::min( size + SIZE_T_INCREMENT, SIZE_T_MAX_SIZE ) );
        }

        template < typename T > static void shrink( std::vector< T >& v )
        {
            if ( v.size() - v.capacity() < SIZE_T_INCREMENT )
                std::vector< T >( v ).swap( v );
        }

        template < typename T > static void clear( std::vector< T >& v )
        {
            static_assert( SIZE_T_INCREMENT > 0 );
            static_assert( SIZE_T_INCREMENT <= SIZE_T_MAX_SIZE );
            return std::vector< T >().swap( v );
        }
    };

    struct VectorGrowthPolicyIncrementalByOne : VectorGrowthPolicyIncremental< 1, static_cast< std::size_t >( -1 ) /* compile time : std::numeric_limits< std::size_t >::min() */ >
    {
        // NOTHING
    };
}

#endif /* !__VECTORGROWTHPOLICY_H__ */
