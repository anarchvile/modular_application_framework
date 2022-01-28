// This is a very simple example plug-in that is meant to both illustrate the process behind
// correctly structuring a plug-in, and highlighting how a plug-in could be expanded to perform
// more interesting tasks.

#include "stdafx.h"
#include <assert.h>
#include <iostream>
#include <vector>
#include <chrono>
#include <thread>

#include "atomicPluginimpl.h"
#include "runner.h"
#include "pluginManager.h"

//#include "event.h"

Runner* runner;
Input* input;
PluginManager* pm;
size_t g_counter = 0;
std::atomic<size_t> g_atomicInt = 0;
//size_t g_atomicInt = 0;
RunnerDesc rDesc;
InputDesc iDesc;
//EventStream<double>* es_d;
//EventStream<InputData>* es_i;
//size_t g_id1, g_id2;

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
    pm = PluginManager::Instance(identifier);
    //es_d = EventStream<double>::Instance();
    //es_i = EventStream<InputData>::Instance();

    std::cout << "AtomicPluginImpl::initialize: " << identifier << std::endl;
}

void AtomicPluginImpl::release()
{
    std::cout << "AtomicPluginImpl::release" << std::endl;
    pm->Unload("runner");
    pm->Unload("input");
}

void AtomicPluginImpl::start()
{
    std::cout << "AtomicPluginImpl::start" << std::endl;
    
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
    
    //g_id1 = es_d->subscribe("runner", &(::updateTick));
    //g_id1 = es_i->subscribe("input", &(::updateEvent));
}

void AtomicPluginImpl::stop()
{
    std::cout << "AtomicPluginImpl::stop" << std::endl;
    runner->pop(rDesc);
    input->pop(iDesc);
    
    //es_d->unsubscribe("runner", g_id1);
    //es_i->unsubscribe("input", g_id2);
}

// Simple update method that signals the runner to unregister itself from the update cycle after 100 iterations.
void AtomicPluginImpl::updateTick(double dt)
{
    ++g_atomicInt;
    std::cout << "AtomicPluginImpl::updateTick " + std::to_string(dt) + ", " + std::to_string(g_atomicInt) << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(1));

    if (g_counter == 4)
    {
        //runner->pop(rDesc);
        //runner->stop();
    }

    ++g_counter;
}

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
