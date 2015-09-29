//--------------------------------------------------------------------------------
// (C) Copyright 2014-2015 Stephane Molina, All rights reserved.
// See https://github.com/Dllieu for updates, documentation, and revision history.
//--------------------------------------------------------------------------------
#pragma once

#include <exception>

#include "AnonymousVariable.h"

// Idea from Andrei Alexandrescu talks about declarative control flows
// Light version of folly implementation
// ScopedGuard class should not be used directly, just use the macro (see below)
namespace tools
{
    template < typename F >
    class ScopeGuard
    {
    public:
        ScopeGuard( F&& f )
            : f_( std::move( f ) )
        {}

        ~ScopeGuard()
        {
            f_();
        }

    private:
        F   f_;
    };

    template < typename F, bool EXECUTE_ON_EXCEPTION >
    class ScopeGuardNewException
    {
    public:
        ScopeGuardNewException( F&& f )
            : f_( std::move( f ) )
            , exceptionThrowns_( std::uncaught_exceptions() )
        {}

        // should not throw if ON_EXCEPTION
        ~ScopeGuardNewException() noexcept( EXECUTE_ON_EXCEPTION )
        {
            if ( EXECUTE_ON_EXCEPTION == ( exceptionThrowns_ < std::uncaught_exceptions() ) )
                f_();
        }

    private:
        F   f_;
        int exceptionThrowns_;
    };

    enum class ScopeGuardOnExit {};
    enum class ScopeGuardOnFailure {};
    enum class ScopeGuardOnSuccess {};

    template < typename F >
    auto    operator+( ScopeGuardOnExit, F&& f ) { return ScopeGuard< F >( std::forward< F >( f ) ); }

    template < typename F >
    auto    operator+( ScopeGuardOnFailure, F&& f ) { return ScopeGuardNewException< F, true >( std::forward< F >( f ) ); }

    template < typename F >
    auto    operator+( ScopeGuardOnSuccess, F&& f ) { return ScopeGuardNewException< F, false >( std::forward< F >( f ) ); }
}

// TO BE USED
#define SCOPE_EXIT \
    auto ANONYMOUS_VARIABLE( SCOPE_EXIT_STATE ) = tools::ScopeGuardOnExit() + [&]()

#define SCOPE_FAIL \
    auto ANONYMOUS_VARIABLE( SCOPE_FAIL_STATE ) = tools::ScopeGuardOnFailure() + [&]()

#define SCOPE_SUCCESS \
    auto ANONYMOUS_VARIABLE( SCOPE_SUCCESS_STATE ) = tools::ScopeGuardOnSuccess() + [&]()
