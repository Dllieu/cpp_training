//--------------------------------------------------------------------------------
// (C) Copyright 2014-2015 Stephane Molina, All rights reserved.
// See https://github.com/Dllieu for updates, documentation, and revision history.
//--------------------------------------------------------------------------------
#ifndef __SPARSEARRAY_H__
#define __SPARSEARRAY_H__

#include <bitset>

#include "VectorGrowthPolicy.h"

namespace containers
{
    template < typename T, std::size_t N, typename GrowthPolicy = VectorGrowthPolicyStd >
    class SparseArray
    {
    public:
        typedef T               value_type;
        static const size_t     MAX_SIZE = N;

        SparseArray() = default;

        std::size_t     size() const
        {
            return bitset_.count();
        }

        bool    isInitialized( std::size_t index ) const
        {
            return bitset_.test( index );
        }

        const T&    operator[]( std::size_t index ) const
        {
            return vector_[ getIndexChecked( index ) ];
        }

        T&    operator[]( std::size_t index )
        {
            if ( isInitialized( index ) )
                return vector_[ getVectorIndex( index ) ];

            bitset_.set( index );
            GrowthPolicy::grow( vector_ );
            return *vector_.insert( std::begin( vector_ ) + static_cast< std::size_t >( getVectorIndex( index ) ), T() );
        }

        void    swap( std::size_t i, std::size_t j )
        {
            if ( ! isInitialized( i ) )
                move( j, i );
            else if ( ! isInitialized( j ) )
                move( i, j );
            else
                std::swap( vector_[ getVectorIndex( i ) ], vector_[ getVectorIndex( j ) ] );
        }

        void    reset()
        {
            bitset_.reset();
            GrowthPolicy::clear( vector_ );
        }

        void    reset( std::size_t i )
        {
            if ( ! isInitialized( i ) )
                return;

            vector_.erase( std::begin( vector_ ) + getVectorIndex( i ) );
            GrowthPolicy::shrink( vector_ );
            bitset_.reset( i );
        }

        void    resetIndex( const std::bitset< N >& indexToReset )
        {
            for ( std::size_t i = 0; i < N; ++i )
                if ( indexToReset.test( i ) && isInitialized( i ) )
                    reset( i );
        }

        void    reserve( std::size_t count )
        {
            vector_.reserve( count );
        }

    private:
        std::size_t     getVectorIndex( std::size_t index ) const
        {
            std::size_t result = 0;
            for ( std::size_t i = 0; i < index; ++i )
                if ( isInitialized( i ) )
                    ++result;
            return result;
        }

        std::size_t     getIndexChecked( std::size_t i ) const
        {
            if ( isInitialized( i ) )
                return getVectorIndex( i );

            std::ostringstream ss;
            ss << "SparseArray index " << i << " is not initialised";
            throw std::out_of_range( ss.str() );
        }

        void    move( std::size_t from, std::size_t to )
        {
            if ( ! bitset_[ from ] )
                return;

            T value = vector_[ getVectorIndex( from ) ];
            operator[]( to ) = value;
            reset( from );
        }

    private:
        std::bitset< N >    bitset_;
        std::vector< T >    vector_;
    };
}

#endif /* !__SPARSEARRAY_H__ */
