#ifndef GOODBYEWORLDIMPL_H
#define GOODBYEWORLDIMPL_H

#include "goodbyeWorld.h"

#ifdef _WIN32
    #ifdef GOODBYEWORLD_EXPORTS
        #define GOODBYEWORLD __declspec(dllexport)
    #else
        #define GOODBYEWORLD __declspec(dllimport)
    #endif
#elif __linux__
    #define GOODBYEWORLD
#endif

class GoodbyeWorldImpl : public GoodbyeWorld
{
public:
    void initialize(size_t identifier);
    void release();
    void start();
    void stop();

    void preupdate(double dt);
    void update(double dt);
    void postupdate(double dt);

    GoodbyeWorldInterface getInterface();
};

#endif // GOODBYEWORLDIMPL_H