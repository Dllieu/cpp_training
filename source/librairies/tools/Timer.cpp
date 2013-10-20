#include <iostream>

#include "Timer.h"

using namespace tools;

Timer::Timer( bool withLog /*= true*/ )
    : clock_( std::chrono::high_resolution_clock::now() )
    , withLog_( withLog )
{
    // NOTHING
}

Timer::~Timer()
{
    if ( withLog_ )
        std::cout << "Time Elapsed: " << elapsed() << std::endl;
}

void    Timer::reset()
{
    clock_ = std::chrono::high_resolution_clock::now();
}

double  Timer::elapsed() const
{
    return std::chrono::duration_cast< std::chrono::duration< double, std::ratio<1> > >( std::chrono::high_resolution_clock::now() - clock_ ).count();
}
