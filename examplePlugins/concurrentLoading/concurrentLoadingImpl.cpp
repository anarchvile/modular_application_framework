// This plug-in is meant to illustrate how this plug-in system operates in a multi-threaded enivronment.

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

HelloWorld* helloWorldPtr = nullptr;
GoodbyeWorld* goodbyeWorldPtr = nullptr;
Runner* runner = nullptr;

PluginManager* pm;
const RunnerDesc* runnerDesc = nullptr;
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

// Define plug-in behavior immediately after being loaded. The plug-in attempts to simultaneously load the helloWorld,
// goodbyeWorld, and runner plugins via multi-threading. Mutex locking in the containerImpl and pluginManager functions
// effetively prevent this from happening and force each plug-in to be loaded one at a time, so as to preserve the integrity
// of the shared data containers. Note here that the 2nd overloaded PluginManager::Load method comes in handy here, since it
// allows us to pass plug-in pointers by reference inside each thread's ::Load method, thereby initializing those variables
// and allowing us to later use them in our single-threaded environment after the 3 threads have been joined back up.
void ConcurrentLoadingImpl::initialize(size_t identifier)
{
    std::cout << "ConcurrentLoadingImpl::initialize " << std::endl;

    pm = PluginManager::Instance(identifier);

    std::thread thread1 = std::thread(static_cast<void (*)(const char*, Plugin* &)>(&PluginManager::Load), "helloWorld", std::ref(helloWorldPluginPtr));
    std::thread thread2 = std::thread(static_cast<void (*)(const char*, Plugin*&)>(&PluginManager::Load), "goodbyeWorld", std::ref(goodbyeWorldPluginPtr));
    std::thread thread3 = std::thread(static_cast<void (*)(const char*, Plugin*&)>(&PluginManager::Load), "runner", std::ref(runnerPluginPtr));

    thread1.join();
    thread2.join();
    thread3.join();

    helloWorldPtr = (HelloWorld*)helloWorldPluginPtr;
    goodbyeWorldPtr = (GoodbyeWorld*)goodbyeWorldPluginPtr;
    runner = (Runner*)runnerPluginPtr;

    desc.priority = 2;
    desc.name = "ConcurrentLoadingImpl::update";
    desc.update = &(::update);
    runner->push(desc);
}

// Simple preupdate function.
void ConcurrentLoadingImpl::preupdate(double dt)
{
    std::cout << "ConcurrentLoadingImpl::preupdate " << dt << std::endl;
}

// Simple update method that signals the runner to unregister itself from the update cycle after 30 iterations. In the first 10 
// iterations, we would see the helloWorld, goodbyeWorld, and concurrentLoading update functions all print messages to the console.
// After that, helloWorld unregisters itself from the tick, leaving only goodbyeWorld and concurrentLoading. After another 10
// ticks, goodbyeWorld also gets unregistered, leaving only the concurrentLoading plug-in to update for the last 10 cycles (provided
// that the runner->stop() method in goodbyeWorldImpl::update is also commented out...).
void ConcurrentLoadingImpl::update(double dt)
{
    std::cout << "ConcurrentLoadingImpl::update " << dt << std::endl;

    if (g_counter == 30)
    {
        runner->pop(desc);
        runner->stop();
    }

    ++g_counter;
}

// Simple postupdate function.
void ConcurrentLoadingImpl::postupdate(double dt)
{
    std::cout << "ConcurrentLoadingImpl::postupdate " << dt << std::endl;
}

// Define plug-in behavior immediately before being unloaded. Unloads the 3 plug-ins inside
// three separate threads, which are still processed one-by-one for the same reasons outlined
// in the initialize function comment.
void ConcurrentLoadingImpl::release()
{
    std::cout << "ConcurrentLoadingImpl::release" << std::endl;
    runner->pop(desc);

    std::thread thread1 = std::thread(&PluginManager::Unload, "helloWorld");
    std::thread thread2 = std::thread(&PluginManager::Unload, "goodbyeWorld");
    std::thread thread3 = std::thread(&PluginManager::Unload, "runner");

    thread1.join();
    thread2.join();
    thread3.join();
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
