//--------------------------------------------------------------------------------
// (C) Copyright 2014-2015 Stephane Molina, All rights reserved.
// See https://github.com/Dllieu for updates, documentation, and revision history.
//--------------------------------------------------------------------------------
#pragma once

#include <vector>
#include <unordered_map>

namespace containers
{
    namespace details
    {
        template < class Base >
        class CollectionChunkBase
        {
        public:
            virtual ~CollectionChunkBase() = default;

            virtual void insert( Base&& x ) = 0;

            template < typename F >
            void    for_each( F&& f )
            {
                auto eltSize = elementSize();
                for ( auto it = begin(), it_end = it + size() * eltSize; it != it_end; it += eltSize )
                    f( *reinterpret_cast< Base* >( it ) );
            }

            template < typename F >
            void    for_each( F&& f ) const
            {
                auto eltSize = elementSize();
                for ( auto it = begin(), it_end = it + size() * eltSize; it != it_end; it += eltSize )
                    f( *reinterpret_cast< const Base* >( it ) );
            }

        private:
            virtual char*           begin() = 0;
            virtual const char*     begin() const = 0;
            virtual std::size_t     size() const = 0;
            virtual std::size_t     elementSize() const = 0;
        };

        template < class Derived, class Base >
        class CollectionChunk : public CollectionChunkBase< Base >
        {
        private:
            virtual void insert( Base&& x ) override final
            {
                store.emplace_back( static_cast< Derived&& >( x ) );
            }

            virtual char* begin() override final
            {
                return reinterpret_cast< char* >( static_cast< Base* >( const_cast< Derived* >( store.data() ) ) );
            }

            virtual const char* begin() const override final
            {
                return reinterpret_cast< const char* >( static_cast< const Base* >( store.data() ) );
            }

            virtual std::size_t     size() const override final { return store.size(); }
            virtual std::size_t     elementSize() const override final { return sizeof( Derived ); }

            std::vector< Derived > store;
        };
    }

    template < typename Base >
    class PolymorphicCollection
    {
    public:
        template < class Derived >
        void    insert( Derived&& x )
        {
            static_assert( std::is_base_of< Base, Derived >::value, "Type mismatch" );

            auto& chunk = chunks[ typeid( x ) ];
            if ( ! chunk )
                chunk.reset( new details::CollectionChunk< Derived, Base >() );

            chunk->insert( std::forward< Derived >( x ) );
        }

        template < typename F >
        void    for_each( F&& f )
        {
            for ( const auto& p : chunks )
                p.second->for_each( std::forward< F >( f ) );
        }

        template < typename F >
        void    for_each( F&& f ) const
        {
            for ( const auto& p : chunks )
                const_cast< const details::CollectionChunkBase< Base >& >( *p.second ).for_each( std::forward< F >( f ) );
        }

    private:
        std::unordered_map< std::type_index, std::unique_ptr< details::CollectionChunkBase< Base > > > chunks;
    };
}
