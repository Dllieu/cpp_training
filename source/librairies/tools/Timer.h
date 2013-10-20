#ifndef __TOOLS_TIMER_H__
#define __TOOLS_TIMER_H__

#include <chrono>

namespace tools
{

class Timer
{
public:
    Timer( bool withLog = true );
    ~Timer();

    void    reset();
    double  elapsed() const;

private:
    std::chrono::time_point< std::chrono::high_resolution_clock >   clock_;
    bool                                                            withLog_;
};

}

#endif /* ! __TOOLS_TIMER_H__ */
