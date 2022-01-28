#ifndef HELLOWORLDIMPL_H
#define HELLOWORLDIMPL_H

#include "helloWorld.h"

#ifdef _WIN32
    #ifdef HELLOWORLD_EXPORTS
        #define HELLOWORLD __declspec(dllexport)
    #else
        #define HELLOWORLD __declspec(dllimport)
    #endif
#elif __linux__
    #define HELLOWORLD
#endif

class HelloWorldImpl : public HelloWorld
{
public:
    void initialize(size_t identifier);
    void release();
    void start();
    void stop();

    void preupdate(double dt);
    void update(double dt);
    void postupdate(double dt);

    HelloWorldInterface getInterface();
};

#endif // HELLOWORLD_H