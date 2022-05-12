// atomicPlugin demonstrates how atomic data can used in multithreaded environments
// to preserve the integrity of said data. In this specific example a simple atomic
// int is incremented using both the runner (on each update loop tick the integer is
// incremented by 1) and input (each time a keyboard and/or mouse event is registered,
// the counter is incremented by 1) plugins, both of which are configured to run in their
// own worker threads separate from the main thread.

#include "stdafx.h"
#include <assert.h>
#include <iostream>
#include <vector>
#include <chrono>
#include <thread>

#include "atomicPluginimpl.h"

#ifdef EVENT_A
#include "event.h"
#endif
#ifdef DIRECT_A
#include "runner.h"
#include "pluginManager.h"
#endif

#ifdef EVENT_A
EventStream<double>* es_d;
EventStream<InputData>* es_i;
std::vector<size_t> g_id1, g_id2, g_id3;
#endif
#ifdef DIRECT_A
Runner* runner;
Input* input;
PluginManager* pm;
size_t g_counter = 0;
RunnerDesc rDesc;
InputDesc iDesc;
#endif
std::atomic<size_t> g_atomicInt = 0;

void updateTick(double dt)
{
    AtomicPluginImpl api;
    api.updateTick(dt);
}

void updateEvent(InputData inputData)
{
    AtomicPluginImpl api;
    api.updateInput(inputData);
}

void AtomicPluginImpl::initialize(size_t identifier)
{
#ifdef EVENT_A
    // Create two separate EventStreams to handle the Events we'll be subscribing to later.
    // es_d is an EventStream with type "double" template specialization, meaning that it's responsible
    // for dealing with all Events whose handlers take a single double as their only argument.
    // es_i is an EventStream with type "struct InputData" template specialization, meaning that it's responsible
    // for dealing with all Events whose handlers take a single InputData struct as their only argument.
    es_d = EventStream<double>::Instance(identifier);
    es_i = EventStream<InputData>::Instance(identifier);
#endif
#ifdef DIRECT_A
    pm = PluginManager::Instance(identifier);
#endif

    std::cout << "AtomicPluginImpl::initialize: " << identifier << std::endl;
}

void AtomicPluginImpl::release()
{
    std::cout << "AtomicPluginImpl::release" << std::endl;
#ifdef DIRECT_A
    pm->Unload("runner");
    pm->Unload("input");
    pm->requestDelete();
#endif
}

void AtomicPluginImpl::start()
{
    std::cout << "AtomicPluginImpl::start" << std::endl;
#ifdef EVENT_A
    g_id1 = es_d->subscribe("runner", &(::updateTick));
    g_id2 = es_i->subscribe("input_keyboard", &(::updateEvent));
    g_id3 = es_i->subscribe("input_mouse", &(::updateEvent));
#endif

#ifdef DIRECT_A
    runner = (Runner*)pm->Load("runner");
    input = (Input*)pm->Load("input");

    rDesc.priority = 2;
    rDesc.name = "AtomicPluginImpl::updateTick";
    rDesc.cUpdate = &(::updateTick);
    rDesc.pyUpdate = nullptr;
    runner->push(rDesc);

    iDesc.keyboardUpdatePriority = 1;
    iDesc.mouseUpdatePriority = 1;
    iDesc.name = "AtomicPluginImpl::updateEvent";
    iDesc.cKeyboardUpdate = &(::updateEvent);
    iDesc.cMouseUpdate = &(::updateEvent);
    iDesc.pyKeyboardUpdate = nullptr;
    iDesc.pyMouseUpdate = nullptr;
    input->push(iDesc);
#endif

    // Put the current thread to sleep for a few seconds so that the stop() function in main.cpp
    // isn't immediately called and the user has an opportunity to see how g_atomicInt evolves
    // with time/input.
    std::this_thread::sleep_for(std::chrono::seconds(5));
}

void AtomicPluginImpl::stop()
{
    std::cout << "AtomicPluginImpl::stop" << std::endl;

#ifdef EVENT_A
    es_d->unsubscribe("runner", g_id1);
    es_i->unsubscribe("input_keyboard", g_id2);
    es_i->unsubscribe("input_mouse", g_id3);

    es_d->requestDelete();
    es_i->requestDelete();
#endif
#ifdef DIRECT_A
    runner->pop(rDesc);
    input->pop(iDesc);
#endif
}

// Simple update method that increments an atomic int each runner tick.
void AtomicPluginImpl::updateTick(double dt)
{
    ++g_atomicInt;
    std::cout << "AtomicPluginImpl::updateTick " + std::to_string(dt) + ", " + std::to_string(g_atomicInt) << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(1));
}

// Simple update method that increments an atomic int each time an input event is registered.
void AtomicPluginImpl::updateInput(InputData inputData)
{
    if (inputData.mouseMoved || inputData.keyDown)
    {
        ++g_atomicInt;
        std::cout << "AtomicPluginImpl::updateEvent " + std::to_string(g_atomicInt) << std::endl;
    }
}

// Define functions with C symbols (create/destroy AtomicPluginImpl instance)
AtomicPluginImpl* g_atomicpluginimpl = nullptr;

extern "C" ATOMICPLUGIN AtomicPluginImpl * Create()
{
    assert(g_atomicpluginimpl == nullptr);
    g_atomicpluginimpl = new AtomicPluginImpl;
    return g_atomicpluginimpl;
}
extern "C" ATOMICPLUGIN void Destroy()
{
    assert(g_atomicpluginimpl);
    delete g_atomicpluginimpl;
    g_atomicpluginimpl = nullptr;
}
