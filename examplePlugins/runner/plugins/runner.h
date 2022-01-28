#ifndef RUNNER_H
#define RUNNER_H

#include "plugin.h"
#include <vector>
#include <functional>

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

class Runner : public Plugin
{
public:
    virtual void initialize(size_t identifier) = 0;
    virtual void release() = 0;
    virtual void start() = 0;
    virtual void stop() = 0;

    virtual void start2() = 0;
    virtual void stop2() = 0;
    virtual void push(const RunnerDesc& desc) = 0;
    virtual bool pop(const RunnerDesc& desc) = 0;

    static std::vector<RunnerDesc> descriptors;
};

#endif // RUNNER_H