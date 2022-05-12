// The runner plug-in is responsible for continuously running a game-engine style update
// loop, and for managing the adding/removing of external plug-in functions to the update cycle 
// via either a RunnerDesc struct (direct) or a corresponding runner event. Functions that wish to 
// be updated in this global loop need to either pass a reference of themselves to a RunnerDesc 
// instance (along with some other information, like priority and name) before passing that RunnerDesc 
// to a local instance of the Runner plug-in (which handles the rest of the process), or subscribe 
// themselves to the runner event using the global EventStream instance.

#include "stdafx.h"

#include <assert.h>
#include <iostream>
#include <algorithm>
#include <chrono>
#include <mutex>
#include <string.h>

#include "runnerImpl.h"

#if defined(EVENT_SYNC_R) || defined(EVENT_ASYNC_R) || defined(EVENT_MULTI_R)
#include "event.h"
#endif

#if defined(EVENT_SYNC_R) || defined(EVENT_ASYNC_R) || defined(EVENT_MULTI_R)
EventStream<double>* es;
#endif
#ifdef DIRECT_R
std::vector<RunnerDesc> Runner::descriptors;
#endif

std::atomic<bool> breakLoop = false;
std::atomic<bool> g_block = false;
std::atomic<bool> g_start = true;
std::atomic<bool> g_stop = false;
std::thread t1, t2;

#ifdef EVENT_MULTI_R
void callAsyncWrapper(std::chrono::high_resolution_clock::time_point lastTime)
{
    g_start = false;
    while (!breakLoop)
    {
        if (g_block) continue;

        std::chrono::high_resolution_clock::time_point currentTime = std::chrono::high_resolution_clock::now();
        double elapsed = std::chrono::duration_cast<std::chrono::duration<double>>(currentTime - lastTime).count();

        es->callAsync("runner", elapsed);
    }
}
#endif

// Define what processes the plug-in runs once it is loaded into a program by the plugin manager.
void RunnerImpl::initialize(size_t identifier)
{
    std::cout << "RunnerImpl::initialize" << std::endl;
#if defined(EVENT_SYNC_R) || defined(EVENT_ASYNC_R) || defined(EVENT_MULTI_R)
    es = EventStream<double>::Instance(identifier);
    es->create("runner");
#endif
}

// Define what processes the plug-in runs immediately before it is unloaded by the plugin manager.
void RunnerImpl::release()
{
    std::cout << "RunnerImpl::release" << std::endl;
#if defined(EVENT_SYNC_R) || defined(EVENT_ASYNC_R) || defined(EVENT_MULTI_R)
    es->destroy("runner");
    es->requestDelete();
    es = nullptr;
#endif
#ifdef DIRECT_R
    descriptors.clear();
#endif
}

// Start the runner plugin in a separate thread. Only used on the C++ end; for some reason python bindings on start() don't work (because 
// python bindings on functions that create new worker threads in general aren't allowed, from what I understand), so we instead have those
// bindings for start2, which doesn't spawn a separate worker thread .
void RunnerImpl::start()
{
    // If a call to stop() was made, wait for it to finish before restarting the runner.
    while (g_block || g_stop) continue;
    g_start = true;
    std::cout << "RunnerImpl::start" << std::endl;
    
    thread = std::thread(&RunnerImpl::start2, RunnerImpl());
}

// Breaks the update loop and joins the runner thread.
void RunnerImpl::stop()
{
    // Since runner is executed on a separate loop, wait for its start() function to complete
    // before attempting to call stop().
    while (g_block || g_start) continue;
    g_stop = true;
    std::cout << "RunnerImpl::stop" << std::endl;
    stop2();
#ifdef EVENT_MULTI_R
    if (t1.joinable())
    {
        t1.join();
    }
    if (t2.joinable())
    {
        t2.join();
    }
#endif
    thread.join();
    g_stop = false;
}

// Helper function to begin running the runner. Note that to update methods on the runner tick, we either call
// the handlers subscribed to the runner event, or call those functions from the internal list of Runner descriptors
// associated with the current Runner instance (or both).

void RunnerImpl::start2()
{
    std::cout << "RunnerImpl::start2" << std::endl;
#ifndef EVENT_MULTI_R
    g_start = false;
#endif
    // Order function descriptors by priority
#ifdef DIRECT_R
    std::sort(Runner::descriptors.begin(), Runner::descriptors.end(), priority_queue());
#endif
    std::chrono::high_resolution_clock::time_point lastTime = std::chrono::high_resolution_clock::now();

    // Note that we can't run std::threads in here if plan on calling this start2() function from Python via
    // python bindings. If we wish to have multithreading on the python end, we need to implement that functionality
    // natively using python, not through bindings/C++.
#ifdef EVENT_MULTI_R
    // Here we call the callAsyncWrapper in two separate threads. Recall that all event-calling functions have mutex locks, and
    // since we're attempting to call runner's handlers in two separate threads, the mutex locks will force one of the threads
    // to wait for the other thread to finish execution before it can run callAsyncWrapper.
    t1 = std::thread(callAsyncWrapper, lastTime);
    t2 = std::thread(callAsyncWrapper, lastTime);
#endif // EVENT_MULTI_R

    while (!breakLoop)
    {
        //std::cout << "Runner loop is going" << std::endl;
        //if (g_block) std::cout << "g_block is also true!" << std::endl;
        if (g_block) continue;

        std::chrono::high_resolution_clock::time_point currentTime = std::chrono::high_resolution_clock::now();
        double elapsed = std::chrono::duration_cast<std::chrono::duration<double>>(currentTime - lastTime).count();

#ifdef EVENT_SYNC_R
        es->call("runner", elapsed);
#elif defined(EVENT_ASYNC_R)
        es->callAsync("runner", elapsed);
#endif // EVENT_SYNC_R or EVENT_ASYNC_R

#ifdef DIRECT_R
        for (int i = 0; i < Runner::descriptors.size(); ++i)
        {
            if (Runner::descriptors[i].cUpdate != nullptr)
            {
                Runner::descriptors[i].cUpdate(elapsed);
            }
            else if (Runner::descriptors[i].pyUpdate != nullptr)
            {
                Runner::descriptors[i].pyUpdate(elapsed);
            }
        }
#endif // DIRECT_R

        lastTime = currentTime;
    }
}

// Sets a parameter to break the update loop.
void RunnerImpl::stop2()
{
    std::cout << "RunnerImpl::stop2" << std::endl;
    breakLoop = true;
}

// Registers the descriptors of functions that have been newly assigned for updating in the main loop.
#ifdef DIRECT_R
void RunnerImpl::push(RunnerDesc desc)
{
    std::cout << "RunnerImpl::push" << std::endl;
    g_block = true; // Blocks runner from calling any functions directly during pushing action.
    size_t cnt = 0;
    for (size_t i = 0; i < Runner::descriptors.size(); ++i)
    {
        if (strcmp(Runner::descriptors[i].name, desc.name) == 0 && Runner::descriptors[i].priority == desc.priority && Runner::descriptors[i].cUpdate == desc.cUpdate)
        {
            return;
        }
        else
        {
            ++cnt;
        }
    }

    if (cnt == Runner::descriptors.size()) Runner::descriptors.push_back(desc);
    g_block = false;
}
#endif

// Removes function descriptors that have been designated for removal from the main update cycle.
#ifdef DIRECT_R
bool RunnerImpl::pop(RunnerDesc desc)
{
    std::cout << "RunnerImpl::pop" << std::endl;
    g_block = true; // Blocks runner from calling any functions directly during popping action.
    size_t count = Runner::descriptors.size();
    std::vector<RunnerDesc>::iterator it = Runner::descriptors.begin();
    while (it != Runner::descriptors.end())
    {
        if (strcmp(it->name, desc.name) == 0 && it->priority == desc.priority && it->cUpdate == desc.cUpdate)
        {
            it = Runner::descriptors.erase(it);
        }
        else
        {
            ++it;
        }
    }
    g_block = false;
    return Runner::descriptors.size() != count;
}
#endif

// Define functions with C symbols (create/destroy Runner instance).
RunnerImpl* g_runner = nullptr;

extern "C" RUNNER RunnerImpl* Create()
{
    if (g_runner != nullptr)
    {
        return g_runner;
    }

    else
    {
        g_runner = new RunnerImpl;
        return g_runner;
    }
}
extern "C" RUNNER void Destroy()
{
    assert(g_runner);
    delete g_runner;
    g_runner = nullptr;
}
