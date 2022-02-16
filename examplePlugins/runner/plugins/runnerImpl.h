#ifndef RUNNERIMPL_H
#define RUNNERIMPL_H

#ifdef _WIN32
    #ifdef RUNNER_EXPORTS
        #define RUNNER __declspec(dllexport)
    #else
        #define RUNNER __declspec(dllimport)
    #endif
#elif __linux__
    #define RUNNER
#endif

#include "runner.h"
#include <thread>

class RunnerImpl : public Runner
{
    void initialize(size_t identifier);
    void release();
    void start();
    void stop();

    void start2();
    void stop2();

#ifdef DIRECT_R
    void push(const RunnerDesc& desc);
    bool pop(const RunnerDesc& desc);
#endif

    std::thread thread;
};

#ifdef DIRECT_R
struct priority_queue
{
    inline bool operator() (const RunnerDesc& runnerDesc1, const RunnerDesc& runnerDesc2)
    {
        return (runnerDesc1.priority < runnerDesc2.priority);
    }
};
#endif

#endif // RUNNERIMPL_H