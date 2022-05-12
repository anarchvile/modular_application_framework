#ifndef ATOMICPLUGIN_H
#define ATOMICPLUGIN_H

// Convenience macros to let the user quickly switch between having the atomicPlugin subscribe its update
// functions to runner, keyboard, and mouse events, or directly pushing its update function to the runner
// and input plugins to be iterated on.
#define EVENT_A
//#define DIRECT_A

#include "plugin.h"

class AtomicPlugin : public Plugin
{
public:
    virtual void initialize(size_t identifier) = 0;
    virtual void release() = 0;
    virtual void start() = 0;
    virtual void stop() = 0;
};

#endif // ATOMICPLUGIN_H