#ifndef GOODBYEWORLD_H
#define GOODBYEWORLD_H

#define EVENT_GW
// #define DIRECT_GW

#include "plugin.h"

struct GoodbyeWorldInterface
{
#ifdef _WIN32
    const char*(__cdecl* goodbyeWorldFunc)();
#elif __linux__
    void(*goodbyeWorldFunc)();
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