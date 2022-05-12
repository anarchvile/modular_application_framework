#ifndef RUNNER_H
#define RUNNER_H

// Convenience macros to define how the runner will call functions that are pushed/subscribed to it. These are not necessary
// for developing custom plugins, they exist to more clearly highlight more options for structuring a plugin. 
//#define EVENT_SYNC_R // Calls each handler in the runner event one-at-a-time, in sequence, per tick.
#define EVENT_ASYNC_R // Spawns new threads for each handler in runner to be executed in, per tick
//#define EVENT_MULTI_R // Utilize the runner event in multiple different threads.
#define DIRECT_R // Call each RunnerDesc registered to the loaded Runner plugin per tick for updating.

#include "plugin.h"
#include <vector>
#include <functional>

// RunnerDesc is a struct for holding methods we wish to directly push to Runner for updating. It also contains a method name and 
// priority, the latter of which lets the user specify what order the functions should be called in.
#ifdef DIRECT_R
struct RunnerDesc
{
    int priority;
    const char* name;
    #ifdef _WIN32
        void(__cdecl* cUpdate)(double);
        std::function<void(double)> pyUpdate;
    #elif __linux__
        void(*cUpdate)(double);
        std::function<void(double)> pyUpdate;
    #endif
};
#endif

class Runner : public Plugin
{
public:
    virtual void initialize(size_t identifier) = 0;
    virtual void release() = 0;
    virtual void start() = 0;
    virtual void stop() = 0;

    virtual void start2() = 0;
    virtual void stop2() = 0;

#ifdef DIRECT_R
    virtual void push(RunnerDesc desc) = 0;
    virtual bool pop(RunnerDesc desc) = 0;

    static std::vector<RunnerDesc> descriptors;
#endif
};

#endif // RUNNER_H