#ifndef HELLOWORLD_H
#define HELLOWORLD_H

// Convenience macros to let the user quickly switch between pushing/subscribing helloWorld
// functions to runner and/or input.
//#define EVENT_RUNNER_HW
#define DIRECT_RUNNER_HW

//#define EVENT_KEYBOARD_INPUT_HW
//#define DIRECT_KEYBOARD_INPUT_HW

#define EVENT_MOUSE_INPUT_HW
//#define DIRECT_MOUSE_INPUT_HW

#include "plugin.h"

// Public interface that lets us expose functions locally defined in helloWorld to other plugins
// allowing those other plugins to load helloWorld and utilize its interface (in this example allowing
// them to call helloWorldFunc()).
struct HelloWorldInterface
{
    #ifdef _WIN32
        const char*(__cdecl* helloWorldFunc)();
    #elif __linux__
        const char*(*helloWorldFunc)();
    #endif
};

class HelloWorld : public Plugin
{
public:
    virtual void initialize(size_t identifier) = 0;
    virtual void release() = 0;

    virtual void start() = 0;
    virtual void stop() = 0;

    virtual HelloWorldInterface getInterface() = 0;
};

#endif // HELLOWORLD_H