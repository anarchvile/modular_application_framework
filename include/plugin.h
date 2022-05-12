// Base plug-in class from which all other plug-ins are derived from.
// Requires that all plug-ins have an initialize and release function 
// implemented, which are called by default when plug-ins are loaded 
// and unloaded, respectively. Also requires the presenc of start and
// stop functions, which define plugin behavior during their first and
// last update cycle.

#ifndef PLUGIN_H
#define PLUGIN_H

class Plugin
{
public:
    virtual void initialize(size_t identifier) = 0;
    virtual void release() = 0;
    // start() and stop() are useful for creating custom plugin management systems (like runner)
    virtual void start() = 0;
    virtual void stop() = 0;
};

#endif // PLUGIN_H
