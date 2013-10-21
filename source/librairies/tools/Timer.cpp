#include <iostream>
#include <string>
#include "Timer.h"

using namespace tools;

Timer::Timer( const std::string& customMessage /*= ""*/ )
    : clock_( std::chrono::high_resolution_clock::now() )
    , customMessage_( customMessage.empty() ? "Time Elapsed" : customMessage )
{
    // NOTHING
}

Timer::~Timer()
{
    log();
}

void    Timer::reset()
{
    clock_ = std::chrono::high_resolution_clock::now();
}

double  Timer::elapsed() const
{
    return std::chrono::duration_cast< std::chrono::duration< double, std::ratio<1> > >( std::chrono::high_resolution_clock::now() - clock_ ).count();
}

void    Timer::log() const
{
    std::cout << customMessage_ << ": " << elapsed() << std::endl;
}
