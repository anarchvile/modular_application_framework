// This plug-in is almost a carbon-copy of the helloWorld plug-in. It is mostly meant to work in
// tandem with helloWorld to show what loading two plug-ins in our main application looks like.

#include "stdafx.h"
#include <assert.h>
#include <iostream>
#include <vector>
#include <pybind11/embed.h>

#include "goodbyeWorldImpl.h"

#ifdef EVENT_GW
#include "event.h"
#endif
#ifdef DIRECT_GW
#include "runner.h"
#include "pluginManager.h"
#endif

#ifdef EVENT_GW
EventStream<double>* es;
size_t g_id = 0, g_id1 = 0, g_id2 = 0, g_id3 = 0;
#endif

#ifdef DIRECT_GW
Runner* runner = nullptr;
PluginManager* pm;
RunnerDesc desc;
size_t g_counter = 0;
#endif

GoodbyeWorldInterface gwInterface;

// A locally-defined function we wish to register to the runner for updating on the tick. In this 
// case our function simply calls the class's member different update functions, but this is not 
// required and could be easily changed to something else.
void update(double dt)
{
    //std::this_thread::sleep_for(std::chrono::seconds(1));
    GoodbyeWorldImpl gwi;
    gwi.preupdate(dt);
    gwi.update(dt);
    gwi.postupdate(dt);
}

void update1(double dt)
{
    std::this_thread::sleep_for(std::chrono::seconds(1));
    std::cout << "GoodbyeWorld Update1 = " << dt << std::endl;
}

void update2(double dt)
{
    std::this_thread::sleep_for(std::chrono::seconds(2));
    std::cout << "GoodbyeWorld Update2 = " << dt << std::endl;
}

void update3(double dt)
{
    std::this_thread::sleep_for(std::chrono::seconds(3));
    std::cout << "GoodbyeWorld Update3 = " << dt << std::endl;
}

// A simple function we wish to later pass by reference to our goodbyeWorld interface.
const char* goodbyeWorldFunc()
{
    return "A goodbyeWorldFunc function defined in goodbyeWorldImpl.cpp";
}

// Define plug-in behavior immediately after being loaded. Get a pluginManager instance and use it to load a 
// local runner instance. Pass by reference both our printGoodbyeWorld() function to the public interface, and 
// our update function to the runner descriptor for registration to the update loop. Note that the priority on
// the update function descriptor is greater than that of the helloWorld plug-in, which means that this goodbyeWorld
// update function will only be iterated over after the helloWorld update function has been processed.
void GoodbyeWorldImpl::initialize(size_t identifier)
{
#ifdef EVENT_GW
    es = EventStream<double>::Instance();
#endif
#ifdef DIRECT_GW
    pm = PluginManager::Instance(identifier);
#endif
    pybind11::print("Hello there using Python API from goodbyeWorld!");
    std::cout << "GoodbyeWorldImpl::initialize: " << identifier << std::endl;

    gwInterface.goodbyeWorldFunc = &(::goodbyeWorldFunc);
}

// Define plug-in behavior immediately before being unloaded. Remove any function descriptor references from the locally-loaded 
// runner instance, before unloading that runner as well.
void GoodbyeWorldImpl::release()
{
    std::cout << "GoodbyeWorldImpl::release" << std::endl;

#ifdef EVENT_GW
    es = nullptr;
#endif
#ifdef DIRECT_GW
    pm->Unload("runner");
#endif
}

void GoodbyeWorldImpl::start()
{
    std::cout << "GoodbyeWorldImpl::start" << std::endl;

#ifdef EVENT_GW
    //g_id = es->subscribe("runner", &(::update));
    g_id1 = es->subscribe("runner", &(::update1));
    g_id2 = es->subscribe("runner", &(::update2));
    g_id3 = es->subscribe("runner", &(::update3));
    //std::cout << "__________Count from goodbyeWorld__________" << std::endl;
    //es->numEvents();
#endif
#ifdef DIRECT_GW
    runner = (Runner*)pm->Load("runner");
    desc.priority = 2;
    desc.name = "GoodbyeWorldImpl::update";
    desc.cUpdate = &(::update);
    desc.pyUpdate = nullptr;
    runner->push(desc);
#endif
}

void GoodbyeWorldImpl::stop()
{
    std::cout << "GoodbyeWorldImpl::stop" << std::endl;
#ifdef EVENT_GW
    //es->unsubscribe("runner", g_id);
    es->unsubscribe("runner", g_id1);
    es->unsubscribe("runner", g_id2);
    es->unsubscribe("runner", g_id3);
#endif
#ifdef DIRECT_GW
    runner->pop(desc);
#endif
}

// Simple preupdate function.
void GoodbyeWorldImpl::preupdate(double dt)
{
    std::cout << "GoodbyeWorldImpl::preupdate " << dt << std::endl;
}

// Simple update method that signals the runner to unregister itself from the update cycle after 20 iterations.
// The greater number of iterations relative to helloWorld means that goodbyeWorld will continue to be updated
// by the runner even after helloWorld has unregistered. This is also why the runner->stop() method is not
// commented out here - we only break the update cycle after goodbyeWorld has finished its 20 iterations, since
// trying to end the process in helloWorld would also stop goodbyeWorld from updating despite having another 10
// iterations left (which is not the correct/intended behavior).
void GoodbyeWorldImpl::update(double dt)
{
    std::cout << "GoodbyeWorldImpl::update " << dt << std::endl;

#ifdef DIRECT_GW
    if (g_counter == 20)
    {
        //runner->pop(desc);
        //runner->stop();
    }

    ++g_counter;
#endif
}

// Simple postupdate function.
void GoodbyeWorldImpl::postupdate(double dt)
{
    std::cout << "GoodbyeWorldImpl::postupdate " << dt << std::endl;
}

// Returns a custom interface to this plug-in, which can then be utilized inside other plug-ins
// to call the functions attached to the interface initially. In the specific case of goodbyeWorldImpl,
// we've attached a printGoodbyeWorld() method to the public interface in our initialize method, so once
// helloWorld is loaded inside another context, we could create an instance of the interface to then
// call printGoodbyeWorld(), even though we would not actually be inside goodbyeWorld!
GoodbyeWorldInterface GoodbyeWorldImpl::getInterface()
{
    return gwInterface;
}

// define functions with C symbols (create/destroy GoodbyeWorldImpl instance)
GoodbyeWorldImpl* g_goodbyeworldimpl = nullptr;

extern "C" GOODBYEWORLD GoodbyeWorldImpl* Create()
{
    assert(g_goodbyeworldimpl == nullptr);
    g_goodbyeworldimpl = new GoodbyeWorldImpl;
    return g_goodbyeworldimpl;
}
extern "C" GOODBYEWORLD void Destroy()
{
    assert(g_goodbyeworldimpl);
    delete g_goodbyeworldimpl;
    g_goodbyeworldimpl = nullptr;
}
