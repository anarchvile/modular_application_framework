// The runner plug-in is responsible for continuously running a game-engine style update
// loop, and for managing the subscription/unsubscription of external plug-in functions
// to the update cycle via a RunnerDesc struct. Functions that wish to be updated in this
// global loop, for example, simply need to pass a reference of themselves to a RunnerDesc 
// instance (along with some other information, like priority and name) before passing that
// RunnerDesc to a local instance of the Runner plug-in, which handles the rest of the process.

#include "stdafx.h"

#include <assert.h>
#include <iostream>
#include <algorithm>
#include <chrono>
#include <mutex>

#include "runnerImpl.h"
#include "event.h"

std::vector<RunnerDesc> Runner::descriptors;
std::mutex m;
bool breakLoop = false;
bool g_block = false;
EventStream<double>* es;
Event<double>* eventPtr;
std::thread t1, t2;

void callAsyncWrapper(std::chrono::high_resolution_clock::time_point lastTime)
{
    while (!breakLoop)
    {
        if (g_block) continue;

        std::chrono::high_resolution_clock::time_point currentTime = std::chrono::high_resolution_clock::now();
        double elapsed = std::chrono::duration_cast<std::chrono::duration<double>>(currentTime - lastTime).count();

        eventPtr->callAsync(elapsed);
    }
}

// Define what processes the plug-in runs once it is loaded into a program by the plugin manager.
void RunnerImpl::initialize(size_t identifier)
{
    std::cout << "RunnerImpl::initialize" << std::endl;
    es = EventStream<double>::Instance();
    eventPtr = es->create("runner");
    std::cout << "__________Count from runner__________" << std::endl;
    es->numEvents();
}

// Define what processes the plug-in runs immediately before it is unloaded the plugin manager.
void RunnerImpl::release()
{
    std::cout << "RunnerImpl::release" << std::endl;
    descriptors.clear();
    es->destroy("runner", eventPtr);
    //delete es; // Don't want to delete the memory allocated at es, since EventStream is a singleton.
    es = nullptr;
}

// Orders function descriptors by priority and calls them one-by-one, in order, in a ticking update loop.
// This is done continously until a valid condition to break the update cycle arises.
void RunnerImpl::start()
{
    thread = std::thread(&RunnerImpl::start2, RunnerImpl());
}

// Sets a parameter to break the update loop.
void RunnerImpl::stop()
{
    std::cout << "RunnerImpl::stop" << std::endl;
    stop2();
    //thread.detach();
    //t1.join();
    //t2.join();
    thread.join();
}

void RunnerImpl::start2()
{
    std::cout << "RunnerImpl::start2" << std::endl;
    std::sort(Runner::descriptors.begin(), Runner::descriptors.end(), priority_queue());
    std::chrono::high_resolution_clock::time_point lastTime = std::chrono::high_resolution_clock::now();

    // Note that we can't run std::threads in here if plan on calling this start2() function froum Python via
    // python bindings. If we wish to have multithreading on the python end, we need to implement that functionality
    // natively using python, not through bindings/C++.
    //t1 = std::thread(callAsyncWrapper, lastTime);
    //t2 = std::thread(callAsyncWrapper, lastTime);

    while (!breakLoop)
    {
        if (g_block) continue;

        std::chrono::high_resolution_clock::time_point currentTime = std::chrono::high_resolution_clock::now();
        double elapsed = std::chrono::duration_cast<std::chrono::duration<double>>(currentTime - lastTime).count();

        // TODO: Should the event system run parallel to the internal update loop on Runner::descriptors?
        //eventPtr->call(elapsed);
        eventPtr->callAsync(elapsed);

        auto epoch = currentTime.time_since_epoch();
        auto value = std::chrono::duration_cast<std::chrono::milliseconds>(epoch);
        long duration = value.count();

        /*for (auto x : Runner::descriptors)
        {
            if (x.cUpdate != nullptr)
            {
                x.cUpdate(elapsed);
            }
            else if (x.cUpdate == nullptr)
            {
                x.pyUpdate(elapsed);
            }
        }*/
        lastTime = currentTime;
    }
}

void RunnerImpl::stop2()
{
    std::cout << "RunnerImpl::stop2" << std::endl;
    breakLoop = true;
}

// Registers the descriptors of functions that have been newly assigned for updating in the main loop.
void RunnerImpl::push(const RunnerDesc& desc)
{
    std::cout << "RunnerImpl::push" << std::endl;
    g_block = true;
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

// Removes function descriptors that have been designated for unsubscription from the main update cycle.
bool RunnerImpl::pop(const RunnerDesc& desc)
{
    std::cout << "RunnerImpl::pop" << std::endl;
    g_block = true;
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

// Define functions with C symbols (create/destroy Runner instance)
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
