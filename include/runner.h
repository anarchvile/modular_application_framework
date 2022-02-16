#ifndef RUNNER_H
#define RUNNER_H

//#define EVENT_SYNC_R
#define EVENT_ASYNC_R
//#define EVENT_MULTI_R
#define DIRECT_R

#include "plugin.h"
#include <vector>
#include <functional>

#ifdef DIRECT_R
struct RunnerDesc
{
    int priority;
    const char* name;
    #ifdef _WIN32
        void(__cdecl* cUpdate)(double);
        std::function<void(double)> pyUpdate; // Note: because of python bindings we leave this std::function type here instead of using simply C-types. To address later...
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
    virtual void push(const RunnerDesc& desc) = 0;
    virtual bool pop(const RunnerDesc& desc) = 0;

    static std::vector<RunnerDesc> descriptors;
#endif
};

#endif // RUNNER_H