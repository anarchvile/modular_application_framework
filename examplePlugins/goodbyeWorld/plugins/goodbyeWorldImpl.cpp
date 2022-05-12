// This plug-in is almost a carbon-copy of the helloWorld plug-in. It is mostly meant to work in
// tandem with helloWorld to show what loading two plug-ins in our main application looks like.

#include "stdafx.h"
#include <assert.h>
#include <iostream>
#include <vector>
#include <pybind11/embed.h>
#include <thread>

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
std::vector<size_t> g_ids;
#endif

#ifdef DIRECT_GW
Runner* runner = nullptr;
PluginManager* pm;
RunnerDesc desc1, desc2, desc3;
#endif

GoodbyeWorldInterface gwInterface;

// A series of locally-defined functions we wish to register to the runner for updating on the tick.
#if defined(EVENT_GW) || defined(DIRECT_GW)
void runnerUpdate1(double dt)
{
    std::this_thread::sleep_for(std::chrono::seconds(1));
    std::cout << "GoodbyeWorld Update1 = " << dt << std::endl;
}

void runnerUpdate2(double dt)
{
    std::this_thread::sleep_for(std::chrono::seconds(2));
    std::cout << "GoodbyeWorld Update2 = " << dt << std::endl;
}

void runnerUpdate3(double dt)
{
    std::this_thread::sleep_for(std::chrono::seconds(3));
    std::cout << "GoodbyeWorld Update3 = " << dt << std::endl;
}
#endif

// Two simple methods we wish to later pass by reference to our goodbyeWorld interface.
const char* goodbyeWorldFunc()
{
    return "A goodbyeWorldFunc function defined in goodbyeWorldImpl.cpp";
}

void anotherGoodbyeWorldFunc()
{
    std::cout << "Yet another function defined in goodbyeWorldImpl.cpp" << std::endl;
}

// Define plug-in behavior immediately after being loaded. Get a pluginManager instance if we'll be loading
// the runner plugin directly later. Pass by reference both our goodbyeWorldFunc() and anotherGoodbyeWorldFunc() 
// to the public interface.
void GoodbyeWorldImpl::initialize(size_t identifier)
{
    std::cout << "GoodbyeWorldImpl::initialize: " << identifier << std::endl;

#ifdef EVENT_GW
    es = EventStream<double>::Instance(identifier);
#endif
#ifdef DIRECT_GW
    pm = PluginManager::Instance(identifier);
#endif
    //pybind11::print("Hello there using Python API from goodbyeWorld!");
    std::cout << "GoodbyeWorldImpl::initialize: " << identifier << std::endl;

    gwInterface.goodbyeWorldFunc = &(::goodbyeWorldFunc);
    gwInterface.anotherGoodbyeWorldFunc = &(::anotherGoodbyeWorldFunc);
}

// Define plug-in behavior immediately before being unloaded. Unload runner if it was loaded (i.e. if DIRECT_GW
// is defined).
void GoodbyeWorldImpl::release()
{
    std::cout << "GoodbyeWorldImpl::release" << std::endl;

#ifdef EVENT_GW
    es->requestDelete();
    es = nullptr;
#endif
#ifdef DIRECT_GW
    pm->Unload("runner");
    pm->requestDelete();
#endif
}

// Starts GoodbyeWorld plugin by pushing and/or subscribing functions to a variety of plugins and events.
void GoodbyeWorldImpl::start()
{
    std::cout << "GoodbyeWorldImpl::start" << std::endl;

    // Subscribe runnerUpdate1, runnerUpdate2, and runnerUpdate3 to the runner event using the double-type
    // EventStream. Note that a few different examples of how these subscriptions can be performed are shown here.
#ifdef EVENT_GW
    // METHOD 1 - Create a vector of std::functions, and pass that vector to the subscribe method.
    /*
    std::function<void(double)> fRunnerUpdate1 = runnerUpdate1;
    std::function<void(double)> fRunnerUpdate2 = runnerUpdate2;
    std::function<void(double)> fRunnerUpdate3 = runnerUpdate3;
    std::vector<std::function<void(double)>> handlerFuncs; handlerFuncs.reserve(3);
    handlerFuncs.push_back(fRunnerUpdate1);
    handlerFuncs.push_back(fRunnerUpdate2);
    handlerFuncs.push_back(fRunnerUpdate3);
    g_ids = es->subscribe("runner", handlerFuncs);
    */

    // METHOD 2 - Create a vector of function pointers, and pass that vector to the subscribe method.
    /*
    std::vector<void (*)(double)> handlerFuncs; handlerFuncs.reserve(3);
    handlerFuncs.push_back(runnerUpdate1);
    handlerFuncs.push_back(runnerUpdate2);
    handlerFuncs.push_back(runnerUpdate3);
    g_ids = es->subscribe("runner", handlerFuncs);
    */

    // METHOD 3 - Directly pass std::functions to the subscribe method.
    /*
    std::function<void(double)> fRunnerUpdate1 = runnerUpdate1;
    std::function<void(double)> fRunnerUpdate2 = runnerUpdate2;
    std::function<void(double)> fRunnerUpdate3 = runnerUpdate3;
    g_ids = es->subscribe("runner", fRunnerUpdate1, fRunnerUpdate2, fRunnerUpdate3);
    */

    // METHOD 4 - Directly pass the function pointers to the subscribe method.
    g_ids = es->subscribe("runner", runnerUpdate1, runnerUpdate2, runnerUpdate3);

#endif
    // Load the Runner plugin using the PluginManager, directly create RunnerDescs for runnerUpdate1, runnerUpdate2, and runnerUpdate3,
    // and push it to the Runner for updating.

    // Note that the priority on the update function descriptor is greater than that of the helloWorld plug-in, 
    // which means that this goodbyeWorld update function will only be iterated over after the helloWorld update 
    // function has been processed.
#ifdef DIRECT_GW
    runner = (Runner*)pm->Load("runner");
    desc1.priority = 2;
    desc1.name = "GoodbyeWorldImpl::update";
    desc1.cUpdate = &(::runnerUpdate1);
    desc1.pyUpdate = nullptr;
    runner->push(desc1);

    desc2.priority = 3;
    desc2.name = "GoodbyeWorldImpl::update";
    desc2.cUpdate = &(::runnerUpdate2);
    desc2.pyUpdate = nullptr;
    runner->push(desc2);

    desc3.priority = 4;
    desc3.name = "GoodbyeWorldImpl::update";
    desc3.cUpdate = &(::runnerUpdate3);
    desc3.pyUpdate = nullptr;
    runner->push(desc3);
#endif
}

// Stops the runnerUpdate1, runnerUpdate2, and/or runnerUpdate3 functions from
// being called through a combination of popping off and unsubscribing these 
// functions from the runner plugin/event they were initially pushed/subscribed to.
void GoodbyeWorldImpl::stop()
{
    std::cout << "GoodbyeWorldImpl::stop" << std::endl;
#ifdef EVENT_GW
    // METHOD 1 - Pass a vector of ids to unsubscribe the corresponding functions in a single call.
    //es->unsubscribe("runner", g_ids);

    // METHOD 2 - Pass each handler id to the unsubscribe method as separate arguments.
    es->unsubscribe("runner", g_ids[0], g_ids[1], g_ids[2]);

#endif
#ifdef DIRECT_GW
    runner->pop(desc1);
    runner->pop(desc2);
    runner->pop(desc3);
#endif
}

// Simple preupdate function.
void GoodbyeWorldImpl::preupdate(double dt)
{
    std::cout << "GoodbyeWorldImpl::preupdate " << dt << std::endl;
}

// Simple update method that can signal to the runner to unregister itself from the update cycle after 20 iterations.
// The greater number of iterations relative to helloWorld means that goodbyeWorld will continue to be updated
// by the runner even after helloWorld has unregistered.
void GoodbyeWorldImpl::update(double dt)
{
    std::cout << "GoodbyeWorldImpl::update " << dt << std::endl;
}

// Simple postupdate function.
void GoodbyeWorldImpl::postupdate(double dt)
{
    std::cout << "GoodbyeWorldImpl::postupdate " << dt << std::endl;
}

// Returns a custom interface to this plug-in, which can then be utilized inside other plug-ins
// to call the functions attached to the interface initially. In the specific case of goodbyeWorldImpl,
// we've attached a goodbyeWorldFunc() and anotherGoodbyeWorldFunc() method to the public interface in 
// our initialize method, so once goodbyeWorld is loaded inside another context, we could create an 
// instance of the interface to then call goodbyeWorldFunc() or anotherGoodbyeWorldFunc(), even though we 
// would not actually be inside goodbyeWorld!
GoodbyeWorldInterface GoodbyeWorldImpl::getInterface()
{
    return gwInterface;
}

// Define functions with C symbols (create/destroy GoodbyeWorldImpl instance).
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
