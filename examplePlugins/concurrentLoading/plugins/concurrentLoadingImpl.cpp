// This plug-in is meant to illustrate how the plug-in system operates in a multi-threaded enivronment,
// specifically highlighting some of the thread safety features surrounding the plugin loading/unloading
// mechanisms.

#include "stdafx.h"
#include <assert.h>
#include <iostream>
#include <vector>
#include <thread>

#include "concurrentLoadingImpl.h"
#include "runner.h"
#include "helloWorld.h"
#include "goodbyeWorld.h"
#include "pluginManager.h"

Plugin* helloWorldPluginPtr = nullptr;
Plugin* goodbyeWorldPluginPtr = nullptr;
Plugin* runnerPluginPtr = nullptr;

Runner* runner = nullptr;

PluginManager* pm;
size_t g_counter = 0;
RunnerDesc desc;

// A locally-defined function we wish to register to the runner for updating on the tick. In this 
// case our function simply calls the class's member different update functions, but this is not 
// required and could be easily changed to something else.
void update(double dt)
{
    ConcurrentLoadingImpl cli;
    cli.preupdate(dt);
    cli.update(dt);
    cli.postupdate(dt);
}

// As was mentioned in pluginManager.cpp, one should avoid spawning new threads for calling PluginManger::Load() or
// PluginManager::Unload() inside any plugin's initialize() or release() function. To see why this is the case, let's
// look at main.cpp and see what steps are being taken when running the entire application:
// loadPlugins()
// start()
// stop()
// unloadPlugins()
// In loadPlugins() we use the PluginManager to Load each plugin, a process which in part calls each plugin's initialize() function.
// In this example the only plugin being loaded/initialized through main (and subsequently startup.cfg) is concurrentLoading.
// Similarly, in unloadPlugins() we use the PluginManager to unload each plugin, a process which in part calls each plugin's release()
// function. If we try spawning new threads in either initialize() or release() through which PluginManager::Load() or 
// PluginManager::Unload() are to be used to try to concurrently load/unload other plugins, then the mutex locks on PluginManager::Load()
// /PluginManager::Unload() will activate and freeze the entire application, since they were already called upon by the main thread
// when attempting to Load/Unload the startup.cfg-specified plugin (in this case concurrentLoading). So, in order to ensure that this
// doesn't happen, any plugin loading/unloading we wish to perform through concurrentLoading is moved to its start() and stop() methods,
// which don't invoke PluginManager's mutex-locked functions whatsoever.
// In general, a plugin's initialize() and release() functions should be geared towards preparing its own native environment for being
// added to/removed from the application runtime, before attempting to pull in/push out other plugins, i.e. a plugin should first be
// fully initialized before attempting to load other plugins, and any plugins it may have loaded should be fully unloaded before it
// tries to release itself from the runtime.

// Define plug-in behavior immediately after being loaded.
void ConcurrentLoadingImpl::initialize(size_t identifier)
{
    std::cout << "ConcurrentLoadingImpl::initialize " << std::endl;

    pm = PluginManager::Instance(identifier);

    // ATTEMPTING TO COMMENT BACK IN THIS CODE WILL LEAD TO THE PLUGIN STALLING
    // THE ENTIRE APPLICATION!
    // ************************************
    // std::thread thread1([]() { pm->Load("helloWorld", helloWorldPluginPtr); });
    // std::thread thread2([]() { pm->Load("goodbyeWorld", goodbyeWorldPluginPtr); });
    // std::thread thread3([]() { pm->Load("runner", runnerPluginPtr); });
    // thread1.join();
    // thread2.join();
    // thread3.join();
    // ************************************
    // LOADING IN EACH PLUGIN WITHIN THE SAME THREAD THAT CONCURRENTLOADING IS
    // BEING EXECUTED IN WILL WORK JUST FINE THOUGH.
    // pm->Load("helloWorld", helloWorldPluginPtr);
    // pm->Load("goodbyeWorld", goodbyeWorldPluginPtr);
    // pm->Load("runner", runnerPluginPtr);
}

// Define plug-in behavior immediately before being unloaded.
void ConcurrentLoadingImpl::release()
{
    std::cout << "ConcurrentLoadingImpl::release" << std::endl;

    // ATTEMPTING TO COMMENT BACK IN THIS CODE WILL LEAD TO THE PLUGIN STALLING
    // THE ENTIRE APPLICATION!
    // ************************************
    // std::thread thread4([]() { pm->Unload("helloWorld"); });
    // std::thread thread5([]() { pm->Unload("goodbyeWorld"); });
    // std::thread thread6([]() { pm->Unload("runner"); });
    // thread4.join();
    // thread5.join();
    // thread6.join();
    // ************************************
    // UNLOADING IN EACH PLUGIN WITHIN THE SAME THREAD THAT CONCURRENTLOADING IS
    // BEING EXECUTED IN WILL WORK JUST FINE THOUGH.
    // pm->Unload("helloWorld");
    // pm->Unload("goodbyeWorld");
    // pm->Unload("runner");

    pm->requestDelete();
}

// The plug - in attempts to simultaneously load the helloWorld,
// goodbyeWorld, and runner plugins via multi-threading. Mutex locking in the containerImpl and pluginManager functions
// effetively prevent this from happening and force each plug-in to be loaded one at a time, so as to preserve the integrity
// of the shared data containers. Note here that the 2nd overloaded PluginManager::Load method comes in handy here, since it
// allows us to pass plug-in pointers by reference inside each thread's ::Load method, thereby initializing those variables
// and allowing us to later use them in our single-threaded environment after the 3 threads have been joined back up.
// The plugin also directly pushes a function to runner for updating, mostly to show that Runner was really, truly successfully
// loaded via thread3.
void ConcurrentLoadingImpl::start()
{
    std::cout << "ConcurrentLoadingImpl::start" << std::endl;

    std::thread thread1([]() { pm->Load("helloWorld", helloWorldPluginPtr); });
    std::thread thread2([]() { pm->Load("goodbyeWorld", goodbyeWorldPluginPtr); });
    std::thread thread3([]() { pm->Load("runner", runnerPluginPtr); });
    thread1.join();
    thread2.join();
    thread3.join();

    runner = (Runner*)runnerPluginPtr;

    desc.priority = 1;
    desc.name = "ConcurrentLoadingImpl::update";
    desc.cUpdate = &(::update);
    desc.pyUpdate = nullptr;
    runner->push(desc);
    runner->start();
}

// Stops the Runner update loop before attempting to unload helloWorld, goodbyeWorld, and runner simultaneously
// in three separate threads. Similar to Load(), the Unload() method is mutex-locked, so each plugin will be unloaded
// one at a time (blocking the other threads until its native process finishes).
void ConcurrentLoadingImpl::stop()
{
    std::cout << "ConcurrentLoadingImpl::stop" << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(5));
    runner->stop();
    std::thread thread4([]() { pm->Unload("helloWorld"); });
    std::thread thread5([]() { pm->Unload("goodbyeWorld"); });
    std::thread thread6([]() { pm->Unload("runner"); });

    thread4.join();
    thread5.join();
    thread6.join();
}

// Simple preupdate function.
void ConcurrentLoadingImpl::preupdate(double dt)
{
    std::cout << "ConcurrentLoadingImpl::preupdate " << dt << std::endl;
}

// Simple update method that pops itself off from runner after 3 iteration cycles.
void ConcurrentLoadingImpl::update(double dt)
{
    std::cout << "ConcurrentLoadingImpl::update " << dt << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(1));

    if (g_counter == 3)
    {
        runner->pop(desc);
    }

    ++g_counter;
}

// Simple postupdate function.
void ConcurrentLoadingImpl::postupdate(double dt)
{
    std::cout << "ConcurrentLoadingImpl::postupdate " << dt << std::endl;
}

// Define functions with C symbols (create/destroy ConcurrentLoadingImpl instance)
ConcurrentLoadingImpl* g_concurrentloadingimpl = nullptr;

extern "C" CONCURRENTLOADING ConcurrentLoadingImpl* Create()
{
    assert(g_concurrentloadingimpl == nullptr);
    g_concurrentloadingimpl = new ConcurrentLoadingImpl;
    return g_concurrentloadingimpl;
}
extern "C" CONCURRENTLOADING void Destroy()
{
    assert(g_concurrentloadingimpl);
    delete g_concurrentloadingimpl;
    g_concurrentloadingimpl = nullptr;
}
