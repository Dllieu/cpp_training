//--------------------------------------------------------------------------------
// (C) Copyright 2014-2015 Stephane Molina, All rights reserved.
// See https://github.com/Dllieu for updates, documentation, and revision history.
//--------------------------------------------------------------------------------
#ifndef __GENERICS_PROXY_FUNCTOR_H__
#define __GENERICS_PROXY_FUNCTOR_H__

#include <unordered_map>
#include <iostream>

#include "HashCombine.h"
#include "TuplePrinter.h"

namespace generics
{
    template < typename R, typename... Args >
    class ProxyFunctor
    {
    public:
        using functor_type = std::function< R ( Args... ) >;
        using proxy_type = std::function< R ( functor_type, Args... ) >;

        ProxyFunctor( proxy_type proxy, functor_type f )
            : proxy_( proxy )
            , functor_( f )
        {}

        inline R   operator()( Args&&... args ) const
        {
            return proxy_( functor_, std::forward< Args... >( args... ) );
        }

    private:
        proxy_type      proxy_;
        functor_type    functor_;
    };

    // TODO : transform into real policies
    template < typename R, typename... Args >
    class ProxyPolicyCache
    {
    public:
        using functor_type = std::function< R( Args... ) >;

        inline R   operator()( const functor_type& functor, Args&&... args )
        {
            size_t hashValue{ hashCombine( args... ) };
            auto it = cachedResults_.find( hashValue );
            if ( it != std::end( cachedResults_ ) )
                return it->second;

            auto result = functor( std::forward< Args... >( args... ) );
            cachedResults_.insert( { hashValue, result } );
            return result;
        }

    private:
        std::unordered_map< size_t, R > cachedResults_;
    };

    template < typename R, typename... Args >
    class ProxyPolicyDisplay
    {
    public:
        using functor_type = std::function< R( Args... ) >;

        inline R   operator()( const functor_type& functor ) const
        {
            return functor();
        }

        inline R   operator()( const functor_type& functor, Args&&... args ) const
        {
            std::tuple< Args... > tuple{ args... };
            std::cout << "Args(" << tuple << ")" << std::endl;

            auto result = functor( std::forward< Args... >( args... ) );
            std::cout << "R(" << result << ")" << std::endl;
            return result;
        }
    };
}

#endif // ! __GENERICS_PROXY_FUNCTOR_H__
