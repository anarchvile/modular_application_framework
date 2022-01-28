#ifndef ATOMICPLUGINIMPL_H
#define ATOMICPLUGINIMPL_H

#include "atomicPlugin.h"
#include "input.h"
#include <atomic>

#ifdef _WIN32
    #ifdef ATOMICPLUGIN_EXPORTS
        #define ATOMICPLUGIN __declspec(dllexport)
    #else
        #define ATOMICPLUGIN __declspec(dllimport)
    #endif
#elif __linux__
    #define ATOMICPLUGIN
#endif

class AtomicPluginImpl : public AtomicPlugin
{
public:
    void initialize(size_t identifier);
    void release();
    void start();
    void stop();

    void updateTick(double dt);
    void updateInput(InputData inputData);
};

#endif // ATOMICPLUGINIMPL_H
