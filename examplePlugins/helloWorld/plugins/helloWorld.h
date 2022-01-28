#ifndef HELLOWORLD_H
#define HELLOWORLD_H

#include "plugin.h"

struct HelloWorldInterface
{
    #ifdef _WIN32
        const char*(__cdecl* printHelloWorld)();
    #elif __linux__
        void(*printHelloWorld)();
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