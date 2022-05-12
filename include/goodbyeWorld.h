#ifndef GOODBYEWORLD_H
#define GOODBYEWORLD_H

// Conveniece macros to let the user quickly switch between pushing goodbyeWorld functions to a Runner plugin directly
// for updating, or subscribing said functions to the corresponding Runner event.
#define EVENT_GW
//#define DIRECT_GW

#include "plugin.h"

// Public interface that lets us expose functions locally defined in goodbyeWorld to other plugins
// allowing those other plugins to load goodbyeWorld and utilize its interface (in this example allowing
// them to call goodbyeWorldFunc()).
struct GoodbyeWorldInterface
{
#ifdef _WIN32
    const char*(__cdecl* goodbyeWorldFunc)();
    void(__cdecl* anotherGoodbyeWorldFunc)();
#elif __linux__
    const char*(*goodbyeWorldFunc)();
    void(*anotherGoodbyeWorldFunc)();
#endif
};

class GoodbyeWorld : public Plugin
{
public:
    virtual void initialize(size_t identifier) = 0;
    virtual void release() = 0;

    virtual void start() = 0;
    virtual void stop() = 0;
    
    virtual GoodbyeWorldInterface getInterface() = 0;
};

#endif // GOODBYEWORLD_H