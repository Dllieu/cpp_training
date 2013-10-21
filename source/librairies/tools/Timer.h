#ifndef __TOOLS_TIMER_H__
#define __TOOLS_TIMER_H__

#include <chrono>

namespace tools
{

class Timer
{
public:
    Timer( const std::string& customMessage = "" );
    ~Timer();

    void    reset();
    double  elapsed() const;
    void    log() const;

private:
    std::chrono::time_point< std::chrono::high_resolution_clock >   clock_;
    std::string                                                     customMessage_;
};

}

#endif /* ! __TOOLS_TIMER_H__ */
