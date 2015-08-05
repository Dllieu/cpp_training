//--------------------------------------------------------------------------------
// (C) Copyright 2014-2015 Stephane Molina, All rights reserved.
// See https://github.com/Dllieu for updates, documentation, and revision history.
//--------------------------------------------------------------------------------
#ifndef __TOOLS_TIMER_H__
#define __TOOLS_TIMER_H__

#include <chrono>

namespace tools
{

/**
 * \brief Timer : timer which print out a message at destruction
 * \author Stephane Molina
 * \date 07 april 2014
 *
 * TODO : utiliser boost::timer en interne pour la clock
 */
class Timer
{
public:
    Timer( const std::string& customMessage = "" );
    ~Timer();

    void    reset();
    double  elapsed() const;
    void    log() const;
    
    template < typename Func, typename... Args >
    static auto call( const std::string& customMessage, Func&& func, Args&&... args ) -> decltype( func( args... ) )
    {
        Timer t( customMessage );
        return func( std::forward< Args >( args... ) );
    }
    
    template < typename Func, typename... Args >
    static double elapsed( const std::string& customMessage, Func&& func, Args&&... args )
    {
        Timer t( customMessage );
        func( std::forward< Args >( args... ) );
        return t.elapsed();
    }

private:
    std::chrono::time_point< std::chrono::high_resolution_clock >   clock_;
    std::string                                                     customMessage_;
};

}

#endif /* ! __TOOLS_TIMER_H__ */
