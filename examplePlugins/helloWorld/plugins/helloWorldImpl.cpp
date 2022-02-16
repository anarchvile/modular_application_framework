// This is a very simple example plug-in that is meant to both illustrate the process behind
// correctly structuring a plug-in, and highlighting how a plug-in could be expanded to perform
// more interesting tasks.

#include "stdafx.h"
#include <assert.h>
#include <iostream>
#include <vector>
#include <chrono>
#include <thread>

#include "helloWorldImpl.h"

#ifdef EVENT_HW
#include "event.h"
#endif

#ifdef DIRECT_HW
#include "runner.h"
#include "pluginManager.h"
#endif

#ifdef EVENT_HW
EventStream<double>* es;
size_t g_id = 0;
#endif

#ifdef DIRECT_HW
PluginManager* pm;
Runner* runner;
RunnerDesc desc;
#endif

size_t g_counter = 0;
HelloWorldInterface hwInterface;

// A locally-defined function we wish to register to the runner for updating on the tick. In this 
// case our function simply calls the class's member different update functions, but this is not 
// required and could be easily changed to something else.
void update(double dt)
{
    HelloWorldImpl hwi;
    hwi.preupdate(dt);
    hwi.update(dt);
    hwi.postupdate(dt);
}

// A simple function we wish to later pass by reference to our helloWorld interface.
const char* helloWorldFunc()
{
    return "A helloWorldFunc function defined in helloWorldImpl.cpp";
}

// Define plug-in behavior immediately after being loaded. Simulate complex computations upon loading by
// delaying the program by 3 seconds. Get a pluginManager instance and use it to load a local runner instance.
// Pass by reference both our printHelloWorld() function to the public interface, and our update function to 
// the runner descriptor for registration to the update loop.
void HelloWorldImpl::initialize(size_t identifier)
{
    std::cout << "HelloWorldImpl::initialize: " << identifier << std::endl;

#ifdef EVENT_HW
    es = EventStream<double>::Instance();
#endif
#ifdef DIRECT_HW
    pm = PluginManager::Instance(identifier);
#endif
    std::cout << "Simulate complex computation during HelloWorld's initialization" << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(3));

    hwInterface.helloWorldFunc = &(::helloWorldFunc);
}

// Define plug-in behavior immediately before being unloaded. Simulate complex computations prior to unloading by
// delaying the program by 3 seconds. Remove any function descriptor references from the locally-loaded runner instance,
// before unloading that runner as well.
void HelloWorldImpl::release()
{
    std::cout << "HelloWorldImpl::release" << std::endl;
    std::cout << "Simulate complex computation during HelloWorld's release" << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(3));

#ifdef EVENT_HW
    es = nullptr;
#endif
#ifdef DIRECT_HW
    pm->Unload("runner");
#endif
}

// Starts HelloWorld plugin by pushing a function to runner for updating.
void HelloWorldImpl::start()
{
    std::cout << "HelloWorldImpl::start" << std::endl;
    // TODO: Whenever we load this plugin (thereby calling into initialize), either from C++ or using python bindings,
    // we automatically subscribe update() to runner, so in pythonExamplePlugin this function also gets updated by the
    // runner. Think this is okay for now since it's only done for testing purposes, but still something to keep note of...
    
#ifdef EVENT_HW
    g_id = es->subscribe("runner", &(::update));
#endif

#ifdef DIRECT_HW
    runner = (Runner*)pm->Load("runner");
    desc.priority = 1;
    desc.name = "HelloWorldImpl::update";
    desc.cUpdate = &(::update);
    desc.pyUpdate = nullptr;
    runner->push(desc);
#endif
}

// Stops the HelloWorld function from being updated by runner by popping it off.
void HelloWorldImpl::stop()
{
    std::cout << "HelloWorldImpl::stop" << std::endl;
#ifdef EVENT_HW
    es->unsubscribe("runner", g_id);
#endif
#ifdef DIRECT_HW
    runner->pop(desc);
#endif
}

// Simple preupdate function.
void HelloWorldImpl::preupdate(double dt)
{
    std::cout << "HelloWorldImpl::preupdate " << dt << std::endl;
    //std::this_thread::sleep_for(std::chrono::seconds(1));
}

// Simple update method that signals the runner to unregister itself from the update cycle after 10 iterations.
void HelloWorldImpl::update(double dt)
{
    std::cout << "HelloWorldImpl::update " << dt << std::endl;
    //std::this_thread::sleep_for(std::chrono::seconds(2));
#ifdef DIRECT_HW
    if (g_counter == 10)
    {
        runner->pop(desc);
        //runner->stop();
    }

    ++g_counter;
#endif
}

// Simple postupdate function.
void HelloWorldImpl::postupdate(double dt)
{
    std::cout << "HelloWorldImpl::postupdate " << dt << std::endl;
    //std::this_thread::sleep_for(std::chrono::seconds(3));
}

// Returns a custom interface to this plug-in, which can then be utilized inside other plug-ins
// to call the functions attached to the interface initially. In the specific case of helloWorldImpl,
// we've attached a printHelloWorld() method to the public interface in our initialize method, so once
// helloWorld is loaded inside another context, we could create an instance of the interface to then
// call printHelloWorld(), even though we would not actually be inside helloWorld!
HelloWorldInterface HelloWorldImpl::getInterface()
{
    return hwInterface;
}

// Define functions with C symbols (create/destroy HelloWorldImpl instance)
HelloWorldImpl* g_helloworldimpl = nullptr;

extern "C" HELLOWORLD HelloWorldImpl* Create()
{
    assert(g_helloworldimpl == nullptr);
    g_helloworldimpl = new HelloWorldImpl;
    return g_helloworldimpl;
}
extern "C" HELLOWORLD void Destroy()
{
    assert(g_helloworldimpl);
    delete g_helloworldimpl;
    g_helloworldimpl = nullptr;
}
