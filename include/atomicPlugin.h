#ifndef ATOMICPLUGIN_H
#define ATOMICPLUGIN_H

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